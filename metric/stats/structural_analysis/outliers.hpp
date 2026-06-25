// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_STRUCTURAL_ANALYSIS_OUTLIERS_HPP
#define _METRIC_STATS_STRUCTURAL_ANALYSIS_OUTLIERS_HPP

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/stats/structural_analysis/clustering.hpp>
#include <metric/stats/structural_analysis/groups.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/stats/structural_analysis/options.hpp>

namespace mtrc::stats::structural_analysis {
namespace outlier_detail {

inline auto validate_nearest_neighbor_outlier_request(std::size_t record_count, std::size_t k) -> void
{
	if (k == 0) {
		throw std::invalid_argument("nearest_neighbor_outliers requires k >= 1");
	}
	if (record_count == 0) {
		throw std::invalid_argument("cannot score outliers in an empty distance provider");
	}
	if (k > record_count - 1) {
		throw std::invalid_argument("k must not exceed record_count - 1");
	}
}

inline auto bounded_sample_too_small_message(std::size_t record_count, std::size_t k, std::size_t sample_count)
	-> std::string
{
	return "nearest_neighbor_outliers default approximate path cannot score k=" + std::to_string(k) +
		   " with bounded sample_count=" + std::to_string(sample_count) +
		   " for record_count=" + std::to_string(record_count) +
		   "; use nearest_neighbor_outliers(provider, k) or nearest_neighbor_outliers(space, k, "
		   "space::storage::exact()) for an explicit exact scan";
}

inline auto outlier_budget_error_message(const space::storage::execution_plan &plan) -> std::string
{
	auto message = plan.reason.empty() ? std::string("outlier runtime policy refused by resource budget") : plan.reason;
	if (!plan.fallback_hint.empty()) {
		message += "; fallback: " + plan.fallback_hint;
	}
	return message;
}

template <typename Space>
auto require_outlier_budget_plan(const Space &space, space::storage::policy runtime_policy)
	-> space::storage::execution_plan
{
	const auto plan = space::storage::estimate_cost(space, "outliers", runtime_policy);
	if (plan.refused) {
		throw ::mtrc::RepresentationError(outlier_budget_error_message(plan));
	}
	if (plan.downgraded && !plan.exact) {
		return plan;
	}
	if (plan.downgraded && !space::storage::uses_blocked_exact_fallback(plan)) {
		throw ::mtrc::RepresentationError(outlier_budget_error_message(plan) +
										  "; executor integration for this downgraded route is not implemented");
	}
	return plan;
}

inline auto sampled_outlier_distance_evaluations(std::size_t record_count, std::size_t sample_count)
	-> std::size_t
{
	if (record_count == 0 || sample_count == 0) {
		return 0;
	}
	if (record_count > std::numeric_limits<std::size_t>::max() / sample_count) {
		return std::numeric_limits<std::size_t>::max();
	}
	const auto total = record_count * sample_count;
	return total > sample_count ? total - sample_count : 0;
}

template <typename Score>
auto mark_sampled_nearest_outlier_quality(core::OutlierResult<Score> &result, std::size_t sample_count,
										  std::size_t k) -> void
{
	const auto candidate_universe = result.record_count > 0 ? result.record_count - 1 : 0;
	const auto candidate_count = std::min(sample_count, candidate_universe);
	auto &quality = result.approximation_quality;
	quality.diagnostic = "outlier_approximation";
	quality.candidate_policy = "regular_sample";
	quality.candidate_count = candidate_count;
	quality.candidate_universe = candidate_universe;
	quality.distance_evaluations = sampled_outlier_distance_evaluations(result.record_count, sample_count);
	quality.requested_count = k;
	quality.sample_count = sample_count;
	quality.sample_universe = result.record_count;
	quality.candidate_fraction = candidate_universe == 0
									  ? 1.0
									  : static_cast<double>(candidate_count) /
											static_cast<double>(candidate_universe);
	quality.sample_fraction = result.record_count == 0
								  ? 1.0
								  : static_cast<double>(sample_count) / static_cast<double>(result.record_count);
	quality.reason =
		"nearest-neighbor outlier scores estimated against a bounded deterministic record sample";
}

template <typename Provider>
auto guarded_nearest_distance_to_positions(const Provider &provider, std::size_t source_position,
										   const std::vector<std::size_t> &record_positions,
										   const char *empty_message,
										   space::storage::runtime_guard runtime,
										   const char *operation)
	-> typename Provider::distance_type
{
	if (record_positions.empty()) {
		throw std::invalid_argument(empty_message);
	}

	auto best_distance = group_detail::guarded_distance_at_position(provider, source_position,
																	record_positions.front(), runtime, operation);
	for (std::size_t index = 1; index < record_positions.size(); ++index) {
		const auto distance = group_detail::guarded_distance_at_position(provider, source_position,
																		 record_positions[index], runtime, operation);
		if (distance < best_distance) {
			best_distance = distance;
		}
	}
	return best_distance;
}

template <typename Provider>
auto guarded_nearest_distance_to_record_ids(const Provider &provider, RecordId source_id,
											const std::vector<RecordId> &record_ids,
											const char *empty_message,
											space::storage::runtime_guard runtime,
											const char *operation)
	-> typename Provider::distance_type
{
	if (record_ids.empty()) {
		throw std::invalid_argument(empty_message);
	}

	std::vector<std::size_t> record_positions;
	record_positions.reserve(record_ids.size());
	for (const auto id : record_ids) {
		record_positions.push_back(provider.position_of(id));
	}
	return guarded_nearest_distance_to_positions(provider, provider.position_of(source_id), record_positions,
												empty_message, runtime, operation);
}

template <typename Provider>
auto guarded_nearest_other_record_distance_at_position_or(const Provider &provider, std::size_t source_position,
														  typename Provider::distance_type fallback,
														  space::storage::runtime_guard runtime,
														  const char *operation) -> typename Provider::distance_type
{
	auto best_distance = fallback;
	bool has_best = false;
	for (std::size_t position = 0; position < provider.record_count(); ++position) {
		if (position == source_position) {
			continue;
		}
		const auto distance = group_detail::guarded_distance_at_position(provider, source_position, position,
																		 runtime, operation);
		if (!has_best || distance < best_distance) {
			best_distance = distance;
			has_best = true;
		}
	}
	return best_distance;
}

template <typename Provider>
auto guarded_nearest_other_record_distance_or(const Provider &provider, RecordId source_id,
											  typename Provider::distance_type fallback,
											  space::storage::runtime_guard runtime,
											  const char *operation) -> typename Provider::distance_type
{
	return guarded_nearest_other_record_distance_at_position_or(provider, provider.position_of(source_id),
															   fallback, runtime, operation);
}

template <typename Provider>
auto kth_nearest_other_position_distance(const Provider &provider, std::size_t source_position, std::size_t k,
										 space::storage::runtime_guard runtime,
										 const char *operation) -> typename Provider::distance_type
{
	using distance_type = typename Provider::distance_type;

	std::vector<distance_type> distances;
	distances.reserve(provider.record_count());
	for (std::size_t position = 0; position < provider.record_count(); ++position) {
		if (position == source_position) {
			continue;
		}
		distances.push_back(group_detail::guarded_distance_at_position(
			provider, source_position, position, runtime, operation));
	}

	std::nth_element(distances.begin(), distances.begin() + static_cast<std::ptrdiff_t>(k - 1), distances.end());
	return distances[k - 1];
}

// Distance from source_id to its k-th nearest OTHER record (self excluded). This is the
// classic k-NN outlier score (Ramaswamy et al. 2000): a record far from its k-th neighbor
// sits in a sparse region of the finite metric space. Precondition: at least k other
// records exist.
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto kth_nearest_other_distance(const Provider &provider, RecordId source_id, std::size_t k,
								space::storage::runtime_guard runtime = {})
	-> typename Provider::distance_type
{
	return kth_nearest_other_position_distance(provider, provider.position_of(source_id), k, runtime,
											   "nearest-neighbor outlier scoring");
}

template <typename Provider>
auto kth_nearest_sampled_other_position_distance(const Provider &provider, std::size_t source_position,
												 const std::vector<std::size_t> &sample_positions,
												 std::size_t k, space::storage::runtime_guard runtime,
												 const char *operation) -> typename Provider::distance_type
{
	using distance_type = typename Provider::distance_type;

	std::vector<distance_type> distances;
	distances.reserve(sample_positions.size());
	for (const auto candidate_position : sample_positions) {
		if (candidate_position == source_position) {
			continue;
		}
		distances.push_back(group_detail::guarded_distance_at_position(
			provider, source_position, candidate_position, runtime, operation));
	}

	if (distances.size() < k) {
		throw std::invalid_argument(
			bounded_sample_too_small_message(provider.record_count(), k, sample_positions.size()));
	}

	std::nth_element(distances.begin(), distances.begin() + static_cast<std::ptrdiff_t>(k - 1), distances.end());
	return distances[k - 1];
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto kth_nearest_sampled_other_distance(const Provider &provider, RecordId source_id,
										const std::vector<RecordId> &sample_ids, std::size_t k,
										space::storage::runtime_guard runtime = {})
	-> typename Provider::distance_type
{
	std::vector<std::size_t> sample_positions;
	sample_positions.reserve(sample_ids.size());
	for (const auto candidate_id : sample_ids) {
		sample_positions.push_back(provider.position_of(candidate_id));
	}
	return kth_nearest_sampled_other_position_distance(provider, provider.position_of(source_id), sample_positions,
													   k, runtime, "sampled nearest-neighbor outlier scoring");
}

template <typename Provider>
auto outlier_result_from_groups(const Provider &provider,
								const ClusteringResult<typename Provider::distance_type> &groups,
								space::storage::runtime_guard runtime = {})
	-> OutlierResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	const auto references = ::mtrc::assigned_record_ids(provider, groups);
	std::vector<std::size_t> reference_positions;
	reference_positions.reserve(references.size());
	for (const auto id : references) {
		reference_positions.push_back(provider.position_of(id));
	}

	auto outliers = core::scored_outliers<distance_type>(groups.unassigned_records, [&](RecordId id) {
		runtime.throw_if_cancelled("dbscan outlier scoring");
		const auto source_position = provider.position_of(id);
		return reference_positions.empty()
			? guarded_nearest_other_record_distance_at_position_or(provider, source_position, distance_type{},
																   runtime, "dbscan outlier scoring")
			: guarded_nearest_distance_to_positions(provider, source_position, reference_positions,
													"outlier references must not be empty", runtime,
													"dbscan outlier scoring");
	});

	return core::make_outlier_result(std::move(outliers), groups.record_count, groups.cluster_count, groups.unassigned_count,
									 "dbscan_density_outlier", groups.representation);
}

template <typename Provider>
auto approximate_outlier_result_from_groups(const Provider &provider,
											const ClusteringResult<typename Provider::distance_type> &groups,
											space::storage::runtime_guard runtime = {})
	-> OutlierResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	const auto references = groups.medoids;
	std::vector<std::size_t> reference_positions;
	reference_positions.reserve(references.size());
	for (const auto id : references) {
		reference_positions.push_back(provider.position_of(id));
	}
	auto outliers = core::scored_outliers<distance_type>(groups.unassigned_records, [&](RecordId id) {
		runtime.throw_if_cancelled("sampled dbscan outlier scoring");
		return reference_positions.empty()
			? distance_type{}
			: guarded_nearest_distance_to_positions(
				  provider, provider.position_of(id), reference_positions,
				  "approximate outlier references must not be empty",
				  runtime, "sampled dbscan outlier scoring");
	});
	return core::make_outlier_result(std::move(outliers), groups.record_count, groups.cluster_count, groups.unassigned_count,
									 "sampled_dbscan_density_outlier", groups.representation, false);
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto sampled_nearest_neighbor_outliers(const Provider &provider, std::size_t k,
									   space::storage::policy runtime_policy,
									   space::storage::runtime_guard runtime = {})
	-> OutlierResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	validate_nearest_neighbor_outlier_request(provider.record_count(), k);

	const auto sample_count = group_detail::approximate_sample_count(provider.record_count(), 0, runtime_policy);
	if (sample_count <= k) {
		throw std::invalid_argument(bounded_sample_too_small_message(provider.record_count(), k, sample_count));
	}

	const auto positions = group_detail::regular_sample_positions(provider.record_count(), sample_count);
	if (positions.size() <= k) {
		throw std::invalid_argument(
			bounded_sample_too_small_message(provider.record_count(), k, positions.size()));
	}

	std::vector<core::Outlier<distance_type>> outliers;
	outliers.reserve(provider.record_count());
	for (std::size_t position = 0; position < provider.record_count(); ++position) {
		runtime.throw_if_cancelled("sampled nearest-neighbor outlier scoring");
		outliers.push_back(core::Outlier<distance_type>{
			provider.id(position),
			kth_nearest_sampled_other_position_distance(provider, position, positions, k, runtime,
														"sampled nearest-neighbor outlier scoring")});
	}
	core::sort_outliers(outliers);

	auto result = core::make_outlier_result(std::move(outliers), provider.record_count(), 0, 0,
											"sampled_nearest_neighbor_distance", "sampled_metric_space", false);
	mark_sampled_nearest_outlier_quality(result, positions.size(), k);
	return result;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto nearest_neighbor_outliers_from_provider(const Provider &provider, std::size_t k,
											 space::storage::runtime_guard runtime = {})
	-> OutlierResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	validate_nearest_neighbor_outlier_request(provider.record_count(), k);

	std::vector<core::Outlier<distance_type>> outliers;
	outliers.reserve(provider.record_count());
	for (std::size_t position = 0; position < provider.record_count(); ++position) {
		runtime.throw_if_cancelled("nearest-neighbor outlier scoring");
		outliers.push_back(core::Outlier<distance_type>{
			provider.id(position),
			kth_nearest_other_position_distance(provider, position, k, runtime,
												"nearest-neighbor outlier scoring")});
	}
	core::sort_outliers(outliers);

	return core::make_outlier_result(std::move(outliers), provider.record_count(), 0, 0,
									 "nearest_neighbor_distance", "pairwise_distances");
}

} // namespace outlier_detail

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_outliers(const Provider &provider, stats::structural_analysis::dbscan_options strategy)
	-> OutlierResult<typename Provider::distance_type>
{
	const auto groups = group_detail::guarded_dbscan(provider, strategy.radius, strategy.min_points, {});
	return outlier_detail::outlier_result_from_groups(provider, groups, {});
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, stats::structural_analysis::dbscan_options strategy) -> OutlierResult<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	if (group_detail::should_use_default_approximation(space.size())) {
		auto groups = group_detail::sampled_dbscan(provider, strategy.radius, strategy.min_points,
												   space::storage::approximate());
		auto result = outlier_detail::approximate_outlier_result_from_groups(provider, groups);
		result.representation = "sampled_metric_space";
		return result;
	}
	auto result = find_outliers(provider, strategy);
	result.representation = "metric_space";
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_outliers(const Container &records, const Metric &metric,
				   stats::structural_analysis::dbscan_options strategy)
	-> OutlierResult<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = find_outliers(space, strategy);
	result.representation = "records";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, stats::structural_analysis::dbscan_options strategy, space::storage::policy runtime_policy)
	-> OutlierResult<typename Space::distance_type>
{
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.is_approximate()) {
		auto runtime = space::storage::runtime_guard(runtime_policy);
		space::storage::LiveDistances<Space> provider(space);
		auto groups = group_detail::sampled_dbscan(provider, strategy.radius, strategy.min_points, runtime_policy,
												   runtime);
		auto result = outlier_detail::approximate_outlier_result_from_groups(provider, groups, runtime);
		result.representation = "sampled_metric_space";
		return result;
	}
	space::storage::require_exact_outliers(runtime_policy);
	auto runtime = space::storage::runtime_guard(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		return space::storage::with_materialized_distance_provider(
			space, runtime_policy, "outliers", [strategy, runtime](const auto &provider, const auto &plan) {
				auto groups = group_detail::guarded_dbscan(provider, strategy.radius, strategy.min_points, runtime);
				auto result = outlier_detail::outlier_result_from_groups(provider, groups, runtime);
				result.representation = space::storage::materialized_operator_representation(plan);
				return result;
			});
	}

	space::storage::LiveDistances<Space> provider(space);
	auto groups = group_detail::guarded_dbscan(provider, strategy.radius, strategy.min_points, runtime);
	auto result = outlier_detail::outlier_result_from_groups(provider, groups, runtime);
	result.representation = space::storage::outlier_representation(runtime_policy);
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_outliers(const Container &records, const Metric &metric,
				   stats::structural_analysis::dbscan_options strategy, space::storage::policy runtime_policy)
	-> OutlierResult<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	return find_outliers(space, strategy, runtime_policy);
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_outliers(const Provider &provider, double radius, std::size_t min_points)
	-> OutlierResult<typename Provider::distance_type>
{
	return find_outliers(provider, stats::structural_analysis::dbscan_options(radius, min_points));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, double radius, std::size_t min_points)
	-> OutlierResult<typename Space::distance_type>
{
	return find_outliers(space, stats::structural_analysis::dbscan_options(radius, min_points));
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_outliers(const Container &records, const Metric &metric, double radius, std::size_t min_points)
	-> OutlierResult<metric_result_t<Metric, Record>>
{
	return find_outliers(records, metric, stats::structural_analysis::dbscan_options(radius, min_points));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, double radius, std::size_t min_points, space::storage::policy runtime_policy)
	-> OutlierResult<typename Space::distance_type>
{
	return find_outliers(space, stats::structural_analysis::dbscan_options(radius, min_points), runtime_policy);
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_outliers(const Container &records, const Metric &metric, double radius, std::size_t min_points,
				   space::storage::policy runtime_policy) -> OutlierResult<metric_result_t<Metric, Record>>
{
	return find_outliers(records, metric, stats::structural_analysis::dbscan_options(radius, min_points),
						 runtime_policy);
}

// k-NN distance outlier score. Unlike DBSCAN density-unassigned records this scores EVERY record by how
// isolated it is (distance to its k-th nearest neighbor), so there is no radius/min_points
// to tune. The result is sorted by score descending, ties broken by RecordId ascending. The
// reported cluster_count/unassigned_count are 0 (this is a per-record score, not a partition).
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto nearest_neighbor_outliers(const Provider &provider, std::size_t k)
	-> OutlierResult<typename Provider::distance_type>
{
	return outlier_detail::nearest_neighbor_outliers_from_provider(provider, k);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto nearest_neighbor_outliers(const Space &space, std::size_t k) -> OutlierResult<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	if (group_detail::should_use_default_approximation(space.size())) {
		return outlier_detail::sampled_nearest_neighbor_outliers(provider, k, space::storage::approximate());
	}
	auto result = nearest_neighbor_outliers(provider, k);
	result.representation = "metric_space";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto nearest_neighbor_outliers(const Space &space, std::size_t k, space::storage::policy runtime_policy)
	-> OutlierResult<typename Space::distance_type>
{
	outlier_detail::validate_nearest_neighbor_outlier_request(space.size(), k);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	const auto plan = outlier_detail::require_outlier_budget_plan(space, runtime_policy);

	if (runtime_policy.is_approximate() || !plan.exact) {
		auto runtime = space::storage::runtime_guard(runtime_policy);
		space::storage::LiveDistances<Space> provider(space);
		auto result = outlier_detail::sampled_nearest_neighbor_outliers(provider, k, runtime_policy, runtime);
		result.representation = "sampled_metric_space";
		return result;
	}

	space::storage::require_exact_outliers(runtime_policy);
	auto runtime = space::storage::runtime_guard(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		return space::storage::with_materialized_distance_provider(
			space, runtime_policy, "outliers", [k, runtime](const auto &provider, const auto &materialized_plan) {
				auto result = outlier_detail::nearest_neighbor_outliers_from_provider(provider, k, runtime);
				result.representation = space::storage::materialized_operator_representation(materialized_plan);
				return result;
			});
	}

	space::storage::LiveDistances<Space> provider(space);
	auto result = outlier_detail::nearest_neighbor_outliers_from_provider(provider, k, runtime);
	result.representation = space::storage::outlier_representation(runtime_policy);
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto nearest_neighbor_outliers(const Container &records, const Metric &metric, std::size_t k)
	-> OutlierResult<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = nearest_neighbor_outliers(space, k);
	result.representation = "records";
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto nearest_neighbor_outliers(const Container &records, const Metric &metric, std::size_t k,
							   space::storage::policy runtime_policy)
	-> OutlierResult<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	return nearest_neighbor_outliers(space, k, runtime_policy);
}

} // namespace mtrc::stats::structural_analysis

namespace mtrc {
using stats::structural_analysis::find_outliers;
using stats::structural_analysis::nearest_neighbor_outliers;
} // namespace mtrc

#endif
