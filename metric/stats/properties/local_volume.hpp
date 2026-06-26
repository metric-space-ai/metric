// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_PROPERTIES_LOCAL_VOLUME_HPP
#define _METRIC_STATS_PROPERTIES_LOCAL_VOLUME_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/space/chunked.hpp>
#include <metric/space/sample_plan.hpp>
#include <metric/space/storage/implicit.hpp>

namespace mtrc::stats::properties {

template <typename Distance> struct LocalVolumeResult {
	using distance_type = Distance;

	std::vector<std::size_t> counts;
	std::vector<double> densities;
	std::size_t record_count{};
	Distance radius{};
	std::size_t minimum_count{};
	std::size_t maximum_count{};
	double average_count{};
	double minimum_density{};
	double maximum_density{};
	double average_density{};
	std::size_t evaluated_distance_count{};
	std::size_t sample_count{};
	std::size_t sample_universe{};
	bool exact{true};
	std::string algorithm{"local_volume"};
	std::string representation;
	core::ApproximationQuality approximation_quality;

	auto empty() const -> bool { return counts.empty(); }
	auto size() const -> std::size_t { return counts.size(); }
};

struct local_volume_options {
	local_volume_options() = default;

	std::size_t max_exact_records{4096};
	std::size_t sample_count{512};
	bool allow_approximate{true};
	// Maximum helper cells for multi-radius exact/chunked profile work.
	// Set to 0 only when unbounded profile helper storage is intentional.
	std::size_t max_profile_cells{1'000'000};
};

namespace local_volume_detail {

template <typename Value> auto finite_scalar(Value value) -> bool
{
	if constexpr (std::is_floating_point<Value>::value) {
		return std::isfinite(static_cast<double>(value));
	} else {
		(void)value;
		return true;
	}
}

template <typename Distance>
auto make_local_volume_result(std::vector<std::size_t> counts, std::vector<double> densities,
							  std::size_t record_count, Distance radius, std::string representation)
	-> LocalVolumeResult<Distance>
{
	LocalVolumeResult<Distance> result;
	result.counts = std::move(counts);
	result.densities = std::move(densities);
	result.record_count = record_count;
	result.radius = radius;
	result.representation = std::move(representation);

	if (result.counts.empty()) {
		return result;
	}

	result.minimum_count = result.counts.front();
	result.maximum_count = result.counts.front();
	result.minimum_density = result.densities.front();
	result.maximum_density = result.densities.front();

	double count_sum = 0.0;
	double density_sum = 0.0;
	for (std::size_t index = 0; index < result.counts.size(); ++index) {
		const auto count = result.counts[index];
		const auto density = result.densities[index];
		if (count < result.minimum_count) {
			result.minimum_count = count;
		}
		if (result.maximum_count < count) {
			result.maximum_count = count;
		}
		if (density < result.minimum_density) {
			result.minimum_density = density;
		}
		if (result.maximum_density < density) {
			result.maximum_density = density;
		}
		count_sum += static_cast<double>(count);
		density_sum += density;
	}

	result.average_count = count_sum / static_cast<double>(result.counts.size());
	result.average_density = density_sum / static_cast<double>(result.densities.size());
	return result;
}

inline auto should_sample_local_volume(std::size_t record_count, local_volume_options options) -> bool
{
	return options.allow_approximate && record_count > options.max_exact_records;
}

inline auto local_volume_exact_work(std::size_t record_count, const char *operation) -> std::size_t
{
	if (record_count != 0 && record_count > std::numeric_limits<std::size_t>::max() / record_count) {
		throw ::mtrc::RepresentationError(
			std::string(operation) + " exact distance-evaluation estimate exceeds size_t capacity");
	}
	return record_count * record_count;
}

inline auto checked_local_volume_size_product(std::size_t lhs, std::size_t rhs, const char *operation)
	-> std::size_t
{
	if (rhs != 0 && lhs > std::numeric_limits<std::size_t>::max() / rhs) {
		throw ::mtrc::RepresentationError(std::string(operation) + " helper size exceeds size_t capacity");
	}
	return lhs * rhs;
}

inline auto checked_local_volume_size_sum(std::size_t lhs, std::size_t rhs, const char *operation) -> std::size_t
{
	if (lhs > std::numeric_limits<std::size_t>::max() - rhs) {
		throw ::mtrc::RepresentationError(std::string(operation) + " helper size exceeds size_t capacity");
	}
	return lhs + rhs;
}

inline auto require_local_volume_profile_cells(std::size_t record_count, std::size_t radius_columns,
											   std::size_t max_profile_cells, const char *operation)
	-> void
{
	const auto helper_cells = checked_local_volume_size_product(record_count, radius_columns, operation);
	if (max_profile_cells == 0 || helper_cells <= max_profile_cells) {
		return;
	}
	throw ::mtrc::RepresentationError(
		std::string(operation) + " refused profile helper storage before allocation: records=" +
		std::to_string(record_count) + ", radius_columns=" + std::to_string(radius_columns) +
		", helper_cells=" + std::to_string(helper_cells) +
		", max_profile_cells=" + std::to_string(max_profile_cells) +
		". Reduce radii, use a streaming/chunked summary with fewer radii, or set max_profile_cells=0 only "
		"when unbounded profile helper storage is intentional.");
}

inline auto require_exact_local_volume_allowed(std::size_t record_count, local_volume_options options,
											   const char *operation) -> void
{
	if (options.allow_approximate || options.max_exact_records == 0 || record_count <= options.max_exact_records) {
		return;
	}
	const auto estimated = local_volume_exact_work(record_count, operation);
	throw ::mtrc::RepresentationError(
		std::string(operation) + " refused exact local-volume work before distance calls: records=" +
		std::to_string(record_count) + ", estimated_distance_evaluations=" + std::to_string(estimated) +
		", max_exact_records=" + std::to_string(options.max_exact_records) +
		". Enable approximate sampling, raise max_exact_records, or set max_exact_records=0 with "
		"allow_approximate=false only when unbounded exact local-volume work is intentional.");
}

inline auto local_volume_row_count_standard_error(std::size_t observed_hits, std::size_t evaluated_candidates,
												  std::size_t candidate_universe) -> double
{
	if (evaluated_candidates < 2 || candidate_universe < 2) {
		return 0.0;
	}
	const auto sample = static_cast<double>(evaluated_candidates);
	const auto universe = static_cast<double>(candidate_universe);
	const auto p = static_cast<double>(observed_hits) / sample;
	const auto fpc = evaluated_candidates < candidate_universe ? (universe - sample) / (universe - 1.0) : 0.0;
	return universe * std::sqrt((p * (1.0 - p) / sample) * fpc);
}

inline auto average_local_volume_count_standard_error(const std::vector<std::size_t> &observed_hits_by_record,
													  const std::vector<std::size_t> &evaluated_by_record,
													  std::size_t candidate_universe) -> double
{
	if (observed_hits_by_record.empty()) {
		return 0.0;
	}
	double standard_error_sum = 0.0;
	for (std::size_t index = 0; index < observed_hits_by_record.size(); ++index) {
		standard_error_sum += local_volume_row_count_standard_error(
			observed_hits_by_record[index], evaluated_by_record[index], candidate_universe);
	}
	return standard_error_sum / static_cast<double>(observed_hits_by_record.size());
}

template <typename Result>
auto mark_local_volume_pair_approximation(Result &result, std::size_t evaluated_distance_count,
										  std::size_t candidate_universe, const char *candidate_policy,
										  const char *reason) -> void
{
	result.evaluated_distance_count = evaluated_distance_count;
	result.sample_count = evaluated_distance_count;
	result.sample_universe = candidate_universe;
	result.approximation_quality.diagnostic = "local_volume_approximation";
	result.approximation_quality.candidate_policy = candidate_policy;
	result.approximation_quality.candidate_count = evaluated_distance_count;
	result.approximation_quality.candidate_universe = candidate_universe;
	result.approximation_quality.distance_evaluations = evaluated_distance_count;
	result.approximation_quality.sample_count = evaluated_distance_count;
	result.approximation_quality.sample_universe = candidate_universe;
	result.approximation_quality.candidate_fraction =
		candidate_universe == 0
			? 1.0
			: static_cast<double>(evaluated_distance_count) / static_cast<double>(candidate_universe);
	result.approximation_quality.sample_fraction = result.approximation_quality.candidate_fraction;
	result.approximation_quality.reason = reason;
}

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto sampled_local_volume(const Provider &provider, Radius radius, local_volume_options options)
	-> LocalVolumeResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;

	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}
	if (!finite_scalar(radius)) {
		throw std::invalid_argument("radius must be finite");
	}
	if (options.sample_count == 0) {
		throw std::invalid_argument("local_volume sample_count must be >= 1 when sampling is enabled");
	}

	const auto record_count = provider.record_count();
	std::vector<std::size_t> counts(record_count, 0);
	std::vector<double> densities(record_count, 0.0);
	if (record_count == 0) {
		auto result = make_local_volume_result(
			std::move(counts), std::move(densities), record_count, static_cast<distance_type>(radius),
			"sampled_metric_space");
		result.exact = false;
		result.algorithm = "sampled_local_volume";
		return result;
	}
	if (record_count == 1) {
		auto result = make_local_volume_result(
			std::vector<std::size_t>{1}, std::vector<double>{1.0}, record_count,
			static_cast<distance_type>(radius), "sampled_metric_space");
		result.sample_universe = 0;
		result.exact = false;
		result.algorithm = "sampled_local_volume";
		return result;
	}

	const auto threshold = static_cast<comparison_type>(radius);
	const auto target_sample_count = std::min(options.sample_count, record_count - 1);
	double standard_error_sum = 0.0;
	for (std::size_t source = 0; source < record_count; ++source) {
		const auto source_id = provider.id(source);
		const auto sample_plan =
			::mtrc::space::regular_sample_positions_excluding(record_count, source, target_sample_count);
		std::size_t sampled_hits = 0;
		for (const auto target : sample_plan.positions) {
			const auto distance = provider.distance(source_id, provider.id(target));
			if (!finite_scalar(distance)) {
				throw std::invalid_argument("distance values must be finite");
			}
			if (static_cast<comparison_type>(distance) <= threshold) {
				++sampled_hits;
			}
		}

		const auto universe = static_cast<double>(record_count - 1);
		const auto sample = static_cast<double>(sample_plan.positions.size());
		const auto estimated_other_count =
			sample == 0.0 ? 0.0 : static_cast<double>(sampled_hits) * universe / sample;
		auto estimated_count = static_cast<std::size_t>(std::llround(1.0 + estimated_other_count));
		if (estimated_count > record_count) {
			estimated_count = record_count;
		}
		counts[source] = estimated_count;
		densities[source] = static_cast<double>(estimated_count) / static_cast<double>(record_count);

		if (sample > 1.0 && universe > 1.0) {
			const auto p = static_cast<double>(sampled_hits) / sample;
			const auto fpc = sample < universe ? (universe - sample) / (universe - 1.0) : 0.0;
			standard_error_sum += universe * std::sqrt((p * (1.0 - p) / sample) * fpc);
		}
	}

	auto result = make_local_volume_result(
		std::move(counts), std::move(densities), record_count, static_cast<distance_type>(radius),
		"sampled_metric_space");
	result.exact = false;
	result.algorithm = "sampled_local_volume";
	result.sample_count = target_sample_count;
	result.sample_universe = record_count - 1;
	result.evaluated_distance_count = record_count * target_sample_count;
	result.approximation_quality.diagnostic = "local_volume_approximation";
	result.approximation_quality.candidate_policy = "regular_target_sample";
	result.approximation_quality.candidate_count = target_sample_count;
	result.approximation_quality.candidate_universe = record_count - 1;
	result.approximation_quality.distance_evaluations = result.evaluated_distance_count;
	result.approximation_quality.sample_count = target_sample_count;
	result.approximation_quality.sample_universe = record_count - 1;
	result.approximation_quality.candidate_fraction =
		record_count <= 1 ? 1.0 : static_cast<double>(target_sample_count) / static_cast<double>(record_count - 1);
	result.approximation_quality.sample_fraction = result.approximation_quality.candidate_fraction;
	result.approximation_quality.standard_error =
		record_count == 0 ? 0.0 : standard_error_sum / static_cast<double>(record_count);
	result.approximation_quality.confidence_radius_95 = 1.96 * result.approximation_quality.standard_error;
	result.approximation_quality.reason =
		"local volume estimated from a deterministic regular target sample per source record";
	return result;
}

} // namespace local_volume_detail

// Local volume counts how many records lie in each closed metric ball B(x, r),
// including x itself. Density is that count normalized by the finite record count.
template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto exact_local_volume(const Provider &provider, Radius radius) -> LocalVolumeResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;

	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}
	if (!local_volume_detail::finite_scalar(radius)) {
		throw std::invalid_argument("radius must be finite");
	}

	const auto record_count = provider.record_count();
	std::vector<std::size_t> counts(record_count, 0);
	std::vector<double> densities(record_count, 0.0);
	if (record_count == 0) {
		return local_volume_detail::make_local_volume_result(
			std::move(counts), std::move(densities), record_count, static_cast<distance_type>(radius),
			"pairwise_distances");
	}

	const auto threshold = static_cast<comparison_type>(radius);
	for (std::size_t source = 0; source < record_count; ++source) {
		const auto source_id = provider.id(source);
		for (std::size_t target = 0; target < record_count; ++target) {
			const auto distance = provider.distance(source_id, provider.id(target));
			if (!local_volume_detail::finite_scalar(distance)) {
				throw std::invalid_argument("distance values must be finite");
			}
			if (static_cast<comparison_type>(distance) <= threshold) {
				++counts[source];
			}
		}
		densities[source] = static_cast<double>(counts[source]) / static_cast<double>(record_count);
	}

	auto result = local_volume_detail::make_local_volume_result(
		std::move(counts), std::move(densities), record_count, static_cast<distance_type>(radius),
		"pairwise_distances");
	result.evaluated_distance_count = record_count * record_count;
	result.sample_count = record_count;
	result.sample_universe = record_count;
	return result;
}

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto local_volume(const Provider &provider, Radius radius, local_volume_options options)
	-> LocalVolumeResult<typename Provider::distance_type>
{
	if (local_volume_detail::should_sample_local_volume(provider.record_count(), options)) {
		return local_volume_detail::sampled_local_volume(provider, radius, options);
	}
	local_volume_detail::require_exact_local_volume_allowed(provider.record_count(), options, "local_volume");
	return exact_local_volume(provider, radius);
}

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto local_volume(const Provider &provider, Radius radius) -> LocalVolumeResult<typename Provider::distance_type>
{
	return local_volume(provider, radius, local_volume_options{});
}

template <typename Space, typename Radius>
auto chunked_local_volume(const ::mtrc::space::ChunkedSpaceView<Space> &chunks, Radius radius)
	-> LocalVolumeResult<typename ::mtrc::space::ChunkedSpaceView<Space>::distance_type>
{
	using distance_type = typename ::mtrc::space::ChunkedSpaceView<Space>::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;

	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}
	if (!local_volume_detail::finite_scalar(radius)) {
		throw std::invalid_argument("radius must be finite");
	}

	const auto plan = chunks.plan_diagnostics();
	std::vector<std::size_t> counts(plan.record_count, plan.record_count == 0 ? 0 : 1);
	std::vector<double> densities(plan.record_count, 0.0);
	std::vector<std::size_t> evaluated_by_record(plan.record_count, 0);
	std::vector<std::size_t> observed_hits_by_record(plan.record_count, 0);
	const auto threshold = static_cast<comparison_type>(radius);

	auto add_pair = [&](auto lhs, auto rhs, distance_type distance) {
		if (!local_volume_detail::finite_scalar(distance)) {
			throw std::invalid_argument("chunked_local_volume requires finite distance values");
		}
		const auto lhs_position = chunks.global_position(lhs);
		const auto rhs_position = chunks.global_position(rhs);
		++evaluated_by_record[lhs_position];
		++evaluated_by_record[rhs_position];
		if (static_cast<comparison_type>(distance) <= threshold) {
			++counts[lhs_position];
			++counts[rhs_position];
			++observed_hits_by_record[lhs_position];
			++observed_hits_by_record[rhs_position];
		}
	};

	const auto local_pairs = chunks.for_each_local_pair(
		[&](std::size_t, auto lhs, auto rhs, distance_type distance) { add_pair(lhs, rhs, distance); });
	const auto representative_pairs = chunks.for_each_representative_pair(
		[&](std::size_t, std::size_t, auto lhs, auto rhs, distance_type distance) {
			add_pair(lhs, rhs, distance);
		});

	if (plan.record_count > 0) {
		for (std::size_t index = 0; index < counts.size(); ++index) {
			densities[index] = static_cast<double>(counts[index]) / static_cast<double>(plan.record_count);
		}
	}

	auto result = local_volume_detail::make_local_volume_result(
		std::move(counts), std::move(densities), plan.record_count, static_cast<distance_type>(radius),
		"chunked_space_view");
	result.exact = false;
	result.algorithm = "chunked_local_volume";
	local_volume_detail::mark_local_volume_pair_approximation(
		result, local_pairs + representative_pairs, plan.dense_pair_distance_evaluations,
		"local_chunks_plus_representative_pairs",
		"local volume estimated from exact local chunk pairs plus chunk representative pairs");
	const auto candidate_universe_per_record = plan.record_count == 0 ? 0 : plan.record_count - 1;
	result.approximation_quality.standard_error = local_volume_detail::average_local_volume_count_standard_error(
		observed_hits_by_record, evaluated_by_record, candidate_universe_per_record);
	result.approximation_quality.confidence_radius_95 = 1.96 * result.approximation_quality.standard_error;
	return result;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto local_volume(const Space &space, Radius radius, local_volume_options options = {})
	-> LocalVolumeResult<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = local_volume(provider, radius, options);
	result.representation = result.exact ? "metric_space" : "sampled_metric_space";
	return result;
}

template <typename Container, typename Metric, typename Radius,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto local_volume(const Container &records, const Metric &metric, Radius radius, local_volume_options options = {})
	-> LocalVolumeResult<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = local_volume(space, radius, options);
	result.representation = result.exact ? "records" : "sampled_metric_space";
	return result;
}

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto density(const Provider &provider, Radius radius) -> LocalVolumeResult<typename Provider::distance_type>
{
	auto result = local_volume(provider, radius);
	result.algorithm = "density";
	return result;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto density(const Space &space, Radius radius) -> LocalVolumeResult<typename Space::distance_type>
{
	auto result = local_volume(space, radius);
	result.algorithm = "density";
	return result;
}

template <typename Container, typename Metric, typename Radius,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto density(const Container &records, const Metric &metric, Radius radius)
	-> LocalVolumeResult<metric_result_t<Metric, Record>>
{
	auto result = local_volume(records, metric, radius);
	result.algorithm = "density";
	return result;
}

// A multi-radius local-volume profile summarizes how local ball occupancy grows with
// radius across a finite metric space. Each entry is the per-record-count summary of
// local_volume at one radius; together they trace the local growth of the space without
// the manifold-dimension assumptions of intrinsic_dimension. This is a read-only
// diagnostic: it never builds a metric or a derived space.
template <typename Distance> struct LocalVolumeProfileEntry {
	using distance_type = Distance;

	Distance radius{};
	std::size_t minimum_count{};
	std::size_t maximum_count{};
	double average_count{};
	double minimum_density{};
	double maximum_density{};
	double average_density{};
};

template <typename Distance> struct LocalVolumeProfile {
	using distance_type = Distance;

	std::vector<LocalVolumeProfileEntry<Distance>> entries;
	std::size_t record_count{};
	std::size_t evaluated_distance_count{};
	std::size_t sample_count{};
	std::size_t sample_universe{};
	bool exact{true};
	std::string algorithm{"local_volume_profile"};
	std::string representation;
	core::ApproximationQuality approximation_quality;

	auto empty() const -> bool { return entries.empty(); }
	auto size() const -> std::size_t { return entries.size(); }
};

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto exact_local_volume_profile(const Provider &provider, const std::vector<Radius> &radii,
								local_volume_options options = {})
	-> LocalVolumeProfile<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;

	LocalVolumeProfile<distance_type> profile;
	profile.record_count = provider.record_count();
	profile.representation = "pairwise_distances";
	profile.entries.reserve(radii.size());
	if (radii.empty()) {
		return profile;
	}

	struct radius_threshold {
		comparison_type threshold{};
		distance_type radius{};
		std::size_t original_index{};
	};

	std::vector<radius_threshold> thresholds;
	thresholds.reserve(radii.size());
	for (std::size_t index = 0; index < radii.size(); ++index) {
		const auto radius = radii[index];
		if (radius < Radius{}) {
			throw std::invalid_argument("radius must be non-negative");
		}
		if (!local_volume_detail::finite_scalar(radius)) {
			throw std::invalid_argument("radius must be finite");
		}
		thresholds.push_back(
			radius_threshold{static_cast<comparison_type>(radius), static_cast<distance_type>(radius), index});
	}
	local_volume_detail::require_local_volume_profile_cells(
		provider.record_count(), radii.size(), options.max_profile_cells, "exact_local_volume_profile");
	profile.evaluated_distance_count = provider.record_count() * provider.record_count();
	profile.sample_count = provider.record_count();
	profile.sample_universe = provider.record_count();
	std::sort(thresholds.begin(), thresholds.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.threshold < rhs.threshold) {
			return true;
		}
		if (rhs.threshold < lhs.threshold) {
			return false;
		}
		return lhs.original_index < rhs.original_index;
	});

	std::vector<std::vector<std::size_t>> counts_by_radius(
		radii.size(), std::vector<std::size_t>(provider.record_count(), 0));
	std::vector<std::size_t> threshold_deltas(thresholds.size() + 1, 0);
	for (std::size_t source = 0; source < provider.record_count(); ++source) {
		std::fill(threshold_deltas.begin(), threshold_deltas.end(), std::size_t{0});
		const auto source_id = provider.id(source);
		for (std::size_t target = 0; target < provider.record_count(); ++target) {
			const auto distance = provider.distance(source_id, provider.id(target));
			if (!local_volume_detail::finite_scalar(distance)) {
				throw std::invalid_argument("distance values must be finite");
			}
			const auto threshold = static_cast<comparison_type>(distance);
			const auto first_matching = std::lower_bound(
				thresholds.begin(), thresholds.end(), threshold,
				[](const auto &entry, const auto &value) { return entry.threshold < value; });
			if (first_matching != thresholds.end()) {
				++threshold_deltas[static_cast<std::size_t>(first_matching - thresholds.begin())];
			}
		}

		std::size_t count = 0;
		for (std::size_t sorted_index = 0; sorted_index < thresholds.size(); ++sorted_index) {
			count += threshold_deltas[sorted_index];
			counts_by_radius[thresholds[sorted_index].original_index][source] = count;
		}
	}

	for (std::size_t radius_index = 0; radius_index < radii.size(); ++radius_index) {
		std::vector<double> densities(provider.record_count(), 0.0);
		if (provider.record_count() > 0) {
			for (std::size_t index = 0; index < provider.record_count(); ++index) {
				densities[index] =
					static_cast<double>(counts_by_radius[radius_index][index]) /
					static_cast<double>(provider.record_count());
			}
		}
		const auto volume = local_volume_detail::make_local_volume_result(
			std::move(counts_by_radius[radius_index]), std::move(densities), provider.record_count(),
			static_cast<distance_type>(radii[radius_index]), "pairwise_distances");
		LocalVolumeProfileEntry<distance_type> entry;
		entry.radius = volume.radius;
		entry.minimum_count = volume.minimum_count;
		entry.maximum_count = volume.maximum_count;
		entry.average_count = volume.average_count;
		entry.minimum_density = volume.minimum_density;
		entry.maximum_density = volume.maximum_density;
		entry.average_density = volume.average_density;
		profile.entries.push_back(entry);
	}

	return profile;
}

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto sampled_local_volume_profile(const Provider &provider, const std::vector<Radius> &radii,
								  local_volume_options options)
	-> LocalVolumeProfile<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;

	if (options.sample_count == 0) {
		throw std::invalid_argument("local_volume_profile sample_count must be >= 1 when sampling is enabled");
	}

	LocalVolumeProfile<distance_type> profile;
	const auto record_count = provider.record_count();
	profile.record_count = record_count;
	profile.exact = false;
	profile.algorithm = "sampled_local_volume_profile";
	profile.representation = "sampled_metric_space";
	profile.entries.resize(radii.size());
	if (radii.empty()) {
		return profile;
	}

	struct radius_threshold {
		comparison_type threshold{};
		distance_type radius{};
		std::size_t original_index{};
	};

	std::vector<radius_threshold> thresholds;
	thresholds.reserve(radii.size());
	for (std::size_t index = 0; index < radii.size(); ++index) {
		const auto radius = radii[index];
		if (radius < Radius{}) {
			throw std::invalid_argument("radius must be non-negative");
		}
		if (!local_volume_detail::finite_scalar(radius)) {
			throw std::invalid_argument("radius must be finite");
		}
		profile.entries[index].radius = static_cast<distance_type>(radius);
		thresholds.push_back(
			radius_threshold{static_cast<comparison_type>(radius), static_cast<distance_type>(radius), index});
	}

	if (record_count == 0) {
		return profile;
	}
	if (record_count == 1) {
		for (auto &entry : profile.entries) {
			entry.minimum_count = 1;
			entry.maximum_count = 1;
			entry.average_count = 1.0;
			entry.minimum_density = 1.0;
			entry.maximum_density = 1.0;
			entry.average_density = 1.0;
		}
		return profile;
	}

	std::sort(thresholds.begin(), thresholds.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.threshold < rhs.threshold) {
			return true;
		}
		if (rhs.threshold < lhs.threshold) {
			return false;
		}
		return lhs.original_index < rhs.original_index;
	});

	const auto target_sample_count = std::min(options.sample_count, record_count - 1);
	const auto candidate_universe = record_count - 1;
	std::vector<double> standard_error_sums(profile.entries.size(), 0.0);
	for (auto &entry : profile.entries) {
		entry.minimum_count = std::numeric_limits<std::size_t>::max();
		entry.minimum_density = std::numeric_limits<double>::infinity();
	}

	for (std::size_t source = 0; source < record_count; ++source) {
		const auto source_id = provider.id(source);
		const auto sample_plan =
			::mtrc::space::regular_sample_positions_excluding(record_count, source, target_sample_count);
		std::vector<std::size_t> threshold_deltas(thresholds.size() + 1, 0);
		for (const auto target : sample_plan.positions) {
			const auto distance = provider.distance(source_id, provider.id(target));
			if (!local_volume_detail::finite_scalar(distance)) {
				throw std::invalid_argument("distance values must be finite");
			}
			const auto threshold = static_cast<comparison_type>(distance);
			const auto first_matching = std::lower_bound(
				thresholds.begin(), thresholds.end(), threshold,
				[](const auto &entry, const auto &value) { return entry.threshold < value; });
			if (first_matching != thresholds.end()) {
				++threshold_deltas[static_cast<std::size_t>(first_matching - thresholds.begin())];
			}
		}

		std::size_t observed_hits = 0;
		for (std::size_t sorted_index = 0; sorted_index < thresholds.size(); ++sorted_index) {
			observed_hits += threshold_deltas[sorted_index];
			const auto sample = static_cast<double>(sample_plan.positions.size());
			const auto estimated_other_count =
				sample == 0.0 ? 0.0 : static_cast<double>(observed_hits) *
										static_cast<double>(candidate_universe) / sample;
			auto estimated_count = static_cast<std::size_t>(std::llround(1.0 + estimated_other_count));
			if (estimated_count > record_count) {
				estimated_count = record_count;
			}
			auto &entry = profile.entries[thresholds[sorted_index].original_index];
			const auto density = static_cast<double>(estimated_count) / static_cast<double>(record_count);
			if (estimated_count < entry.minimum_count) {
				entry.minimum_count = estimated_count;
			}
			if (entry.maximum_count < estimated_count) {
				entry.maximum_count = estimated_count;
			}
			if (density < entry.minimum_density) {
				entry.minimum_density = density;
			}
			if (entry.maximum_density < density) {
				entry.maximum_density = density;
			}
			entry.average_count += static_cast<double>(estimated_count);
			entry.average_density += density;
			standard_error_sums[thresholds[sorted_index].original_index] +=
				local_volume_detail::local_volume_row_count_standard_error(
					observed_hits, sample_plan.positions.size(), candidate_universe);
		}
	}

	double maximum_standard_error = 0.0;
	for (std::size_t index = 0; index < profile.entries.size(); ++index) {
		auto &entry = profile.entries[index];
		entry.average_count /= static_cast<double>(record_count);
		entry.average_density /= static_cast<double>(record_count);
		const auto standard_error = standard_error_sums[index] / static_cast<double>(record_count);
		maximum_standard_error = std::max(maximum_standard_error, standard_error);
	}

	profile.sample_count = target_sample_count;
	profile.sample_universe = candidate_universe;
	profile.evaluated_distance_count = record_count * target_sample_count;
	profile.approximation_quality.diagnostic = "local_volume_approximation";
	profile.approximation_quality.candidate_policy = "regular_target_sample";
	profile.approximation_quality.candidate_count = target_sample_count;
	profile.approximation_quality.candidate_universe = candidate_universe;
	profile.approximation_quality.distance_evaluations = profile.evaluated_distance_count;
	profile.approximation_quality.sample_count = target_sample_count;
	profile.approximation_quality.sample_universe = candidate_universe;
	profile.approximation_quality.candidate_fraction =
		static_cast<double>(target_sample_count) / static_cast<double>(candidate_universe);
	profile.approximation_quality.sample_fraction = profile.approximation_quality.candidate_fraction;
	profile.approximation_quality.standard_error = maximum_standard_error;
	profile.approximation_quality.confidence_radius_95 = 1.96 * maximum_standard_error;
	profile.approximation_quality.reason =
		"local volume profile estimated from one deterministic regular target sample per source record";
	return profile;
}

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto local_volume_profile(const Provider &provider, const std::vector<Radius> &radii,
						  local_volume_options options = {})
	-> LocalVolumeProfile<typename Provider::distance_type>
{
	if (local_volume_detail::should_sample_local_volume(provider.record_count(), options)) {
		return sampled_local_volume_profile(provider, radii, options);
	}
	local_volume_detail::require_exact_local_volume_allowed(
		provider.record_count(), options, "local_volume_profile");
	return exact_local_volume_profile(provider, radii, options);
}

template <typename Space, typename Radius>
auto chunked_local_volume_profile(const ::mtrc::space::ChunkedSpaceView<Space> &chunks,
								  const std::vector<Radius> &radii, local_volume_options options = {})
	-> LocalVolumeProfile<typename ::mtrc::space::ChunkedSpaceView<Space>::distance_type>
{
	using distance_type = typename ::mtrc::space::ChunkedSpaceView<Space>::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;

	const auto plan = chunks.plan_diagnostics();
	LocalVolumeProfile<distance_type> profile;
	profile.record_count = plan.record_count;
	profile.exact = false;
	profile.algorithm = "chunked_local_volume_profile";
	profile.representation = "chunked_space_view";
	profile.entries.resize(radii.size());

	struct radius_threshold {
		comparison_type threshold{};
		distance_type radius{};
		std::size_t original_index{};
	};

	std::vector<radius_threshold> thresholds;
	thresholds.reserve(radii.size());
	for (std::size_t index = 0; index < radii.size(); ++index) {
		const auto radius = radii[index];
		if (radius < Radius{}) {
			throw std::invalid_argument("radius must be non-negative");
		}
		if (!local_volume_detail::finite_scalar(radius)) {
			throw std::invalid_argument("radius must be finite");
		}
		profile.entries[index].radius = static_cast<distance_type>(radius);
		thresholds.push_back(
			radius_threshold{static_cast<comparison_type>(radius), static_cast<distance_type>(radius), index});
	}

	if (thresholds.empty()) {
		local_volume_detail::mark_local_volume_pair_approximation(
			profile, 0, plan.dense_pair_distance_evaluations, "local_chunks_plus_representative_pairs",
			"local volume profile estimated from exact local chunk pairs plus chunk representative pairs");
		return profile;
	}
	const auto helper_columns = local_volume_detail::checked_local_volume_size_sum(
		thresholds.size(), std::size_t{1}, "chunked_local_volume_profile");
	local_volume_detail::require_local_volume_profile_cells(
		plan.record_count, helper_columns, options.max_profile_cells, "chunked_local_volume_profile");

	std::sort(thresholds.begin(), thresholds.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.threshold < rhs.threshold) {
			return true;
		}
		if (rhs.threshold < lhs.threshold) {
			return false;
		}
		return lhs.original_index < rhs.original_index;
	});

	std::vector<std::vector<std::size_t>> threshold_deltas(
		plan.record_count, std::vector<std::size_t>(thresholds.size() + 1, 0));
	std::vector<std::size_t> evaluated_by_record(plan.record_count, 0);

	auto add_pair = [&](auto lhs, auto rhs, distance_type distance) {
		if (!local_volume_detail::finite_scalar(distance)) {
			throw std::invalid_argument("chunked_local_volume_profile requires finite distance values");
		}
		const auto lhs_position = chunks.global_position(lhs);
		const auto rhs_position = chunks.global_position(rhs);
		++evaluated_by_record[lhs_position];
		++evaluated_by_record[rhs_position];
		const auto threshold = static_cast<comparison_type>(distance);
		const auto first_matching = std::lower_bound(
			thresholds.begin(), thresholds.end(), threshold,
			[](const auto &entry, const auto &value) { return entry.threshold < value; });
		if (first_matching == thresholds.end()) {
			return;
		}
		const auto sorted_index = static_cast<std::size_t>(first_matching - thresholds.begin());
		++threshold_deltas[lhs_position][sorted_index];
		++threshold_deltas[rhs_position][sorted_index];
	};

	const auto local_pairs = chunks.for_each_local_pair(
		[&](std::size_t, auto lhs, auto rhs, distance_type distance) { add_pair(lhs, rhs, distance); });
	const auto representative_pairs = chunks.for_each_representative_pair(
		[&](std::size_t, std::size_t, auto lhs, auto rhs, distance_type distance) {
			add_pair(lhs, rhs, distance);
		});

	if (plan.record_count > 0) {
		std::vector<double> standard_error_sums(profile.entries.size(), 0.0);
		const auto candidate_universe_per_record = plan.record_count - 1;
		for (auto &entry : profile.entries) {
			entry.minimum_count = std::numeric_limits<std::size_t>::max();
			entry.minimum_density = std::numeric_limits<double>::infinity();
		}
		for (std::size_t record_index = 0; record_index < plan.record_count; ++record_index) {
			std::size_t count = 1;
			std::size_t observed_hits = 0;
			for (std::size_t sorted_index = 0; sorted_index < thresholds.size(); ++sorted_index) {
				observed_hits += threshold_deltas[record_index][sorted_index];
				count = 1 + observed_hits;
				auto &entry = profile.entries[thresholds[sorted_index].original_index];
				const auto density = static_cast<double>(count) / static_cast<double>(plan.record_count);
				if (count < entry.minimum_count) {
					entry.minimum_count = count;
				}
				if (entry.maximum_count < count) {
					entry.maximum_count = count;
				}
				if (density < entry.minimum_density) {
					entry.minimum_density = density;
				}
				if (entry.maximum_density < density) {
					entry.maximum_density = density;
				}
				entry.average_count += static_cast<double>(count);
				entry.average_density += density;
				standard_error_sums[thresholds[sorted_index].original_index] +=
					local_volume_detail::local_volume_row_count_standard_error(
						observed_hits, evaluated_by_record[record_index], candidate_universe_per_record);
			}
		}
		double maximum_standard_error = 0.0;
		for (auto &entry : profile.entries) {
			entry.average_count /= static_cast<double>(plan.record_count);
			entry.average_density /= static_cast<double>(plan.record_count);
		}
		for (const auto standard_error_sum : standard_error_sums) {
			maximum_standard_error =
				std::max(maximum_standard_error, standard_error_sum / static_cast<double>(plan.record_count));
		}
		profile.approximation_quality.standard_error = maximum_standard_error;
		profile.approximation_quality.confidence_radius_95 = 1.96 * maximum_standard_error;
	}

	local_volume_detail::mark_local_volume_pair_approximation(
		profile, local_pairs + representative_pairs, plan.dense_pair_distance_evaluations,
		"local_chunks_plus_representative_pairs",
		"local volume profile estimated from exact local chunk pairs plus chunk representative pairs");
	if (plan.record_count > 0) {
		const auto maximum_standard_error = profile.approximation_quality.standard_error;
		profile.approximation_quality.confidence_radius_95 = 1.96 * maximum_standard_error;
	}
	return profile;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto local_volume_profile(const Space &space, const std::vector<Radius> &radii, local_volume_options options = {})
	-> LocalVolumeProfile<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto profile = local_volume_profile(provider, radii, options);
	profile.representation = profile.exact ? "metric_space" : "sampled_metric_space";
	return profile;
}

template <typename Container, typename Metric, typename Radius,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto local_volume_profile(const Container &records, const Metric &metric, const std::vector<Radius> &radii,
						  local_volume_options options = {})
	-> LocalVolumeProfile<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto profile = local_volume_profile(space, radii, options);
	profile.representation = profile.exact ? "records" : "sampled_metric_space";
	return profile;
}

} // namespace mtrc::stats::properties

namespace mtrc::stats {
using properties::chunked_local_volume;
using properties::chunked_local_volume_profile;
using properties::density;
using properties::local_volume;
using properties::local_volume_options;
using properties::local_volume_profile;
template <typename Distance> using LocalVolumeResult = properties::LocalVolumeResult<Distance>;
template <typename Distance> using LocalVolumeProfile = properties::LocalVolumeProfile<Distance>;
template <typename Distance> using LocalVolumeProfileEntry = properties::LocalVolumeProfileEntry<Distance>;
} // namespace mtrc::stats

namespace mtrc {
using stats::properties::chunked_local_volume;
using stats::properties::chunked_local_volume_profile;
using stats::properties::density;
using stats::properties::local_volume;
using stats::properties::local_volume_options;
using stats::properties::local_volume_profile;
} // namespace mtrc

#endif
