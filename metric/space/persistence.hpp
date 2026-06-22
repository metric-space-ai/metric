// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_PERSISTENCE_HPP
#define _METRIC_SPACE_PERSISTENCE_HPP

#include <cmath>
#include <cstddef>
#include <iomanip>
#include <ios>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/core/version.hpp>
#include <metric/record/id.hpp>
#include <metric/space/distances.hpp>

// Native finite-space artifacts.
//
// `mtrc::space::persistence` is deliberately small and dependency-free. It exports a finite metric
// space as records + RecordIds + metric identity + version + optional materialized pair values. Loading
// takes an explicit metric object; METRIC does not try to serialize arbitrary metric callables. This
// keeps algorithms in C++, preserves finite-space identity, and gives callers a stable interchange point
// for common streamable records while allowing custom record codecs for application-specific types.
namespace mtrc::space::persistence {

struct artifact_options {
	bool include_pair_distances{true};
	std::string metric_key;
};

struct load_options {
	bool require_metric_key_match{true};
	long double distance_tolerance{};
};

template <typename Record> struct record_text_codec {
	auto encode(const Record &record) const -> std::string
	{
		std::ostringstream output;
		output << record;
		if (!output) {
			throw MetricInputError("record_text_codec could not encode record");
		}
		return output.str();
	}

	auto decode(const std::string &payload) const -> Record
	{
		std::istringstream input(payload);
		Record record{};
		if (!(input >> record)) {
			throw MetricInputError("record_text_codec could not decode record");
		}
		return record;
	}
};

template <> struct record_text_codec<std::string> {
	auto encode(const std::string &record) const -> std::string { return record; }
	auto decode(const std::string &payload) const -> std::string { return payload; }
};

template <typename Record> struct record_entry {
	RecordId id;
	Record record;
};

template <typename Distance> struct distance_entry {
	RecordId lhs;
	RecordId rhs;
	Distance distance{};
};

template <typename Record, typename Distance> struct space_artifact {
	using record_type = Record;
	using distance_type = Distance;

	std::string format{"mtrc.space.artifact"};
	std::size_t format_version{1};
	std::string metric_key;
	std::string metric_law;
	SpaceVersion space_version{initial_space_version};
	std::size_t next_record_id{};
	std::vector<record_entry<record_type>> records;
	std::vector<distance_entry<distance_type>> distances;
	bool pair_distances_included{};

	auto record_count() const -> std::size_t { return records.size(); }
	auto pair_count() const -> std::size_t { return distances.size(); }
};

template <typename Distance> struct distance_mismatch {
	RecordId lhs;
	RecordId rhs;
	Distance expected{};
	Distance actual{};
};

template <typename Distance> struct distance_verification {
	std::size_t checked{};
	std::vector<distance_mismatch<Distance>> mismatches;

	auto ok() const -> bool { return mismatches.empty(); }
};

namespace detail {

inline auto next_record_id_from_ids(const std::vector<RecordId> &ids) -> std::size_t
{
	std::size_t next = 0;
	for (const auto id : ids) {
		if (id.index() >= next) {
			next = id.index() + 1;
		}
	}
	return next;
}

template <typename Space, typename = void> struct next_record_id_reader {
	static auto get(const Space &space) -> std::size_t { return next_record_id_from_ids(record_ids(space)); }
};

template <typename Space>
struct next_record_id_reader<Space, std::void_t<decltype(std::declval<const Space &>().next_record_id())>> {
	static auto get(const Space &space) -> std::size_t { return space.next_record_id(); }
};

template <typename Space> auto next_record_id(const Space &space) -> std::size_t
{
	return next_record_id_reader<Space>::get(space);
}

template <typename Value> auto value_to_payload(const Value &value) -> std::string
{
	std::ostringstream output;
	if constexpr (std::is_arithmetic<Value>::value) {
		output << std::setprecision(std::numeric_limits<Value>::max_digits10);
	}
	output << value;
	if (!output) {
		throw MetricInputError("could not encode artifact value");
	}
	return output.str();
}

template <typename Value> auto value_from_payload(const std::string &payload) -> Value
{
	std::istringstream input(payload);
	Value value{};
	if (!(input >> value)) {
		throw MetricInputError("could not decode artifact value");
	}
	return value;
}

inline auto values_match(long double lhs, long double rhs, long double tolerance) -> bool
{
	return tolerance <= 0 ? lhs == rhs : std::fabs(lhs - rhs) <= tolerance;
}

template <typename Value> auto values_match(const Value &lhs, const Value &rhs, long double tolerance) -> bool
{
	if constexpr (std::is_arithmetic<Value>::value) {
		return values_match(static_cast<long double>(lhs), static_cast<long double>(rhs), tolerance);
	} else {
		(void)tolerance;
		return lhs == rhs;
	}
}

inline auto expect_token(std::istream &input, const char *expected) -> void
{
	std::string actual;
	if (!(input >> actual) || actual != expected) {
		throw MetricInputError(std::string("invalid space artifact: expected token ") + expected);
	}
}

inline auto read_size(std::istream &input, const char *field) -> std::size_t
{
	std::size_t value{};
	if (!(input >> value)) {
		throw MetricInputError(std::string("invalid space artifact: expected size for ") + field);
	}
	return value;
}

inline auto consume_payload_newline(std::istream &input, const char *field) -> void
{
	const auto next = input.get();
	if (next == '\r') {
		if (input.get() != '\n') {
			throw MetricInputError(std::string("invalid space artifact newline after ") + field);
		}
		return;
	}
	if (next != '\n') {
		throw MetricInputError(std::string("invalid space artifact newline after ") + field);
	}
}

inline auto write_blob(std::ostream &output, const std::string &payload) -> void
{
	output << payload.size() << '\n';
	output.write(payload.data(), static_cast<std::streamsize>(payload.size()));
	output << '\n';
	if (!output) {
		throw MetricInputError("could not write space artifact payload");
	}
}

inline auto read_blob(std::istream &input, const char *field) -> std::string
{
	const auto size = read_size(input, field);
	if (size > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())) {
		throw MetricInputError(std::string("space artifact payload is too large for ") + field);
	}
	consume_payload_newline(input, field);
	std::string payload(size, '\0');
	if (size > 0) {
		input.read(&payload[0], static_cast<std::streamsize>(size));
		if (input.gcount() != static_cast<std::streamsize>(size)) {
			throw MetricInputError(std::string("space artifact payload ended early for ") + field);
		}
	}
	consume_payload_newline(input, field);
	return payload;
}

inline auto write_field(std::ostream &output, const char *name, const std::string &payload) -> void
{
	output << name << ' ';
	write_blob(output, payload);
}

inline auto read_field_blob(std::istream &input, const char *name) -> std::string
{
	expect_token(input, name);
	return read_blob(input, name);
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto export_space(const Space &space, artifact_options options = {})
	-> space_artifact<typename Space::record_type, typename Space::distance_type>
{
	using artifact_type = space_artifact<typename Space::record_type, typename Space::distance_type>;
	artifact_type artifact;
	artifact.metric_key = options.metric_key.empty() ? metric_cache_key(space.metric()) : std::move(options.metric_key);
	artifact.metric_law = metric_law_name(metric_traits<typename Space::metric_type>::law);
	artifact.space_version = space.version();
	artifact.next_record_id = detail::next_record_id(space);
	artifact.pair_distances_included = options.include_pair_distances;

	artifact.records.reserve(space.size());
	for (std::size_t position = 0; position < space.size(); ++position) {
		const auto id = space.id(position);
		artifact.records.push_back(record_entry<typename Space::record_type>{id, space.record(id)});
	}

	if (options.include_pair_distances) {
		for (const auto &pair : distances::pairs(space)) {
			artifact.distances.push_back(
				distance_entry<typename Space::distance_type>{pair.lhs, pair.rhs, pair.distance});
		}
	}

	return artifact;
}

template <typename Space, typename Record, typename Distance,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto verify_distances(const Space &space, const space_artifact<Record, Distance> &artifact,
					  long double tolerance = 0) -> distance_verification<Distance>;

template <typename Record, typename Distance, typename Metric>
auto restore_space(const space_artifact<Record, Distance> &artifact, Metric metric, load_options options = {})
	-> core::MetricSpace<Record, typename std::decay<Metric>::type>
{
	using metric_type = typename std::decay<Metric>::type;
	using distance_type = metric_result_t<metric_type, Record>;
	static_assert(std::is_convertible<distance_type, Distance>::value,
				  "space artifact distance type must be compatible with the supplied metric");
	const auto actual_metric_key = metric_cache_key(metric);
	if (options.require_metric_key_match && !artifact.metric_key.empty() && artifact.metric_key != actual_metric_key) {
		throw MetricInputError("space artifact metric key does not match the metric supplied for restore");
	}

	std::vector<Record> records;
	std::vector<RecordId> ids;
	records.reserve(artifact.records.size());
	ids.reserve(artifact.records.size());
	for (const auto &entry : artifact.records) {
		ids.push_back(entry.id);
		records.push_back(entry.record);
	}

	auto space = core::MetricSpace<Record, metric_type>(
		std::move(records), std::move(metric), std::move(ids), artifact.next_record_id, artifact.space_version);
	if (options.distance_tolerance >= 0 && artifact.pair_distances_included) {
		const auto verification = verify_distances(space, artifact, options.distance_tolerance);
		if (!verification.ok()) {
			throw MetricInputError("space artifact materialized distances do not match the supplied metric");
		}
	}
	return space;
}

template <typename Space, typename Record, typename Distance,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type>
auto verify_distances(const Space &space, const space_artifact<Record, Distance> &artifact,
					  long double tolerance) -> distance_verification<Distance>
{
	distance_verification<Distance> report;
	for (const auto &entry : artifact.distances) {
		if (!space.contains(entry.lhs) || !space.contains(entry.rhs)) {
			throw MetricInputError("space artifact distance references a RecordId outside the supplied space");
		}
		const auto actual = space.distance(entry.lhs, entry.rhs);
		++report.checked;
		if (!detail::values_match(actual, entry.distance, tolerance)) {
			report.mismatches.push_back(distance_mismatch<Distance>{entry.lhs, entry.rhs, entry.distance, actual});
		}
	}
	return report;
}

template <typename Record, typename Distance, typename RecordCodec = record_text_codec<Record>>
auto save_artifact(std::ostream &output, const space_artifact<Record, Distance> &artifact,
				   RecordCodec codec = RecordCodec{}) -> void
{
	output << "mtrc_space_artifact_v1\n";
	detail::write_field(output, "metric_key", artifact.metric_key);
	detail::write_field(output, "metric_law", artifact.metric_law);
	output << "space_version " << artifact.space_version << '\n';
	output << "next_record_id " << artifact.next_record_id << '\n';
	output << "records " << artifact.records.size() << '\n';
	for (const auto &entry : artifact.records) {
		output << "record " << entry.id.index() << ' ';
		detail::write_blob(output, codec.encode(entry.record));
	}
	output << "pair_distances " << (artifact.pair_distances_included ? 1 : 0) << ' ' << artifact.distances.size()
		   << '\n';
	for (const auto &entry : artifact.distances) {
		output << "distance " << entry.lhs.index() << ' ' << entry.rhs.index() << ' ';
		detail::write_blob(output, detail::value_to_payload(entry.distance));
	}
	output << "end\n";
}

template <typename Record, typename Distance, typename RecordCodec = record_text_codec<Record>>
auto load_artifact(std::istream &input, RecordCodec codec = RecordCodec{}) -> space_artifact<Record, Distance>
{
	detail::expect_token(input, "mtrc_space_artifact_v1");

	space_artifact<Record, Distance> artifact;
	artifact.metric_key = detail::read_field_blob(input, "metric_key");
	artifact.metric_law = detail::read_field_blob(input, "metric_law");

	detail::expect_token(input, "space_version");
	artifact.space_version = detail::read_size(input, "space_version");
	detail::expect_token(input, "next_record_id");
	artifact.next_record_id = detail::read_size(input, "next_record_id");

	detail::expect_token(input, "records");
	const auto record_count = detail::read_size(input, "records");
	artifact.records.reserve(record_count);
	for (std::size_t index = 0; index < record_count; ++index) {
		(void)index;
		detail::expect_token(input, "record");
		const auto id = RecordId::from_index(detail::read_size(input, "record id"));
		const auto payload = detail::read_blob(input, "record");
		artifact.records.push_back(record_entry<Record>{id, codec.decode(payload)});
	}

	detail::expect_token(input, "pair_distances");
	artifact.pair_distances_included = detail::read_size(input, "pair_distances flag") != 0;
	const auto distance_count = detail::read_size(input, "pair_distances count");
	artifact.distances.reserve(distance_count);
	for (std::size_t index = 0; index < distance_count; ++index) {
		(void)index;
		detail::expect_token(input, "distance");
		const auto lhs = RecordId::from_index(detail::read_size(input, "distance lhs"));
		const auto rhs = RecordId::from_index(detail::read_size(input, "distance rhs"));
		const auto payload = detail::read_blob(input, "distance");
		artifact.distances.push_back(distance_entry<Distance>{lhs, rhs, detail::value_from_payload<Distance>(payload)});
	}

	detail::expect_token(input, "end");
	return artifact;
}

template <typename Space, typename RecordCodec = record_text_codec<typename Space::record_type>,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto save(std::ostream &output, const Space &space, artifact_options options = {}, RecordCodec codec = RecordCodec{})
	-> void
{
	save_artifact(output, export_space(space, std::move(options)), std::move(codec));
}

template <typename Record, typename Metric> struct loaded_space {
	using metric_type = Metric;
	using space_type = core::MetricSpace<Record, metric_type>;
	using distance_type = typename space_type::distance_type;

	space_type space;
	space_artifact<Record, distance_type> artifact;
};

template <typename Record, typename Metric, typename RecordCodec = record_text_codec<Record>>
auto load(std::istream &input, Metric metric, load_options options = {}, RecordCodec codec = RecordCodec{})
	-> loaded_space<Record, typename std::decay<Metric>::type>
{
	using metric_type = typename std::decay<Metric>::type;
	using distance_type = metric_result_t<metric_type, Record>;
	auto artifact = load_artifact<Record, distance_type>(input, std::move(codec));
	auto space = restore_space(artifact, std::move(metric), options);
	return loaded_space<Record, metric_type>{std::move(space), std::move(artifact)};
}

} // namespace mtrc::space::persistence

#endif
