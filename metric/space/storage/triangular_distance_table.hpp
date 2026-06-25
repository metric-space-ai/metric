// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_TRIANGULAR_DISTANCE_TABLE_HPP
#define _METRIC_REPRESENTATIONS_TRIANGULAR_DISTANCE_TABLE_HPP

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/record/id.hpp>

#include "diagnostics.hpp"
#include "policy.hpp"

namespace mtrc::space::storage {

namespace triangular_detail {

using record_position_lookup_type = std::unordered_map<RecordId, std::size_t>;

inline auto checked_product(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
{
	if (rhs != 0 && lhs > std::numeric_limits<std::size_t>::max() / rhs) {
		throw RepresentationError(message);
	}
	return lhs * rhs;
}

inline auto checked_sum(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
{
	if (lhs > std::numeric_limits<std::size_t>::max() - rhs) {
		throw RepresentationError(message);
	}
	return lhs + rhs;
}

inline auto make_record_position_lookup(const std::vector<RecordId> &ids) -> record_position_lookup_type
{
	record_position_lookup_type positions;
	positions.reserve(ids.size());
	for (std::size_t position = 0; position < ids.size(); ++position) {
		positions.emplace(ids[position], position);
	}
	return positions;
}

inline auto record_id_storage_bytes(std::size_t record_count) -> std::size_t
{
	return checked_product(record_count, sizeof(RecordId),
						   "triangular distance table memory estimate exceeds size_t capacity");
}

inline auto position_index_storage_bytes(std::size_t record_count) -> std::size_t
{
	return checked_product(record_count, sizeof(record_position_lookup_type::value_type),
						   "triangular distance table memory estimate exceeds size_t capacity");
}

} // namespace triangular_detail

inline auto triangular_distance_slot_count(std::size_t record_count) -> std::size_t
{
	if (record_count < 2) {
		return 0;
	}
	if (record_count % 2 == 0) {
		return triangular_detail::checked_product(
			record_count / 2, record_count - 1,
			"triangular distance table slot count exceeds size_t capacity");
	}
	return triangular_detail::checked_product(
		record_count, (record_count - 1) / 2,
		"triangular distance table slot count exceeds size_t capacity");
}

template <typename Distance> auto estimate_triangular_distance_table_memory_bytes(std::size_t record_count)
	-> std::size_t
{
	const auto slots = triangular_distance_slot_count(record_count);
	const auto table_bytes = triangular_detail::checked_product(
		slots, sizeof(Distance), "triangular distance table memory estimate exceeds size_t capacity");
	const auto id_bytes = triangular_detail::record_id_storage_bytes(record_count);
	const auto index_bytes = triangular_detail::position_index_storage_bytes(record_count);
	return triangular_detail::checked_sum(
		triangular_detail::checked_sum(table_bytes, id_bytes,
									   "triangular distance table memory estimate exceeds size_t capacity"),
		index_bytes, "triangular distance table memory estimate exceeds size_t capacity");
}

template <typename Space> class SymmetricDistanceTable {
  public:
	using space_type = Space;
	using metric_type = typename space_type::metric_type;
	using distance_type = typename space_type::distance_type;
	static constexpr auto metric_type_law = core::metric_traits<metric_type>::law;

	explicit SymmetricDistanceTable(const space_type &space, runtime_guard runtime = {})
		: space_(&space), record_count_(space.size()), version_(space.version()),
		  off_diagonal_slots_(triangular_distance_slot_count(record_count_)),
		  memory_bytes_estimate_(estimate_triangular_distance_table_memory_bytes<distance_type>(record_count_)),
		  metric_key_(core::metric_cache_key(space.metric()))
	{
		require_admitted_symmetric_metric();
		ids_ = mtrc::record_ids(space);
		id_positions_ = triangular_detail::make_record_position_lookup(ids_);
		cache_key_ = representation_cache_key("symmetric_distance_table", metric_key_, version_, ids_);
		distances_.reserve(off_diagonal_slots_);
		for (std::size_t lhs = 0; lhs < record_count_; ++lhs) {
			for (std::size_t rhs = lhs + 1; rhs < record_count_; ++rhs) {
				runtime.throw_if_cancelled("symmetric_distance_table materialization");
				distances_.push_back(compute_source_distance_at_position(lhs, rhs));
			}
		}
	}

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type
	{
		const auto lhs_position = position_of(lhs);
		const auto rhs_position = position_of(rhs);
		return distance_at_validated_position(lhs_position, rhs_position);
	}

	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position) const -> distance_type
	{
		validate_position(lhs_position);
		validate_position(rhs_position);
		return distance_at_validated_position(lhs_position, rhs_position);
	}

	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position,
							  runtime_guard runtime) const -> distance_type
	{
		runtime.throw_if_cancelled("symmetric_distance_table lookup");
		validate_position(lhs_position);
		validate_position(rhs_position);
		return distance_at_validated_position(lhs_position, rhs_position);
	}

	auto record_count() const -> std::size_t { return record_count_; }

	auto id(std::size_t position) const -> RecordId
	{
		validate_position(position);
		return ids_[position];
	}

	auto position_of(RecordId id) const -> std::size_t
	{
		const auto position = id_positions_.find(id);
		if (position == id_positions_.end()) {
			throw std::out_of_range("record id is outside the symmetric distance table");
		}
		return position->second;
	}

	auto contains(RecordId id) const -> bool { return id_positions_.find(id) != id_positions_.end(); }
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }
	auto cached_distances() const -> std::size_t { return distances_.size(); }
	auto dense_distance_slots() const -> std::size_t { return off_diagonal_slots_; }
	auto off_diagonal_slots() const -> std::size_t { return off_diagonal_slots_; }
	auto memory_bytes_estimate() const -> std::size_t { return memory_bytes_estimate_; }
	auto metric_key() const -> const std::string & { return metric_key_; }
	auto cache_key() const -> const std::string & { return cache_key_; }
	auto source_record_ids() const -> const std::vector<RecordId> & { return ids_; }

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::distance_table, exactness::exact,
										  materialization::materialized, update_mode::snapshot};
		result.space_version = space_->version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.records = record_count_;
		result.distance_evaluations = distances_.size();
		result.cached_distances = distances_.size();
		result.dense_distance_slots = off_diagonal_slots_;
		result.memory_bytes_estimate = memory_bytes_estimate_;
		result.cache_key = cache_key_;
		result.metric_key = metric_key_;
		result.source_record_ids = ids_;
		if (result.stale) {
			result.warnings.push_back("symmetric distance table was built for an older metric-space version");
		}
		return result;
	}

  private:
	static auto require_admitted_symmetric_metric() -> void
	{
		constexpr auto law = metric_type_law;
		if constexpr (law != core::metric_law::metric && law != core::metric_law::pseudo_metric) {
			throw RepresentationError(
				"symmetric distance table requires metric_traits<Metric>::law to be metric or pseudo_metric");
		}
	}

	static auto offset(std::size_t lhs_position, std::size_t rhs_position, std::size_t record_count) -> std::size_t
	{
		if (lhs_position > rhs_position) {
			std::swap(lhs_position, rhs_position);
		}
		const auto slots_before_row =
			triangular_distance_slot_count(record_count) - triangular_distance_slot_count(record_count - lhs_position);
		return slots_before_row + (rhs_position - lhs_position - 1);
	}

	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the symmetric distance table");
		}
	}

	auto distance_at_validated_position(std::size_t lhs_position, std::size_t rhs_position) const -> distance_type
	{
		if (lhs_position == rhs_position) {
			return distance_type{};
		}
		return distances_[offset(lhs_position, rhs_position, record_count_)];
	}

	auto compute_source_distance_at_position(std::size_t lhs_position, std::size_t rhs_position) const
		-> distance_type
	{
		return space_->metric()(space_->records().at(lhs_position), space_->records().at(rhs_position));
	}

	const space_type *space_;
	std::size_t record_count_;
	std::size_t version_;
	std::size_t off_diagonal_slots_;
	std::size_t memory_bytes_estimate_;
	std::string metric_key_;
	std::string cache_key_;
	std::vector<RecordId> ids_;
	triangular_detail::record_position_lookup_type id_positions_;
	std::vector<distance_type> distances_;
};

template <typename Space> auto symmetric_distance_table(const Space &space) -> SymmetricDistanceTable<Space>
{
	return SymmetricDistanceTable<Space>(space);
}

} // namespace mtrc::space::storage

#endif
