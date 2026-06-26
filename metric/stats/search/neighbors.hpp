// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_SEARCH_NEIGHBORS_HPP
#define _METRIC_STATS_SEARCH_NEIGHBORS_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/neighbor.hpp>
#include <metric/core/parameters.hpp>
#include <metric/record/id.hpp>
#include <metric/core/result.hpp>
#include <metric/stats/search/nearest.hpp>
#include <metric/space/chunked.hpp>
#include <metric/space/sample_plan.hpp>
#include <metric/space/storage/cover_tree_index.hpp>
#include <metric/space/storage/knn_graph_index.hpp>
#include <metric/space/storage/landmark_index.hpp>
#include <metric/space/storage/exact_scan_index.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/stats/search/options.hpp>

namespace mtrc::stats::search {

namespace detail {

constexpr std::size_t default_approximate_search_candidate_count = 4096;
constexpr std::size_t default_approximate_recall_reference_limit = 4096;

template <typename Distance, typename Radius>
auto range_from_neighbors(std::vector<Neighbor<Distance>> neighbors, Radius radius, std::size_t record_count,
						  const char *representation) -> NeighborSet<Distance>
{
	std::vector<Neighbor<Distance>> within;
	within.reserve(neighbors.size());
	for (const auto &neighbor : neighbors) {
		if (neighbor.distance <= radius) {
			within.push_back(neighbor);
		}
	}
	return core::range_neighbor_set(std::move(within), record_count, representation);
}

template <typename Distance>
auto set_search_representation(NeighborSet<Distance> &result, std::string representation) -> void
{
	result.representation = std::move(representation);
	core::set_default_search_provenance(result.provenance, result.representation);
}

template <typename Distance>
auto exclude_neighbor_id(std::vector<Neighbor<Distance>> neighbors, RecordId excluded) -> std::vector<Neighbor<Distance>>
{
	std::vector<Neighbor<Distance>> filtered;
	filtered.reserve(neighbors.size());
	for (const auto &neighbor : neighbors) {
		if (neighbor.id != excluded) {
			filtered.push_back(neighbor);
		}
	}
	return filtered;
}

inline auto approximate_candidate_count(std::size_t record_count, std::size_t requested_count,
										space::storage::policy runtime_policy,
										bool excludes_query_record = false) -> std::size_t
{
	const auto available = excludes_query_record && record_count > 0 ? record_count - 1 : record_count;
	if (runtime_policy.graph_neighbors() != 0) {
		return std::min(runtime_policy.graph_neighbors(), available);
	}
	if (available == 0) {
		return 0;
	}
	auto candidate_count = std::min(default_approximate_search_candidate_count, available);
	if (requested_count > candidate_count) {
		candidate_count = std::min(requested_count, available);
	}
	return candidate_count;
}

inline auto landmark_candidate_count(std::size_t record_count, space::storage::policy runtime_policy,
									 bool excludes_query_record = false,
									 std::size_t requested_count = 0,
									 std::size_t query_count = 0) -> std::size_t
{
	return space::storage::landmark_index_candidate_limit(
		record_count, runtime_policy, excludes_query_record, requested_count, query_count);
}

inline auto landmark_count(std::size_t record_count, space::storage::policy runtime_policy,
						   std::size_t query_count = 0) -> std::size_t
{
	return space::storage::landmark_index_landmark_count(record_count, runtime_policy, query_count);
}

inline auto require_approximate_search_representation(space::storage::policy runtime_policy) -> void
{
	const auto representation = runtime_policy.representation_mode();
	if (representation != space::storage::representation::automatic &&
		representation != space::storage::representation::implicit &&
		representation != space::storage::representation::knn_graph &&
		representation != space::storage::representation::landmark_index) {
		throw InvalidRuntimePolicyError(
			"approximate search runtime policy currently supports automatic, implicit, kNN graph, or landmark candidate representation");
	}
}

inline auto uses_landmark_search(space::storage::policy runtime_policy) -> bool
{
	return runtime_policy.representation_mode() == space::storage::representation::landmark_index;
}

inline auto uses_chunked_search_plan(const space::storage::execution_plan &plan) -> bool
{
	return plan.allowed && plan.downgraded && !plan.exact && plan.representation == "chunked_space_view";
}

template <typename Quality>
auto mark_recall_confidence_interval(Quality &quality, std::size_t matched_count, std::size_t reference_count)
	-> void
{
	if (reference_count == 0) {
		quality.standard_error = 0.0;
		quality.confidence_radius_95 = 0.0;
		return;
	}
	const auto recall = static_cast<double>(matched_count) / static_cast<double>(reference_count);
	const auto variance = recall * (1.0 - recall) / static_cast<double>(reference_count);
	quality.standard_error = std::sqrt(variance < 0.0 ? 0.0 : variance);
	quality.confidence_radius_95 = std::min(1.0, 1.96 * quality.standard_error);
}

template <typename Distance>
auto mark_approximate_sampled_search(NeighborSet<Distance> &result) -> void
{
	result.exact = false;
	result.representation = "sampled_metric_space";
	core::set_search_route(result.provenance, core::search_route_kind::approximate_sampled_candidates);
	core::set_search_domain(result.provenance, core::result_domain_kind::sampled_metric_space);
}

template <typename Distance>
auto mark_sampled_search_quality(NeighborSet<Distance> &result, std::size_t candidate_count,
								 std::size_t candidate_universe, std::size_t requested_count) -> void
{
	result.approximation_quality.diagnostic = "search_approximation";
	result.approximation_quality.candidate_policy = "regular_sample";
	result.approximation_quality.candidate_count = candidate_count;
	result.approximation_quality.candidate_universe = candidate_universe;
	result.approximation_quality.distance_evaluations = candidate_count;
	result.approximation_quality.requested_count = requested_count;
	result.approximation_quality.sample_count = candidate_count;
	result.approximation_quality.sample_universe = candidate_universe;
	result.approximation_quality.recall_reference_count = 0;
	result.approximation_quality.recall_matched_count = 0;
	result.approximation_quality.recall_distance_evaluations = 0;
	result.approximation_quality.recall_sample_query_count = 0;
	result.approximation_quality.candidate_fraction =
		candidate_universe == 0 ? 1.0 : static_cast<double>(candidate_count) / static_cast<double>(candidate_universe);
	result.approximation_quality.sample_fraction = result.approximation_quality.candidate_fraction;
	result.approximation_quality.recall_measured = false;
	result.approximation_quality.recall = 0.0;
	result.approximation_quality.reason =
			"approximate runtime policy selected bounded sampled candidates; recall was not measured";
}

template <typename Distance, typename ChunkedResult>
auto mark_chunked_search_quality(NeighborSet<Distance> &result, const ChunkedResult &chunked,
								 std::size_t requested_count) -> void
{
	result.exact = chunked.exact;
	result.representation = "chunked_space_view";
	core::set_search_route(result.provenance, core::search_route_kind::custom);
	core::set_search_domain(result.provenance, core::result_domain_kind::custom);
	result.provenance.route = "chunked_search_refinement";
	if (result.exact) {
		return;
	}

	const auto chunk_pair_universe = space::storage::detail::distance_table_dense_slot_count(chunked.chunk_count);
	result.approximation_quality.diagnostic = "chunked_search_refinement";
	result.approximation_quality.candidate_policy = "local_chunks_plus_representative_refinement";
	result.approximation_quality.candidate_count = chunked.refinement_candidate_pair_count;
	result.approximation_quality.candidate_universe = chunked.dense_pair_distance_evaluations;
	result.approximation_quality.distance_evaluations = chunked.bounded_pair_distance_evaluations;
	result.approximation_quality.requested_count = requested_count;
	result.approximation_quality.sample_count = chunked.refinement_chunk_pair_count;
	result.approximation_quality.sample_universe = chunk_pair_universe;
	result.approximation_quality.recall_reference_count = 0;
	result.approximation_quality.recall_matched_count = 0;
	result.approximation_quality.recall_distance_evaluations = 0;
	result.approximation_quality.recall_sample_query_count = 0;
	result.approximation_quality.candidate_fraction =
		chunked.dense_pair_distance_evaluations == 0
			? 1.0
			: std::min(1.0, static_cast<double>(chunked.bounded_pair_distance_evaluations) /
								 static_cast<double>(chunked.dense_pair_distance_evaluations));
	result.approximation_quality.sample_fraction =
		chunk_pair_universe == 0 ? 1.0
								 : static_cast<double>(chunked.refinement_chunk_pair_count) /
									   static_cast<double>(chunk_pair_universe);
	result.approximation_quality.recall_measured = false;
	result.approximation_quality.recall = 0.0;
	result.approximation_quality.reason =
		"runtime policy selected bounded chunked search refinement; recall was not measured";
}

template <typename Distance>
auto mark_chunked_query_search_quality(NeighborSet<Distance> &result, std::size_t candidate_count,
									   std::size_t candidate_universe, std::size_t distance_evaluations,
									   std::size_t requested_count, std::size_t sample_count,
									   std::size_t sample_universe, bool exact) -> void
{
	result.exact = exact;
	result.representation = "chunked_space_view";
	core::set_search_route(result.provenance, core::search_route_kind::custom);
	core::set_search_domain(result.provenance, core::result_domain_kind::custom);
	result.provenance.route = "chunked_search_refinement";
	if (result.exact) {
		return;
	}

	result.approximation_quality.diagnostic = "chunked_search_refinement";
	result.approximation_quality.candidate_policy = "query_representative_chunk_refinement";
	result.approximation_quality.candidate_count = candidate_count;
	result.approximation_quality.candidate_universe = candidate_universe;
	result.approximation_quality.distance_evaluations = distance_evaluations;
	result.approximation_quality.requested_count = requested_count;
	result.approximation_quality.sample_count = sample_count;
	result.approximation_quality.sample_universe = sample_universe;
	result.approximation_quality.recall_reference_count = 0;
	result.approximation_quality.recall_matched_count = 0;
	result.approximation_quality.recall_distance_evaluations = 0;
	result.approximation_quality.recall_sample_query_count = 0;
	result.approximation_quality.candidate_fraction =
		candidate_universe == 0
			? 1.0
			: std::min(1.0, static_cast<double>(candidate_count) / static_cast<double>(candidate_universe));
	result.approximation_quality.sample_fraction =
		sample_universe == 0
			? 1.0
			: std::min(1.0, static_cast<double>(sample_count) / static_cast<double>(sample_universe));
	result.approximation_quality.recall_measured = false;
	result.approximation_quality.recall = 0.0;
	result.approximation_quality.reason =
		"runtime policy selected bounded query chunk refinement; recall was not measured";
}

template <typename Distance> struct chunked_query_chunk_candidate {
	std::size_t chunk{};
	RecordId representative_id{};
	Distance distance{};
};

template <typename Distance>
auto chunked_query_chunk_candidate_less(const chunked_query_chunk_candidate<Distance> &lhs,
										const chunked_query_chunk_candidate<Distance> &rhs) -> bool
{
	if (core::NeighborLess<Distance>{}({lhs.representative_id, lhs.distance},
									   {rhs.representative_id, rhs.distance})) {
		return true;
	}
	if (core::NeighborLess<Distance>{}({rhs.representative_id, rhs.distance},
									   {lhs.representative_id, lhs.distance})) {
		return false;
	}
	return lhs.chunk < rhs.chunk;
}

template <typename Space>
auto select_chunked_query_chunks(const Space &space, const ::mtrc::space::ChunkedSpaceView<Space> &chunks,
								 const typename Space::record_type &query,
								 std::size_t requested_chunk_count,
								 ::mtrc::space::storage::runtime_guard runtime)
	-> std::vector<chunked_query_chunk_candidate<typename Space::distance_type>>
{
	using distance_type = typename Space::distance_type;
	const auto selected_count = chunks.chunk_count() == 0
									? std::size_t{0}
									: std::min(chunks.chunk_count(), std::max(std::size_t{1}, requested_chunk_count));
	std::vector<chunked_query_chunk_candidate<distance_type>> candidates;
	candidates.reserve(chunks.chunk_count());
	for (std::size_t chunk_index = 0; chunk_index < chunks.chunk_count(); ++chunk_index) {
		runtime.throw_if_cancelled("chunked query representative search");
		const auto representative_id = chunks.representative_id(chunk_index);
		candidates.push_back(
			{chunk_index, representative_id,
			 static_cast<distance_type>(space.metric()(query, chunks.representative_record(chunk_index)))});
	}
	std::sort(candidates.begin(), candidates.end(), chunked_query_chunk_candidate_less<distance_type>);
	if (selected_count < candidates.size()) {
		candidates.resize(selected_count);
	}
	return candidates;
}

template <typename Space>
auto chunked_plan_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
							const space::storage::execution_plan &plan,
							space::storage::policy runtime_policy,
							::mtrc::space::storage::runtime_guard runtime)
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	runtime.throw_if_cancelled("chunked query view construction");
	const auto chunks = ::mtrc::space::chunked_view(space, plan.chunk_size == 0 ? std::size_t{1} : plan.chunk_size);
	if (count == 0) {
		auto result = core::make_neighbor_set(
			std::vector<Neighbor<distance_type>>{}, chunks.record_count(), count, "knn", "chunked_space_view", false);
		mark_chunked_query_search_quality(
			result, 0, chunks.record_count(), 0, count, 0, chunks.chunk_count(), false);
		return result;
	}

	const auto candidate_chunks =
		space::storage::chunked_search_candidate_chunks_per_chunk(chunks.chunk_count(), runtime_policy);
	const auto selected_chunks = select_chunked_query_chunks(space, chunks, query, candidate_chunks, runtime);
	std::vector<Neighbor<distance_type>> candidates;
	std::size_t candidate_record_count = 0;
	std::size_t distance_evaluations = chunks.chunk_count();
	for (const auto &selected : selected_chunks) {
		const auto chunk = chunks.chunk(selected.chunk);
		for (std::size_t local_position = 0; local_position < chunk.local_record_count(); ++local_position) {
			runtime.throw_if_cancelled("chunked query neighbor refinement");
			const auto id = chunk.id(local_position);
			auto distance = selected.distance;
			if (id != selected.representative_id) {
				distance = static_cast<distance_type>(space.metric()(query, chunk.record(local_position)));
				++distance_evaluations;
			}
			candidates.push_back({id, distance});
			++candidate_record_count;
		}
	}

	const auto exact = selected_chunks.size() == chunks.chunk_count();
	auto result = core::nearest_neighbor_set(std::move(candidates), count, chunks.record_count(), "chunked_space_view");
	mark_chunked_query_search_quality(
		result, candidate_record_count, chunks.record_count(), distance_evaluations, count,
		selected_chunks.size(), chunks.chunk_count(), exact);
	return result;
}

template <typename Space>
auto chunked_plan_neighbors(const Space &space, RecordId query_id, std::size_t count,
							const space::storage::execution_plan &plan,
							space::storage::policy runtime_policy,
							::mtrc::space::storage::runtime_guard runtime)
	-> NeighborSet<typename Space::distance_type>
{
	runtime.throw_if_cancelled("chunked search view construction");
	const auto chunks = ::mtrc::space::chunked_view(space, plan.chunk_size == 0 ? std::size_t{1} : plan.chunk_size);
	const auto candidate_chunks =
		space::storage::chunked_search_candidate_chunks_per_chunk(chunks.chunk_count(), runtime_policy);
	runtime.throw_if_cancelled("chunked neighbor search");
	const auto chunked = chunks.refined_neighbors(count, candidate_chunks);
	auto neighbors = chunked.neighbors(query_id);
	auto result = core::make_neighbor_set(
		std::move(neighbors), chunked.record_count, count, "knn", "chunked_space_view", chunked.exact);
	mark_chunked_search_quality(result, chunked, count);
	return result;
}

template <typename Space, typename Radius>
auto chunked_plan_range(const Space &space, const typename Space::record_type &query, Radius radius,
						const space::storage::execution_plan &plan,
						space::storage::policy runtime_policy,
						::mtrc::space::storage::runtime_guard runtime)
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;

	runtime.throw_if_cancelled("chunked query view construction");
	const auto chunks = ::mtrc::space::chunked_view(space, plan.chunk_size == 0 ? std::size_t{1} : plan.chunk_size);
	const auto candidate_chunks =
		space::storage::chunked_search_candidate_chunks_per_chunk(chunks.chunk_count(), runtime_policy);
	const auto selected_chunks = select_chunked_query_chunks(space, chunks, query, candidate_chunks, runtime);
	const auto threshold = static_cast<comparison_type>(radius);
	std::vector<Neighbor<distance_type>> candidates;
	std::size_t candidate_record_count = 0;
	std::size_t distance_evaluations = chunks.chunk_count();
	for (const auto &selected : selected_chunks) {
		const auto chunk = chunks.chunk(selected.chunk);
		for (std::size_t local_position = 0; local_position < chunk.local_record_count(); ++local_position) {
			runtime.throw_if_cancelled("chunked query range refinement");
			const auto id = chunk.id(local_position);
			auto distance = selected.distance;
			if (id != selected.representative_id) {
				distance = static_cast<distance_type>(space.metric()(query, chunk.record(local_position)));
				++distance_evaluations;
			}
			if (static_cast<comparison_type>(distance) <= threshold) {
				candidates.push_back({id, distance});
			}
			++candidate_record_count;
		}
	}

	const auto exact = selected_chunks.size() == chunks.chunk_count();
	auto result = core::range_neighbor_set(std::move(candidates), chunks.record_count(), "chunked_space_view");
	mark_chunked_query_search_quality(
		result, candidate_record_count, chunks.record_count(), distance_evaluations, 0,
		selected_chunks.size(), chunks.chunk_count(), exact);
	return result;
}

template <typename Space, typename Radius>
auto chunked_plan_range(const Space &space, RecordId query_id, Radius radius,
						const space::storage::execution_plan &plan,
						space::storage::policy runtime_policy,
						::mtrc::space::storage::runtime_guard runtime)
	-> NeighborSet<typename Space::distance_type>
{
	runtime.throw_if_cancelled("chunked search view construction");
	const auto chunks = ::mtrc::space::chunked_view(space, plan.chunk_size == 0 ? std::size_t{1} : plan.chunk_size);
	const auto candidate_chunks =
		space::storage::chunked_search_candidate_chunks_per_chunk(chunks.chunk_count(), runtime_policy);
	runtime.throw_if_cancelled("chunked range search");
	const auto chunked = chunks.refined_range(radius, candidate_chunks);
	auto neighbors = chunked.neighbors(query_id);
	core::sort_neighbors(neighbors);
	const auto requested_count = neighbors.size();
	auto result = core::make_neighbor_set(
		std::move(neighbors), chunked.record_count, requested_count, "range", "chunked_space_view", chunked.exact);
	mark_chunked_search_quality(result, chunked, 0);
	return result;
}

template <typename Distance>
auto mark_approximate_landmark_search(NeighborSet<Distance> &result, std::size_t candidate_count,
									  std::size_t candidate_universe, std::size_t requested_count,
									  std::size_t distance_evaluations) -> void
{
	result.exact = false;
	result.representation = "landmark_index";
	core::set_search_route(result.provenance, core::search_route_kind::approximate_sampled_candidates);
	core::set_search_domain(result.provenance, core::result_domain_kind::neighbor_index);
	result.approximation_quality.diagnostic = "landmark_search_approximation";
	result.approximation_quality.candidate_policy = "landmark_lower_bound_refinement";
	result.approximation_quality.candidate_count = candidate_count;
	result.approximation_quality.candidate_universe = candidate_universe;
	result.approximation_quality.distance_evaluations = distance_evaluations;
	result.approximation_quality.requested_count = requested_count;
	result.approximation_quality.sample_count = candidate_count;
	result.approximation_quality.sample_universe = candidate_universe;
	result.approximation_quality.recall_reference_count = 0;
	result.approximation_quality.recall_matched_count = 0;
	result.approximation_quality.recall_distance_evaluations = 0;
	result.approximation_quality.recall_sample_query_count = 0;
	result.approximation_quality.candidate_fraction =
		candidate_universe == 0 ? 1.0 : static_cast<double>(candidate_count) / static_cast<double>(candidate_universe);
	result.approximation_quality.sample_fraction = result.approximation_quality.candidate_fraction;
	result.approximation_quality.recall_measured = false;
	result.approximation_quality.recall = 0.0;
	result.approximation_quality.reason =
		"approximate runtime policy selected landmark lower-bound candidates; recall calibration has not run yet";
}

inline auto estimate_landmark_calibration_distance_evaluations(std::size_t sample_query_count,
															   std::size_t landmark_count,
															   std::size_t candidate_limit,
															   std::size_t reference_candidate_count)
	-> std::size_t
{
	const auto max_value = static_cast<std::size_t>(-1);
	if (landmark_count > max_value - candidate_limit) {
		return max_value;
	}
	const auto partial = landmark_count + candidate_limit;
	if (partial > max_value - reference_candidate_count) {
		return max_value;
	}
	const auto per_query = partial + reference_candidate_count;
	if (sample_query_count > 0 && per_query > max_value / sample_query_count) {
		return max_value;
	}
	return sample_query_count * per_query;
}

inline auto landmark_recall_unmeasured_reason(std::size_t requested_count, std::size_t sample_query_count,
											  std::size_t reference_candidate_count,
											  std::size_t estimated_calibration_evaluations,
											  std::size_t already_planned_evaluations,
											  space::storage::policy runtime_policy) -> std::string
{
	if (requested_count == 0) {
		return "approximate runtime policy selected landmark lower-bound candidates; recall calibration was not run "
			   "because the requested neighbor count is zero";
	}
	if (sample_query_count == 0) {
		return "approximate runtime policy selected landmark lower-bound candidates; recall calibration was not run "
			   "because there are no source queries in the holdout window";
	}
	if (reference_candidate_count == 0) {
		return "approximate runtime policy selected landmark lower-bound candidates; recall calibration was not run "
			   "because the bounded reference candidate window is empty";
	}
	const auto budget = runtime_policy.max_distance_evaluations();
	if (budget != 0 &&
		(estimated_calibration_evaluations > budget ||
		 already_planned_evaluations > budget - estimated_calibration_evaluations)) {
		return "approximate runtime policy selected landmark lower-bound candidates; recall calibration was not run "
			   "because the bounded holdout window would exceed max_distance_evaluations";
	}
	return "approximate runtime policy selected landmark lower-bound candidates; recall calibration was not run";
}

template <typename Distance>
auto mark_landmark_search_recall_unmeasured(NeighborSet<Distance> &result, std::string reason) -> void
{
	result.approximation_quality.recall_measured = false;
	result.approximation_quality.recall = 0.0;
	result.approximation_quality.recall_reference_count = 0;
	result.approximation_quality.recall_matched_count = 0;
	result.approximation_quality.recall_distance_evaluations = 0;
	result.approximation_quality.recall_sample_query_count = 0;
	result.approximation_quality.standard_error = 0.0;
	result.approximation_quality.confidence_radius_95 = 0.0;
	result.approximation_quality.reason = std::move(reason);
}

template <typename Distance>
auto mark_landmark_search_recall(NeighborSet<Distance> &result,
								 const space::storage::landmark_recall_calibration &calibration) -> void
{
	result.approximation_quality.recall_reference_count = calibration.reference_count;
	result.approximation_quality.recall_matched_count = calibration.matched_count;
	result.approximation_quality.recall_distance_evaluations = calibration.reference_candidate_count;
	result.approximation_quality.recall_sample_query_count = calibration.sample_query_count;
	result.approximation_quality.distance_evaluations += calibration.distance_evaluations;
	result.approximation_quality.recall_measured = calibration.measured;
	result.approximation_quality.recall = calibration.recall;
	result.approximation_quality.standard_error = calibration.standard_error;
	result.approximation_quality.confidence_radius_95 = calibration.confidence_radius_95;
	result.approximation_quality.reason =
		"approximate runtime policy selected landmark lower-bound candidates; recall calibrated against a bounded "
		"holdout/reference window with query-level standard error";
}

template <typename Space, typename Distance>
auto calibrate_landmark_knn_quality(NeighborSet<Distance> &result,
									const space::storage::LandmarkIndex<Space> &index,
									std::size_t requested_count,
									std::size_t candidate_limit,
									std::size_t candidate_universe,
									space::storage::policy runtime_policy,
									::mtrc::space::storage::runtime_guard runtime) -> void
{
	const auto sample_query_count =
		space::storage::landmark_calibration_sample_query_count(index.record_count(), runtime_policy);
	const auto reference_candidate_count =
		space::storage::landmark_calibration_reference_candidate_limit(candidate_universe, candidate_limit);
	const auto estimated_calibration_evaluations = estimate_landmark_calibration_distance_evaluations(
		sample_query_count, index.landmark_count(), candidate_limit, reference_candidate_count);
	const auto budget = runtime_policy.max_distance_evaluations();
	if (requested_count == 0 || sample_query_count == 0 || reference_candidate_count == 0 ||
		(budget != 0 &&
		 (estimated_calibration_evaluations > budget ||
		  result.approximation_quality.distance_evaluations > budget - estimated_calibration_evaluations))) {
		mark_landmark_search_recall_unmeasured(
			result, landmark_recall_unmeasured_reason(
						requested_count, sample_query_count, reference_candidate_count,
						estimated_calibration_evaluations, result.approximation_quality.distance_evaluations,
						runtime_policy));
		return;
	}

	const auto calibration = index.calibrate_knn_recall(
		requested_count, candidate_limit, sample_query_count, reference_candidate_count, runtime);
	if (!calibration.measured) {
		mark_landmark_search_recall_unmeasured(
			result, landmark_recall_unmeasured_reason(
						requested_count, sample_query_count, reference_candidate_count,
						estimated_calibration_evaluations, result.approximation_quality.distance_evaluations,
						runtime_policy));
		return;
	}
	mark_landmark_search_recall(result, calibration);
}

template <typename Space, typename Distance, typename Radius>
auto calibrate_landmark_range_quality(NeighborSet<Distance> &result,
									  const space::storage::LandmarkIndex<Space> &index,
									  Radius radius,
									  std::size_t candidate_limit,
									  std::size_t candidate_universe,
									  space::storage::policy runtime_policy,
									  ::mtrc::space::storage::runtime_guard runtime) -> void
{
	const auto sample_query_count =
		space::storage::landmark_calibration_sample_query_count(index.record_count(), runtime_policy);
	const auto reference_candidate_count =
		space::storage::landmark_calibration_reference_candidate_limit(candidate_universe, candidate_limit);
	const auto estimated_calibration_evaluations = estimate_landmark_calibration_distance_evaluations(
		sample_query_count, index.landmark_count(), candidate_limit, reference_candidate_count);
	const auto budget = runtime_policy.max_distance_evaluations();
	if (sample_query_count == 0 || reference_candidate_count == 0 ||
		(budget != 0 &&
		 (estimated_calibration_evaluations > budget ||
		  result.approximation_quality.distance_evaluations > budget - estimated_calibration_evaluations))) {
		mark_landmark_search_recall_unmeasured(
			result, landmark_recall_unmeasured_reason(
						1, sample_query_count, reference_candidate_count,
						estimated_calibration_evaluations, result.approximation_quality.distance_evaluations,
						runtime_policy));
		return;
	}

	const auto calibration = index.calibrate_range_recall(
		radius, candidate_limit, sample_query_count, reference_candidate_count, runtime);
	if (!calibration.measured) {
		mark_landmark_search_recall_unmeasured(
			result, landmark_recall_unmeasured_reason(
						1, sample_query_count, reference_candidate_count,
						estimated_calibration_evaluations, result.approximation_quality.distance_evaluations,
						runtime_policy));
		return;
	}
	mark_landmark_search_recall(result, calibration);
}

inline auto can_measure_sampled_recall(std::size_t candidate_count, std::size_t candidate_universe,
									   space::storage::policy runtime_policy) -> bool
{
	if (candidate_universe > default_approximate_recall_reference_limit) {
		return false;
	}
	const auto budget = runtime_policy.max_distance_evaluations();
	return budget == 0 || candidate_count + candidate_universe <= budget;
}

inline auto sampled_recall_unmeasured_reason(std::size_t candidate_count, std::size_t candidate_universe,
											 space::storage::policy runtime_policy) -> std::string
{
	if (candidate_universe > default_approximate_recall_reference_limit) {
		return "approximate runtime policy selected bounded sampled candidates; recall was not measured because the "
			   "candidate universe exceeds the recall calibration guard";
	}
	const auto budget = runtime_policy.max_distance_evaluations();
	if (budget != 0 && candidate_count + candidate_universe > budget) {
		return "approximate runtime policy selected bounded sampled candidates; recall was not measured because the "
			   "distance-evaluation budget is reserved for the candidate sample";
	}
	return "approximate runtime policy selected bounded sampled candidates; recall was not measured";
}

template <typename Distance>
auto mark_sampled_search_recall_unmeasured(NeighborSet<Distance> &result, std::size_t candidate_count,
										   std::size_t candidate_universe,
										   space::storage::policy runtime_policy) -> void
{
	result.approximation_quality.recall_measured = false;
	result.approximation_quality.recall = 0.0;
	result.approximation_quality.standard_error = 0.0;
	result.approximation_quality.confidence_radius_95 = 0.0;
	result.approximation_quality.reason =
		sampled_recall_unmeasured_reason(candidate_count, candidate_universe, runtime_policy);
}

template <typename Distance>
auto mark_sampled_search_recall(NeighborSet<Distance> &result, const std::vector<Neighbor<Distance>> &reference,
								std::size_t reference_distance_evaluations,
								double empty_reference_recall = 1.0) -> void
{
	const auto matched = core::neighbor_id_overlap_count(reference, result.neighbors);
	const auto possible = reference.size();
	result.approximation_quality.recall_reference_count = possible;
	result.approximation_quality.recall_matched_count = matched;
	result.approximation_quality.recall_distance_evaluations = reference_distance_evaluations;
	result.approximation_quality.recall_sample_query_count = 1;
	result.approximation_quality.distance_evaluations += reference_distance_evaluations;
	result.approximation_quality.recall_measured = true;
	result.approximation_quality.recall =
		possible == 0 ? empty_reference_recall : static_cast<double>(matched) / static_cast<double>(possible);
	mark_recall_confidence_interval(result.approximation_quality, matched, possible);
	result.approximation_quality.reason =
		"approximate runtime policy selected bounded sampled candidates; recall measured against a bounded exact "
		"reference";
}

inline auto search_budget_error_message(const space::storage::execution_plan &plan) -> std::string
{
	auto message = plan.reason.empty() ? std::string("search runtime policy refused by resource budget") : plan.reason;
	if (!plan.fallback_hint.empty()) {
		message += "; fallback: " + plan.fallback_hint;
	}
	return message;
}

template <typename Space>
auto require_search_budget_plan(const Space &space, const char *intent, space::storage::policy runtime_policy)
	-> space::storage::execution_plan
{
	const auto plan = space::storage::estimate_cost(space, intent, std::size_t{1}, runtime_policy);
	if (plan.refused) {
		throw RepresentationError(search_budget_error_message(plan));
	}
	if (plan.downgraded && !plan.exact) {
		return plan;
	}
	if (plan.downgraded && !space::storage::uses_blocked_exact_fallback(plan)) {
		throw RepresentationError(search_budget_error_message(plan) +
								  "; executor integration for this downgraded route is not implemented");
	}
	return plan;
}

inline auto approximate_search_fallback_policy(space::storage::policy runtime_policy) -> space::storage::policy
{
	auto fallback = space::storage::approximate();
	fallback = space::storage::with_distance_table_budget(
		fallback, runtime_policy.max_dense_records(), runtime_policy.max_memory_bytes());
	fallback = space::storage::with_distance_evaluation_budget(fallback, runtime_policy.max_distance_evaluations());
	fallback = space::storage::with_runtime_budget(fallback, runtime_policy.max_runtime_ms());
	if (uses_landmark_search(runtime_policy)) {
		fallback = space::storage::using_landmark_index(runtime_policy.graph_neighbors(), fallback);
	} else if (runtime_policy.graph_neighbors() != 0) {
		fallback = space::storage::using_knn_graph(runtime_policy.graph_neighbors(), fallback);
	}
	return fallback;
}

template <typename Provider, typename = void> struct HasGuardedDistanceAtPosition : std::false_type {};

template <typename Provider>
struct HasGuardedDistanceAtPosition<
	Provider, std::void_t<decltype(std::declval<const Provider &>().distance_at_position(
				  std::declval<std::size_t>(), std::declval<std::size_t>(),
				  std::declval<::mtrc::space::storage::runtime_guard>()))>> : std::true_type {};

template <typename Provider>
auto guarded_provider_distance_from_query_position(const Provider &provider, RecordId query_id,
												   std::size_t query_position, RecordId candidate_id,
												   std::size_t candidate_position,
												   ::mtrc::space::storage::runtime_guard runtime)
	-> typename Provider::distance_type
{
	runtime.throw_if_cancelled("pairwise search distance evaluation");
	if constexpr (HasGuardedDistanceAtPosition<Provider>::value) {
		(void)query_id;
		(void)candidate_id;
		return provider.distance_at_position(query_position, candidate_position, runtime);
	} else {
		return engine_detail::provider_distance_from_query_position(
			provider, query_id, query_position, candidate_id, candidate_position);
	}
}

template <typename Distance, typename IdAt, typename DistanceAt, typename Include>
auto guarded_neighbor_candidates_if(std::size_t record_count, IdAt id_at, DistanceAt distance_at,
									Include include, ::mtrc::space::storage::runtime_guard runtime,
									const char *operation) -> std::vector<Neighbor<Distance>>
{
	std::vector<Neighbor<Distance>> candidates;
	candidates.reserve(record_count);
	for (std::size_t position = 0; position < record_count; ++position) {
		runtime.throw_if_cancelled(operation);
		const auto id = id_at(position);
		if (include(id, position)) {
			candidates.push_back(Neighbor<Distance>{id, static_cast<Distance>(distance_at(id, position))});
		}
	}
	return candidates;
}

template <typename Distance, typename IdAt, typename DistanceAt>
auto guarded_neighbor_candidates(std::size_t record_count, IdAt id_at, DistanceAt distance_at,
								 ::mtrc::space::storage::runtime_guard runtime, const char *operation)
	-> std::vector<Neighbor<Distance>>
{
	return guarded_neighbor_candidates_if<Distance>(
		record_count, id_at, distance_at, [](RecordId, std::size_t) { return true; }, runtime, operation);
}

template <typename Distance, typename Radius, typename IdAt, typename DistanceAt, typename Include>
auto guarded_neighbor_candidates_within_if(std::size_t record_count, IdAt id_at, DistanceAt distance_at,
										   Radius radius, Include include,
										   ::mtrc::space::storage::runtime_guard runtime, const char *operation)
	-> std::vector<Neighbor<Distance>>
{
	using comparison_type = typename std::common_type<Distance, Radius>::type;

	const auto threshold = static_cast<comparison_type>(radius);
	std::vector<Neighbor<Distance>> candidates;
	candidates.reserve(record_count);
	for (std::size_t position = 0; position < record_count; ++position) {
		runtime.throw_if_cancelled(operation);
		const auto id = id_at(position);
		if (!include(id, position)) {
			continue;
		}
		const auto distance = static_cast<Distance>(distance_at(id, position));
		if (static_cast<comparison_type>(distance) <= threshold) {
			candidates.push_back(Neighbor<Distance>{id, distance});
		}
	}
	return candidates;
}

template <typename Distance, typename Radius, typename IdAt, typename DistanceAt>
auto guarded_neighbor_candidates_within(std::size_t record_count, IdAt id_at, DistanceAt distance_at,
										Radius radius, ::mtrc::space::storage::runtime_guard runtime,
										const char *operation) -> std::vector<Neighbor<Distance>>
{
	return guarded_neighbor_candidates_within_if<Distance>(
		record_count, id_at, distance_at, radius, [](RecordId, std::size_t) { return true; }, runtime, operation);
}

template <typename Space>
auto approximate_sampled_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
								   space::storage::policy runtime_policy,
								   ::mtrc::space::storage::runtime_guard runtime = {})
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;
	require_approximate_search_representation(runtime_policy);

	if (count == 0) {
		auto result = core::make_neighbor_set(std::vector<Neighbor<distance_type>>{}, space.size(), count, "knn",
											  "sampled_metric_space", false);
		mark_approximate_sampled_search(result);
		mark_sampled_search_quality(result, 0, space.size(), count);
		return result;
	}

	const auto candidate_count = approximate_candidate_count(space.size(), count, runtime_policy);
	const auto candidate_plan = ::mtrc::space::regular_sample_positions(space.size(), candidate_count);
	std::vector<Neighbor<distance_type>> candidates;
	candidates.reserve(candidate_plan.size());
	for (const auto position : candidate_plan.positions) {
		runtime.throw_if_cancelled("approximate sampled neighbor search");
		const auto id = space.id(position);
		candidates.push_back(
			Neighbor<distance_type>{id, static_cast<distance_type>(space.metric()(query, space.record(id)))});
	}

	auto result = core::nearest_neighbor_set(std::move(candidates), count, space.size(), "sampled_metric_space");
	mark_approximate_sampled_search(result);
	mark_sampled_search_quality(result, candidate_plan.size(), candidate_plan.candidate_universe, count);
	if (count > 0) {
		if (can_measure_sampled_recall(candidate_plan.size(), candidate_plan.candidate_universe, runtime_policy)) {
			auto reference = guarded_neighbor_candidates<distance_type>(
				space.size(), [&space](std::size_t position) { return space.id(position); },
				[&space, &query](RecordId id, std::size_t) { return space.metric()(query, space.record(id)); },
				runtime, "approximate sampled neighbor recall");
			reference = core::take_nearest_neighbors(std::move(reference), count);
			mark_sampled_search_recall(result, reference, candidate_plan.candidate_universe);
		} else {
			mark_sampled_search_recall_unmeasured(result, candidate_plan.size(), candidate_plan.candidate_universe,
												  runtime_policy);
		}
	}
	return result;
}

template <typename Space>
auto approximate_sampled_neighbors(const Space &space, RecordId query_id, std::size_t count,
								   space::storage::policy runtime_policy,
								   ::mtrc::space::storage::runtime_guard runtime = {})
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;
	require_approximate_search_representation(runtime_policy);
	const auto query_position = space.position_of(query_id);

	if (count == 0) {
		auto result = core::make_neighbor_set(std::vector<Neighbor<distance_type>>{}, space.size(), count, "knn",
											  "sampled_metric_space", false);
		mark_approximate_sampled_search(result);
		mark_sampled_search_quality(result, 0, space.size() > 0 ? space.size() - 1 : 0, count);
		return result;
	}

	const auto candidate_count = approximate_candidate_count(space.size(), count, runtime_policy, true);
	const auto candidate_plan =
		::mtrc::space::regular_sample_positions_excluding(space.size(), query_position, candidate_count);
	std::vector<Neighbor<distance_type>> candidates;
	candidates.reserve(candidate_plan.size());
	for (const auto position : candidate_plan.positions) {
		runtime.throw_if_cancelled("approximate sampled neighbor search");
		const auto id = space.id(position);
		candidates.push_back(Neighbor<distance_type>{id, static_cast<distance_type>(space.distance(query_id, id))});
	}

	auto result = core::nearest_neighbor_set(std::move(candidates), count, space.size(), "sampled_metric_space");
	mark_approximate_sampled_search(result);
	mark_sampled_search_quality(result, candidate_plan.size(), candidate_plan.candidate_universe, count);
	const auto candidate_universe = candidate_plan.candidate_universe;
	if (count > 0) {
		if (can_measure_sampled_recall(candidate_plan.size(), candidate_universe, runtime_policy)) {
			auto reference = guarded_neighbor_candidates_if<distance_type>(
				space.size(), [&space](std::size_t position) { return space.id(position); },
				[&space, query_id](RecordId id, std::size_t) { return space.distance(query_id, id); },
				[query_id](RecordId id, std::size_t) { return id != query_id; },
				runtime, "approximate sampled neighbor recall");
			reference = core::take_nearest_neighbors(std::move(reference), count);
			mark_sampled_search_recall(result, reference, candidate_universe);
		} else {
			mark_sampled_search_recall_unmeasured(result, candidate_plan.size(), candidate_universe, runtime_policy);
		}
	}
	result.record_count = space.size();
	result.requested_count = count;
	return result;
}

template <typename Space, typename Radius>
auto approximate_sampled_range(const Space &space, const typename Space::record_type &query, Radius radius,
							   space::storage::policy runtime_policy,
							   ::mtrc::space::storage::runtime_guard runtime = {})
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;
	require_approximate_search_representation(runtime_policy);
	engine_detail::validate_radius(radius);

	const auto candidate_count = approximate_candidate_count(space.size(), 0, runtime_policy);
	const auto candidate_plan = ::mtrc::space::regular_sample_positions(space.size(), candidate_count);
	std::vector<Neighbor<distance_type>> candidates;
	candidates.reserve(candidate_plan.size());
	for (const auto position : candidate_plan.positions) {
		runtime.throw_if_cancelled("approximate sampled range search");
		const auto id = space.id(position);
		const auto distance = static_cast<distance_type>(space.metric()(query, space.record(id)));
		if (distance <= radius) {
			candidates.push_back(Neighbor<distance_type>{id, distance});
		}
	}

	auto result = core::range_neighbor_set(std::move(candidates), space.size(), "sampled_metric_space");
	mark_approximate_sampled_search(result);
	mark_sampled_search_quality(result, candidate_plan.size(), candidate_plan.candidate_universe, 0);
	if (can_measure_sampled_recall(candidate_plan.size(), candidate_plan.candidate_universe, runtime_policy)) {
		auto reference = guarded_neighbor_candidates_within<distance_type>(
			space.size(), [&space](std::size_t position) { return space.id(position); },
			[&space, &query](RecordId id, std::size_t) { return space.metric()(query, space.record(id)); }, radius,
			runtime, "approximate sampled range recall");
		core::sort_neighbors(reference);
		mark_sampled_search_recall(result, reference, candidate_plan.candidate_universe);
	} else {
		mark_sampled_search_recall_unmeasured(result, candidate_plan.size(), candidate_plan.candidate_universe,
											  runtime_policy);
	}
	return result;
}

template <typename Space, typename Radius>
auto approximate_sampled_range(const Space &space, RecordId query_id, Radius radius,
							   space::storage::policy runtime_policy,
							   ::mtrc::space::storage::runtime_guard runtime = {})
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;
	require_approximate_search_representation(runtime_policy);
	engine_detail::validate_radius(radius);
	const auto query_position = space.position_of(query_id);

	const auto candidate_count = approximate_candidate_count(space.size(), 0, runtime_policy, true);
	const auto candidate_plan =
		::mtrc::space::regular_sample_positions_excluding(space.size(), query_position, candidate_count);
	std::vector<Neighbor<distance_type>> candidates;
	candidates.reserve(candidate_plan.size());
	for (const auto position : candidate_plan.positions) {
		runtime.throw_if_cancelled("approximate sampled range search");
		const auto id = space.id(position);
		const auto distance = static_cast<distance_type>(space.distance(query_id, id));
		if (distance <= radius) {
			candidates.push_back(Neighbor<distance_type>{id, distance});
		}
	}

	auto result = core::range_neighbor_set(std::move(candidates), space.size(), "sampled_metric_space");
	mark_approximate_sampled_search(result);
	mark_sampled_search_quality(result, candidate_plan.size(), candidate_plan.candidate_universe, 0);
	const auto candidate_universe = candidate_plan.candidate_universe;
	if (can_measure_sampled_recall(candidate_plan.size(), candidate_universe, runtime_policy)) {
		auto reference = guarded_neighbor_candidates_within_if<distance_type>(
			space.size(), [&space](std::size_t position) { return space.id(position); },
			[&space, query_id](RecordId id, std::size_t) { return space.distance(query_id, id); }, radius,
			[query_id](RecordId id, std::size_t) { return id != query_id; },
			runtime, "approximate sampled range recall");
		core::sort_neighbors(reference);
		mark_sampled_search_recall(result, reference, candidate_universe);
	} else {
		mark_sampled_search_recall_unmeasured(result, candidate_plan.size(), candidate_universe, runtime_policy);
	}
	return result;
}

template <typename Space>
auto approximate_landmark_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
									space::storage::policy runtime_policy,
									::mtrc::space::storage::runtime_guard runtime = {})
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;
	require_approximate_search_representation(runtime_policy);
	if (count == 0) {
		auto result = core::make_neighbor_set(std::vector<Neighbor<distance_type>>{}, space.size(), count, "knn",
											  "landmark_index", false);
		mark_approximate_landmark_search(result, 0, space.size(), count, 0);
		mark_landmark_search_recall_unmeasured(
			result, landmark_recall_unmeasured_reason(
						count, space::storage::landmark_calibration_sample_query_count(space.size(), runtime_policy),
						space::storage::landmark_calibration_reference_candidate_limit(space.size(), 0),
						0, 0, runtime_policy));
		return result;
	}

	const auto candidate_limit = landmark_candidate_count(space.size(), runtime_policy, false, count);
	runtime.throw_if_cancelled("landmark index construction");
	space::storage::LandmarkIndex<Space> index(
		space, space::storage::landmark_index_options_from_policy(
				   landmark_count(space.size(), runtime_policy), candidate_limit, runtime_policy, runtime));
	runtime.throw_if_cancelled("landmark neighbor search");
	auto result = core::make_neighbor_set(
		index.knn(query, count, candidate_limit, runtime), space.size(), count, "knn", "landmark_index", false);
	mark_approximate_landmark_search(
		result, candidate_limit, space.size(), count,
		index.build_distance_evaluations() + index.query_landmark_distance_count() + candidate_limit);
	calibrate_landmark_knn_quality(
		result, index, count, candidate_limit, space.size(), runtime_policy, runtime);
	return result;
}

template <typename Space>
auto approximate_landmark_neighbors(const Space &space, RecordId query_id, std::size_t count,
									space::storage::policy runtime_policy,
									::mtrc::space::storage::runtime_guard runtime = {})
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;
	require_approximate_search_representation(runtime_policy);
	(void)space.record(query_id);
	const auto candidate_universe = space.size() > 0 ? space.size() - 1 : 0;
	if (count == 0) {
		auto result = core::make_neighbor_set(std::vector<Neighbor<distance_type>>{}, space.size(), count, "knn",
											  "landmark_index", false);
		mark_approximate_landmark_search(result, 0, candidate_universe, count, 0);
		mark_landmark_search_recall_unmeasured(
			result, landmark_recall_unmeasured_reason(
						count, space::storage::landmark_calibration_sample_query_count(space.size(), runtime_policy),
						space::storage::landmark_calibration_reference_candidate_limit(candidate_universe, 0),
						0, 0, runtime_policy));
		return result;
	}

	const auto candidate_limit = landmark_candidate_count(space.size(), runtime_policy, true, count);
	runtime.throw_if_cancelled("landmark index construction");
	space::storage::LandmarkIndex<Space> index(
		space, space::storage::landmark_index_options_from_policy(
				   landmark_count(space.size(), runtime_policy), candidate_limit, runtime_policy, runtime));
	runtime.throw_if_cancelled("landmark neighbor search");
	auto result = core::make_neighbor_set(
		index.knn(query_id, count, candidate_limit, runtime), space.size(), count, "knn", "landmark_index", false);
	mark_approximate_landmark_search(
		result, candidate_limit, candidate_universe, count,
		index.build_distance_evaluations() + index.query_landmark_distance_count() + candidate_limit);
	calibrate_landmark_knn_quality(
		result, index, count, candidate_limit, candidate_universe, runtime_policy, runtime);
	return result;
}

template <typename Space, typename Radius>
auto approximate_landmark_range(const Space &space, const typename Space::record_type &query, Radius radius,
								space::storage::policy runtime_policy,
								::mtrc::space::storage::runtime_guard runtime = {})
	-> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	const auto candidate_limit = landmark_candidate_count(space.size(), runtime_policy);
	runtime.throw_if_cancelled("landmark index construction");
	space::storage::LandmarkIndex<Space> index(
		space, space::storage::landmark_index_options_from_policy(
				   landmark_count(space.size(), runtime_policy), candidate_limit, runtime_policy, runtime));
	runtime.throw_if_cancelled("landmark range search");
	auto result = core::range_neighbor_set(
		index.range(query, radius, candidate_limit, runtime), space.size(), "landmark_index");
	mark_approximate_landmark_search(
		result, candidate_limit, space.size(), 0,
		index.build_distance_evaluations() + index.query_landmark_distance_count() + candidate_limit);
	calibrate_landmark_range_quality(
		result, index, radius, candidate_limit, space.size(), runtime_policy, runtime);
	return result;
}

template <typename Space, typename Radius>
auto approximate_landmark_range(const Space &space, RecordId query_id, Radius radius,
								space::storage::policy runtime_policy,
								::mtrc::space::storage::runtime_guard runtime = {})
	-> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	(void)space.record(query_id);
	const auto candidate_universe = space.size() > 0 ? space.size() - 1 : 0;
	const auto candidate_limit = landmark_candidate_count(space.size(), runtime_policy, true);
	runtime.throw_if_cancelled("landmark index construction");
	space::storage::LandmarkIndex<Space> index(
		space, space::storage::landmark_index_options_from_policy(
				   landmark_count(space.size(), runtime_policy), candidate_limit, runtime_policy, runtime));
	runtime.throw_if_cancelled("landmark range search");
	auto result = core::range_neighbor_set(
		index.range(query_id, radius, candidate_limit, runtime), space.size(), "landmark_index");
	mark_approximate_landmark_search(
		result, candidate_limit, candidate_universe, 0,
		index.build_distance_evaluations() + index.query_landmark_distance_count() + candidate_limit);
	calibrate_landmark_range_quality(
		result, index, radius, candidate_limit, candidate_universe, runtime_policy, runtime);
	return result;
}

template <typename MappingResult, typename std::enable_if<core::MappingResultLike_v<MappingResult>, int>::type = 0>
auto mapped_position_for_source_id(const MappingResult &mapping, RecordId source_id) -> std::size_t
{
	for (std::size_t position = 0; position < mapping.source_records.size(); ++position) {
		for (const auto candidate : mapping.source_records[position]) {
			if (candidate == source_id) {
				return position;
			}
		}
	}
	throw std::out_of_range("source RecordId is not represented in the mapping result");
}

using mapped_source_position_index = std::unordered_map<RecordId, std::size_t>;

template <typename MappingResult, typename std::enable_if<core::MappingResultLike_v<MappingResult>, int>::type = 0>
auto build_mapped_source_position_index(const MappingResult &mapping) -> mapped_source_position_index
{
	std::size_t source_count = 0;
	for (const auto &row : mapping.source_records) {
		source_count += row.size();
	}
	mapped_source_position_index index;
	index.reserve(source_count);
	for (std::size_t position = 0; position < mapping.source_records.size(); ++position) {
		for (const auto source_id : mapping.source_records[position]) {
			index.emplace(source_id, position);
		}
	}
	return index;
}

inline auto mapped_position_for_source_id(const mapped_source_position_index &index, RecordId source_id)
	-> std::size_t
{
	const auto found = index.find(source_id);
	if (found == index.end()) {
		throw std::out_of_range("source RecordId is not represented in the mapping result");
	}
	return found->second;
}

template <typename MappingResult, typename Distance,
		  typename std::enable_if<core::MappingResultLike_v<MappingResult>, int>::type = 0>
auto annotate_mapped_search_result(const MappingResult &mapping, RecordId source_query_id,
								   NeighborSet<Distance> &result) -> void
{
	result.exact = false;
	core::set_search_route(result.provenance, core::search_route_kind::mapped_space);
	core::set_search_domain(result.provenance, core::result_domain_kind::mapped_space);
	result.provenance.mapping = mapping.mapping;
	result.provenance.mapping_strategy = mapping.strategy;
	result.provenance.query_source_id = source_query_id;
	result.provenance.has_query_source_id = true;
	result.provenance.representative_route = true;
	result.provenance.source_neighbor_ids.clear();
	result.provenance.source_neighbor_lineage.clear();
	result.provenance.representative_neighbor_ids.clear();
	result.provenance.source_neighbor_lineage.reserve(result.neighbors.size());

	for (const auto &neighbor : result.neighbors) {
		const auto target_position = mapping.space.position_of(neighbor.id);
		if (target_position < mapping.source_records.size()) {
			result.provenance.source_neighbor_lineage.push_back(mapping.source_records[target_position]);
			for (const auto source_id : mapping.source_records[target_position]) {
				result.provenance.source_neighbor_ids.push_back(source_id);
			}
		} else {
			result.provenance.source_neighbor_lineage.push_back({});
		}
		if (target_position < mapping.representative_records.size()) {
			result.provenance.representative_neighbor_ids.push_back(mapping.representative_records[target_position]);
		}
	}
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto knn(const Space &space, const typename Space::record_type &query, std::size_t k,
		 ::mtrc::space::storage::runtime_guard runtime) -> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	auto candidates = detail::guarded_neighbor_candidates<distance_type>(
		space.size(), [&space](std::size_t index) { return space.id(index); },
		[&space, &query](RecordId id, std::size_t) { return space.metric()(query, space.record(id)); },
		runtime, "knn search");

	return core::nearest_neighbor_set(std::move(candidates), k, space.size(), "metric_space");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto knn(const Space &space, RecordId query_id, std::size_t k,
		 ::mtrc::space::storage::runtime_guard runtime) -> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	(void)space.record(query_id);
	auto candidates = detail::guarded_neighbor_candidates_if<distance_type>(
		space.size(), [&space](std::size_t index) { return space.id(index); },
		[&space, query_id](RecordId id, std::size_t) { return space.distance(query_id, id); },
		[query_id](RecordId id, std::size_t) { return id != query_id; }, runtime, "knn search");

	return core::nearest_neighbor_set(std::move(candidates), k, space.size(), "metric_space");
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto knn(const Provider &provider, RecordId query_id, std::size_t k,
		 ::mtrc::space::storage::runtime_guard runtime) -> NeighborSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (!provider.contains(query_id)) {
		throw std::out_of_range("query record id is outside the distance provider");
	}
	const auto query_position = provider.position_of(query_id);
	auto candidates = detail::guarded_neighbor_candidates_if<distance_type>(
		provider.record_count(), [&provider](std::size_t index) { return provider.id(index); },
		[&provider, query_id, query_position, runtime](RecordId id, std::size_t position) {
			return detail::guarded_provider_distance_from_query_position(
				provider, query_id, query_position, id, position, runtime);
		},
		[query_id](RecordId id, std::size_t) { return id != query_id; }, runtime, "pairwise knn search");

	return core::nearest_neighbor_set(std::move(candidates), k, provider.record_count(), "pairwise_distances");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius,
		   ::mtrc::space::storage::runtime_guard runtime) -> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	engine_detail::validate_radius(radius);
	auto candidates = detail::guarded_neighbor_candidates_within<distance_type>(
		space.size(), [&space](std::size_t index) { return space.id(index); },
		[&space, &query](RecordId id, std::size_t) { return space.metric()(query, space.record(id)); }, radius,
		runtime, "range search");

	return core::range_neighbor_set(std::move(candidates), space.size(), "metric_space");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius,
		   ::mtrc::space::storage::runtime_guard runtime) -> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	engine_detail::validate_radius(radius);
	(void)space.record(query_id);
	auto candidates = detail::guarded_neighbor_candidates_within_if<distance_type>(
		space.size(), [&space](std::size_t index) { return space.id(index); },
		[&space, query_id](RecordId id, std::size_t) { return space.distance(query_id, id); }, radius,
		[query_id](RecordId id, std::size_t) { return id != query_id; }, runtime, "range search");

	return core::range_neighbor_set(std::move(candidates), space.size(), "metric_space");
}

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto range(const Provider &provider, RecordId query_id, Radius radius,
		   ::mtrc::space::storage::runtime_guard runtime) -> NeighborSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	engine_detail::validate_radius(radius);
	if (!provider.contains(query_id)) {
		throw std::out_of_range("query record id is outside the distance provider");
	}
	const auto query_position = provider.position_of(query_id);
	auto candidates = detail::guarded_neighbor_candidates_within_if<distance_type>(
		provider.record_count(), [&provider](std::size_t index) { return provider.id(index); },
		[&provider, query_id, query_position, runtime](RecordId id, std::size_t position) {
			return detail::guarded_provider_distance_from_query_position(
				provider, query_id, query_position, id, position, runtime);
		},
		radius, [query_id](RecordId id, std::size_t) { return id != query_id; }, runtime,
		"pairwise range search");

	return core::range_neighbor_set(std::move(candidates), provider.record_count(), "pairwise_distances");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count)
	-> NeighborSet<typename Space::distance_type>
{
	return stats::search::knn(space, query, count);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value);
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_neighbors(const Container &records, const Metric &metric, const Record &query, std::size_t count)
	-> NeighborSet<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = find_neighbors(space, query, count);
	detail::set_search_representation(result, "records");
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_neighbors(const Container &records, const Metric &metric, const Record &query, ::mtrc::count requested)
	-> NeighborSet<metric_result_t<Metric, Record>>
{
	return find_neighbors(records, metric, query, requested.value);
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto knn_batch(const Container &records, const Metric &metric, const std::vector<Record> &queries, std::size_t count)
	-> std::vector<NeighborSet<metric_result_t<Metric, Record>>>
{
	auto space = core::make_space(records, metric);
	auto results = stats::search::knn_batch(space, queries, count);
	for (auto &result : results) {
		detail::set_search_representation(result, "records");
	}
	return results;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto knn_batch(const Container &records, const Metric &metric, const std::vector<Record> &queries, std::size_t count,
			   ::mtrc::space::storage::policy runtime_policy)
	-> std::vector<NeighborSet<metric_result_t<Metric, Record>>>
{
	auto space = core::make_space(records, metric);
	auto results = stats::search::knn_batch(space, queries, count, runtime_policy);
	for (auto &result : results) {
		detail::set_search_representation(result, "records");
	}
	return results;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto knn_batch(const Container &records, const Metric &metric, const std::vector<Record> &queries,
			   ::mtrc::count requested) -> std::vector<NeighborSet<metric_result_t<Metric, Record>>>
{
	return knn_batch(records, metric, queries, requested.value);
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto knn_batch(const Container &records, const Metric &metric, const std::vector<Record> &queries,
			   ::mtrc::count requested, ::mtrc::space::storage::policy runtime_policy)
	-> std::vector<NeighborSet<metric_result_t<Metric, Record>>>
{
	return knn_batch(records, metric, queries, requested.value, runtime_policy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count)
	-> NeighborSet<typename Space::distance_type>
{
	return stats::search::knn(space, query_id, count);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value);
}

template <typename MappingResult,
		  typename std::enable_if<core::MappingResultLike_v<MappingResult> &&
									  core::RecordMetricSpaceLike_v<typename MappingResult::space_type>,
								  int>::type = 0>
auto find_neighbors(const MappingResult &mapping, RecordId source_query_id, std::size_t count)
	-> NeighborSet<typename MappingResult::space_type::distance_type>
{
	core::require_mapping_result_contract(mapping, "find_neighbors(mapping)");
	const auto target_position = detail::mapped_position_for_source_id(mapping, source_query_id);
	auto result = find_neighbors(mapping.space, mapping.space.id(target_position), count);
	detail::annotate_mapped_search_result(mapping, source_query_id, result);
	return result;
}

template <typename MappingResult,
		  typename std::enable_if<core::MappingResultLike_v<MappingResult> &&
									  core::RecordMetricSpaceLike_v<typename MappingResult::space_type>,
								  int>::type = 0>
auto find_neighbors(const MappingResult &mapping, RecordId source_query_id, ::mtrc::count requested)
	-> NeighborSet<typename MappingResult::space_type::distance_type>
{
	return find_neighbors(mapping, source_query_id, requested.value);
}

template <typename MappingResult,
		  typename std::enable_if<core::MappingResultLike_v<MappingResult> &&
									  core::RecordMetricSpaceLike_v<typename MappingResult::space_type>,
								  int>::type = 0>
auto knn_batch(const MappingResult &mapping, const std::vector<RecordId> &source_query_ids, std::size_t count)
	-> std::vector<NeighborSet<typename MappingResult::space_type::distance_type>>
{
	core::require_mapping_result_contract(mapping, "knn_batch(mapping)");
	const auto source_positions = detail::build_mapped_source_position_index(mapping);
	std::vector<NeighborSet<typename MappingResult::space_type::distance_type>> results;
	results.reserve(source_query_ids.size());
	for (const auto source_query_id : source_query_ids) {
		const auto target_position = detail::mapped_position_for_source_id(source_positions, source_query_id);
		auto result = find_neighbors(mapping.space, mapping.space.id(target_position), count);
		detail::annotate_mapped_search_result(mapping, source_query_id, result);
		results.push_back(std::move(result));
	}
	return results;
}

template <typename MappingResult,
		  typename std::enable_if<core::MappingResultLike_v<MappingResult> &&
									  core::RecordMetricSpaceLike_v<typename MappingResult::space_type>,
								  int>::type = 0>
auto knn_batch(const MappingResult &mapping, const std::vector<RecordId> &source_query_ids, std::size_t count,
			   ::mtrc::space::storage::policy runtime_policy)
	-> std::vector<NeighborSet<typename MappingResult::space_type::distance_type>>
{
	core::require_mapping_result_contract(mapping, "knn_batch(mapping)");
	auto runtime = batch_detail::require_exact_batch_plan(
		mapping.space, "neighbors", source_query_ids.size(), runtime_policy);
	const auto source_positions = detail::build_mapped_source_position_index(mapping);
	std::vector<NeighborSet<typename MappingResult::space_type::distance_type>> results;
	results.reserve(source_query_ids.size());
	for (const auto source_query_id : source_query_ids) {
		runtime.throw_if_cancelled("knn_batch(mapping)");
		const auto target_position = detail::mapped_position_for_source_id(source_positions, source_query_id);
		auto result = find_neighbors(mapping.space, mapping.space.id(target_position), count);
		detail::annotate_mapped_search_result(mapping, source_query_id, result);
		results.push_back(std::move(result));
	}
	return results;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					stats::search::exact_scan) -> NeighborSet<typename Space::distance_type>
{
	space::storage::ExactScanIndex<Space> index(space);
	auto result = stats::search::knn(index, query, count);
	detail::set_search_representation(result, "exact_scan_index");
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested,
					stats::search::exact_scan strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, stats::search::exact_scan)
	-> NeighborSet<typename Space::distance_type>
{
	space::storage::ExactScanIndex<Space> index(space);
	(void)space.record(query_id);
	auto neighbors = index.knn(space.record(query_id), count + 1);
	return core::make_neighbor_set(core::take_neighbors_excluding_id(neighbors, query_id, count),
								   index.record_count(), count, "knn", "exact_scan_index");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested,
					stats::search::exact_scan strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					stats::search::brute_force) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, count, stats::search::exact_scan{});
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested,
					stats::search::brute_force strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, stats::search::brute_force)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, count, stats::search::exact_scan{});
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested,
					stats::search::brute_force strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, stats::search::distance_table)
	-> NeighborSet<typename Space::distance_type>
{
	space::storage::DistanceTable<Space> matrix(space);
	auto result = stats::search::knn(matrix, query_id, count);
	detail::set_search_representation(result, "distance_table");
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested, stats::search::distance_table strategy)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, strategy);
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius,
		   stats::search::exact_scan) -> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	space::storage::ExactScanIndex<Space> index(space);
	return detail::range_from_neighbors(index.knn(query, index.record_count()), radius, index.record_count(),
										"exact_scan_index");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, stats::search::exact_scan)
	-> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	(void)space.record(query_id);
	space::storage::ExactScanIndex<Space> index(space);
	auto neighbors = detail::exclude_neighbor_id(index.knn(space.record(query_id), index.record_count()), query_id);
	return detail::range_from_neighbors(std::move(neighbors), radius, index.record_count(), "exact_scan_index");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius,
		   stats::search::brute_force) -> NeighborSet<typename Space::distance_type>
{
	return range(space, query, radius, stats::search::exact_scan{});
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, stats::search::brute_force)
	-> NeighborSet<typename Space::distance_type>
{
	return range(space, query_id, radius, stats::search::exact_scan{});
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &, const typename Space::record_type &, Radius, stats::search::distance_table)
	-> NeighborSet<typename Space::distance_type>
{
	throw InvalidRuntimePolicyError("distance-table range strategy requires a RecordId query");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, stats::search::distance_table)
	-> NeighborSet<typename Space::distance_type>
{
	space::storage::DistanceTable<Space> matrix(space);
	auto result = stats::search::range(matrix, query_id, radius);
	detail::set_search_representation(result, "distance_table");
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					stats::search::cover_tree) -> NeighborSet<typename Space::distance_type>
{
	space::storage::CoverTreeIndex<Space> index(space);
	auto result = stats::search::knn(index, query, count);
	detail::set_search_representation(result, "cover_tree_index");
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested,
					stats::search::cover_tree strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, strategy);
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius,
		   stats::search::cover_tree) -> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	space::storage::CoverTreeIndex<Space> index(space);
	return detail::range_from_neighbors(index.knn(query, index.record_count()), radius, index.record_count(),
										"cover_tree_index");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, stats::search::cover_tree)
	-> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	(void)space.record(query_id);
	space::storage::CoverTreeIndex<Space> index(space);
	auto neighbors = detail::exclude_neighbor_id(index.knn(space.record(query_id), index.record_count()), query_id);
	return detail::range_from_neighbors(std::move(neighbors), radius, index.record_count(), "cover_tree_index");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					stats::search::knn_graph strategy) -> NeighborSet<typename Space::distance_type>
{
	const auto graph_neighbors = strategy.graph_neighbors == 0 ? count : strategy.graph_neighbors;
	space::storage::KnnGraphIndex<Space> index(space, graph_neighbors);
	auto result = stats::search::knn(index, query, count);
	detail::set_search_representation(result, "knn_graph_index");
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested,
					stats::search::knn_graph strategy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, strategy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, stats::search::knn_graph strategy)
	-> NeighborSet<typename Space::distance_type>
{
	if (query_id.index() >= space.size()) {
		throw std::out_of_range("query record id is outside the metric space");
	}
	const auto graph_neighbors = strategy.graph_neighbors == 0 ? count : strategy.graph_neighbors;
	const auto available_neighbors = space.size() > 0 ? space.size() - 1 : 0;
	const auto required_graph_neighbors = count < available_neighbors ? count : available_neighbors;
	if (graph_neighbors < required_graph_neighbors) {
		throw RepresentationError("kNN graph RecordId lookup requires graph_neighbors >= requested neighbor count");
	}
	space::storage::KnnGraphIndex<Space> index(space, graph_neighbors);

	NeighborSet<typename Space::distance_type> result;
	result.neighbors = index.neighbors(query_id);
	if (result.neighbors.size() > count) {
		result.neighbors.resize(count);
	}
	result.record_count = index.record_count();
	result.requested_count = count;
	result.exact = true;
	result.operator_name = "knn";
	detail::set_search_representation(result, "knn_graph_index");
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested, stats::search::knn_graph strategy)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, strategy);
}

template <typename Container, typename Metric, typename Strategy,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_neighbors(const Container &records, const Metric &metric, const Record &query, std::size_t count,
					Strategy strategy) -> NeighborSet<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = find_neighbors(space, query, count, strategy);
	return result;
}

template <typename Container, typename Metric, typename Strategy,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_neighbors(const Container &records, const Metric &metric, const Record &query, ::mtrc::count requested,
					Strategy strategy) -> NeighborSet<metric_result_t<Metric, Record>>
{
	return find_neighbors(records, metric, query, requested.value, strategy);
}

template <typename MappingResult, typename Strategy,
		  typename std::enable_if<core::MappingResultLike_v<MappingResult> &&
									  core::RecordMetricSpaceLike_v<typename MappingResult::space_type>,
								  int>::type = 0>
auto find_neighbors(const MappingResult &mapping, RecordId source_query_id, std::size_t count, Strategy strategy)
	-> NeighborSet<typename MappingResult::space_type::distance_type>
{
	core::require_mapping_result_contract(mapping, "find_neighbors(mapping,strategy)");
	const auto target_position = detail::mapped_position_for_source_id(mapping, source_query_id);
	auto result = find_neighbors(mapping.space, mapping.space.id(target_position), count, strategy);
	detail::annotate_mapped_search_result(mapping, source_query_id, result);
	return result;
}

template <typename MappingResult, typename Strategy,
		  typename std::enable_if<core::MappingResultLike_v<MappingResult> &&
									  core::RecordMetricSpaceLike_v<typename MappingResult::space_type>,
								  int>::type = 0>
auto find_neighbors(const MappingResult &mapping, RecordId source_query_id, ::mtrc::count requested, Strategy strategy)
	-> NeighborSet<typename MappingResult::space_type::distance_type>
{
	return find_neighbors(mapping, source_query_id, requested.value, strategy);
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius,
		   stats::search::knn_graph strategy) -> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	const auto graph_neighbors = strategy.graph_neighbors == 0 ? space.size() : strategy.graph_neighbors;
	space::storage::KnnGraphIndex<Space> index(space, graph_neighbors);
	return detail::range_from_neighbors(index.knn(query, index.record_count()), radius, index.record_count(),
										"knn_graph_index");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, stats::search::knn_graph strategy)
	-> NeighborSet<typename Space::distance_type>
{
	engine_detail::validate_radius(radius);
	(void)space.record(query_id);
	const auto available_neighbors = space.size() > 0 ? space.size() - 1 : 0;
	const auto graph_neighbors = strategy.graph_neighbors == 0 ? available_neighbors : strategy.graph_neighbors;
	if (graph_neighbors < available_neighbors) {
		throw RepresentationError("kNN graph range lookup requires graph_neighbors >= space.size() - 1");
	}
	space::storage::KnnGraphIndex<Space> index(space, graph_neighbors);
	return detail::range_from_neighbors(index.neighbors(query_id), radius, index.record_count(), "knn_graph_index");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, std::size_t count,
					space::storage::policy runtime_policy) -> NeighborSet<typename Space::distance_type>
{
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	auto runtime = ::mtrc::space::storage::runtime_guard(runtime_policy);
	if (runtime_policy.is_approximate()) {
		if (detail::uses_landmark_search(runtime_policy)) {
			return detail::approximate_landmark_neighbors(space, query, count, runtime_policy, runtime);
		}
		return detail::approximate_sampled_neighbors(space, query, count, runtime_policy, runtime);
	}
	space::storage::require_exact_neighbors(runtime_policy);
	const auto plan = detail::require_search_budget_plan(space, "neighbors", runtime_policy);
	if (!plan.exact) {
		if (detail::uses_chunked_search_plan(plan)) {
			return detail::chunked_plan_neighbors(space, query, count, plan, runtime_policy, runtime);
		}
		const auto fallback_policy = detail::approximate_search_fallback_policy(runtime_policy);
		if (detail::uses_landmark_search(fallback_policy)) {
			return detail::approximate_landmark_neighbors(space, query, count, fallback_policy, runtime);
		}
		return detail::approximate_sampled_neighbors(space, query, count, fallback_policy, runtime);
	}
	switch (runtime_policy.representation_mode()) {
	case space::storage::representation::distance_table:
		throw InvalidRuntimePolicyError("materialized neighbor runtime policy requires a RecordId query");
	case space::storage::representation::cover_tree: {
		runtime.throw_if_cancelled("cover_tree index construction");
		space::storage::CoverTreeIndex<Space> index(
			space, space::storage::cover_tree_index_options_from_policy(runtime_policy, runtime));
		runtime.throw_if_cancelled("cover_tree knn search");
		auto result = stats::search::knn(index, query, count);
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		return result;
	}
	case space::storage::representation::knn_graph: {
		const auto graph_neighbors = runtime_policy.graph_neighbors() == 0 ? count : runtime_policy.graph_neighbors();
		runtime.throw_if_cancelled("knn_graph index construction");
		space::storage::KnnGraphIndex<Space> index(space, graph_neighbors);
		runtime.throw_if_cancelled("knn_graph search");
		auto result = stats::search::knn(index, query, count);
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		return result;
	}
	case space::storage::representation::landmark_index:
		throw InvalidRuntimePolicyError("landmark neighbor runtime policy requires approximate accuracy");
	case space::storage::representation::implicit: {
		auto result = stats::search::knn(space, query, count, runtime);
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		return result;
	}
	case space::storage::representation::automatic: {
		if (runtime_policy.uses_materialization()) {
			throw InvalidRuntimePolicyError("materialized neighbor runtime policy requires a RecordId query");
		}
		auto result = stats::search::knn(space, query, count, runtime);
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		return result;
	}
	}
	return stats::search::knn(space, query, count, runtime);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, const typename Space::record_type &query, ::mtrc::count requested,
					space::storage::policy runtime_policy) -> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query, requested.value, runtime_policy);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, std::size_t count, space::storage::policy runtime_policy)
	-> NeighborSet<typename Space::distance_type>
{
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	(void)space.record(query_id);
	auto runtime = ::mtrc::space::storage::runtime_guard(runtime_policy);
	if (runtime_policy.is_approximate()) {
		if (detail::uses_landmark_search(runtime_policy)) {
			return detail::approximate_landmark_neighbors(space, query_id, count, runtime_policy, runtime);
		}
		return detail::approximate_sampled_neighbors(space, query_id, count, runtime_policy, runtime);
	}
	space::storage::require_exact_neighbors(runtime_policy);
	const auto plan = detail::require_search_budget_plan(space, "neighbors", runtime_policy);
	if (!plan.exact) {
		if (detail::uses_chunked_search_plan(plan)) {
			return detail::chunked_plan_neighbors(space, query_id, count, plan, runtime_policy, runtime);
		}
		const auto fallback_policy = detail::approximate_search_fallback_policy(runtime_policy);
		if (detail::uses_landmark_search(fallback_policy)) {
			return detail::approximate_landmark_neighbors(space, query_id, count, fallback_policy, runtime);
		}
		return detail::approximate_sampled_neighbors(space, query_id, count, fallback_policy, runtime);
	}
	if (runtime_policy.representation_mode() == space::storage::representation::distance_table ||
		(runtime_policy.representation_mode() == space::storage::representation::automatic &&
		 runtime_policy.uses_materialization())) {
		const auto materialized_plan =
			space::storage::require_materialized_operator_plan(space, runtime_policy, "neighbors");
		if (space::storage::uses_blocked_exact_fallback(materialized_plan)) {
			const auto provider = space::storage::make_blocked_distance_table(space, runtime_policy, runtime);
			auto result = stats::search::knn(provider, query_id, count, runtime);
			detail::set_search_representation(
				result, space::storage::materialized_operator_representation(materialized_plan));
			return result;
		}
		const auto provider = space::storage::make_distance_table(
			space, runtime_policy, space::storage::distance_table_mode::eager, runtime);
		auto result = stats::search::knn(provider, query_id, count, runtime);
		detail::set_search_representation(
			result, space::storage::materialized_operator_representation(materialized_plan));
		return result;
	}

	if (runtime_policy.representation_mode() == space::storage::representation::cover_tree) {
		runtime.throw_if_cancelled("cover_tree index construction");
		space::storage::CoverTreeIndex<Space> index(
			space, space::storage::cover_tree_index_options_from_policy(runtime_policy, runtime));
		NeighborSet<typename Space::distance_type> result;
		result.record_count = index.record_count();
		result.requested_count = count;
		result.exact = true;
		result.operator_name = "knn";
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		if (count == 0) {
			return result;
		}

		runtime.throw_if_cancelled("cover_tree knn search");
		const auto neighbors = index.knn(space.record(query_id), count + 1);
		result.neighbors = core::take_neighbors_excluding_id(neighbors, query_id, count);
		return result;
	}

	if (runtime_policy.representation_mode() == space::storage::representation::knn_graph) {
		const auto graph_neighbors = runtime_policy.graph_neighbors() == 0 ? count : runtime_policy.graph_neighbors();
		runtime.throw_if_cancelled("knn_graph index construction");
		auto result = find_neighbors(space, query_id, count, stats::search::knn_graph(graph_neighbors));
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		return result;
	}

	if (runtime_policy.representation_mode() == space::storage::representation::landmark_index) {
		throw InvalidRuntimePolicyError("landmark neighbor runtime policy requires approximate accuracy");
	}

	auto result = stats::search::knn(space, query_id, count, runtime);
	detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_neighbors(const Space &space, RecordId query_id, ::mtrc::count requested, space::storage::policy runtime_policy)
	-> NeighborSet<typename Space::distance_type>
{
	return find_neighbors(space, query_id, requested.value, runtime_policy);
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius,
		   space::storage::policy runtime_policy) -> NeighborSet<typename Space::distance_type>
{
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	auto runtime = ::mtrc::space::storage::runtime_guard(runtime_policy);
	if (runtime_policy.is_approximate()) {
		if (detail::uses_landmark_search(runtime_policy)) {
			return detail::approximate_landmark_range(space, query, radius, runtime_policy, runtime);
		}
		return detail::approximate_sampled_range(space, query, radius, runtime_policy, runtime);
	}
	space::storage::require_exact_neighbors(runtime_policy);
	const auto plan = detail::require_search_budget_plan(space, "range", runtime_policy);
	if (!plan.exact) {
		if (detail::uses_chunked_search_plan(plan)) {
			return detail::chunked_plan_range(space, query, radius, plan, runtime_policy, runtime);
		}
		const auto fallback_policy = detail::approximate_search_fallback_policy(runtime_policy);
		if (detail::uses_landmark_search(fallback_policy)) {
			return detail::approximate_landmark_range(space, query, radius, fallback_policy, runtime);
		}
		return detail::approximate_sampled_range(space, query, radius, fallback_policy, runtime);
	}
	switch (runtime_policy.representation_mode()) {
	case space::storage::representation::distance_table:
		throw InvalidRuntimePolicyError("materialized range runtime policy requires a RecordId query");
	case space::storage::representation::cover_tree: {
		runtime.throw_if_cancelled("cover_tree index construction");
		space::storage::CoverTreeIndex<Space> index(
			space, space::storage::cover_tree_index_options_from_policy(runtime_policy, runtime));
		runtime.throw_if_cancelled("cover_tree range search");
		auto result = detail::range_from_neighbors(index.knn(query, index.record_count()), radius,
												   index.record_count(), "cover_tree_index");
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		return result;
	}
	case space::storage::representation::knn_graph: {
		runtime.throw_if_cancelled("knn_graph range search");
		auto result = range(space, query, radius, stats::search::knn_graph(runtime_policy.graph_neighbors()));
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		return result;
	}
	case space::storage::representation::landmark_index:
		throw InvalidRuntimePolicyError("landmark range runtime policy requires approximate accuracy");
	case space::storage::representation::implicit: {
		auto result = stats::search::range(space, query, radius, runtime);
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		return result;
	}
	case space::storage::representation::automatic: {
		if (runtime_policy.uses_materialization()) {
			throw InvalidRuntimePolicyError("materialized range runtime policy requires a RecordId query");
		}
		auto result = stats::search::range(space, query, radius, runtime);
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		return result;
	}
	}
	return stats::search::range(space, query, radius, runtime);
}

template <typename Container, typename Metric, typename Radius,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto range(const Container &records, const Metric &metric, const Record &query, Radius radius)
	-> NeighborSet<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = stats::search::range(space, query, radius);
	detail::set_search_representation(result, "records");
	return result;
}

template <typename Container, typename Metric, typename Radius,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto range_batch(const Container &records, const Metric &metric, const std::vector<Record> &queries, Radius radius)
	-> std::vector<NeighborSet<metric_result_t<Metric, Record>>>
{
	auto space = core::make_space(records, metric);
	auto results = stats::search::range_batch(space, queries, radius);
	for (auto &result : results) {
		detail::set_search_representation(result, "records");
	}
	return results;
}

template <typename Container, typename Metric, typename Radius,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto range_batch(const Container &records, const Metric &metric, const std::vector<Record> &queries, Radius radius,
				 ::mtrc::space::storage::policy runtime_policy)
	-> std::vector<NeighborSet<metric_result_t<Metric, Record>>>
{
	auto space = core::make_space(records, metric);
	auto results = stats::search::range_batch(space, queries, radius, runtime_policy);
	for (auto &result : results) {
		detail::set_search_representation(result, "records");
	}
	return results;
}

template <typename Container, typename Metric, typename Radius, typename Strategy,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto range(const Container &records, const Metric &metric, const Record &query, Radius radius, Strategy strategy)
	-> NeighborSet<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = range(space, query, radius, strategy);
	return result;
}

template <typename MappingResult, typename Radius,
		  typename std::enable_if<core::MappingResultLike_v<MappingResult> &&
									  core::RecordMetricSpaceLike_v<typename MappingResult::space_type>,
								  int>::type = 0>
auto range(const MappingResult &mapping, RecordId source_query_id, Radius radius)
	-> NeighborSet<typename MappingResult::space_type::distance_type>
{
	core::require_mapping_result_contract(mapping, "range(mapping)");
	const auto target_position = detail::mapped_position_for_source_id(mapping, source_query_id);
	auto result = stats::search::range(mapping.space, mapping.space.id(target_position), radius);
	detail::annotate_mapped_search_result(mapping, source_query_id, result);
	return result;
}

template <typename MappingResult, typename Radius,
		  typename std::enable_if<core::MappingResultLike_v<MappingResult> &&
									  core::RecordMetricSpaceLike_v<typename MappingResult::space_type>,
								  int>::type = 0>
auto range_batch(const MappingResult &mapping, const std::vector<RecordId> &source_query_ids, Radius radius)
	-> std::vector<NeighborSet<typename MappingResult::space_type::distance_type>>
{
	engine_detail::validate_radius(radius);
	core::require_mapping_result_contract(mapping, "range_batch(mapping)");
	const auto source_positions = detail::build_mapped_source_position_index(mapping);
	std::vector<NeighborSet<typename MappingResult::space_type::distance_type>> results;
	results.reserve(source_query_ids.size());
	for (const auto source_query_id : source_query_ids) {
		const auto target_position = detail::mapped_position_for_source_id(source_positions, source_query_id);
		auto result = stats::search::range(mapping.space, mapping.space.id(target_position), radius);
		detail::annotate_mapped_search_result(mapping, source_query_id, result);
		results.push_back(std::move(result));
	}
	return results;
}

template <typename MappingResult, typename Radius,
		  typename std::enable_if<core::MappingResultLike_v<MappingResult> &&
									  core::RecordMetricSpaceLike_v<typename MappingResult::space_type>,
								  int>::type = 0>
auto range_batch(const MappingResult &mapping, const std::vector<RecordId> &source_query_ids, Radius radius,
				 ::mtrc::space::storage::policy runtime_policy)
	-> std::vector<NeighborSet<typename MappingResult::space_type::distance_type>>
{
	engine_detail::validate_radius(radius);
	core::require_mapping_result_contract(mapping, "range_batch(mapping)");
	auto runtime = batch_detail::require_exact_batch_plan(
		mapping.space, "range", source_query_ids.size(), runtime_policy);
	const auto source_positions = detail::build_mapped_source_position_index(mapping);
	std::vector<NeighborSet<typename MappingResult::space_type::distance_type>> results;
	results.reserve(source_query_ids.size());
	for (const auto source_query_id : source_query_ids) {
		runtime.throw_if_cancelled("range_batch(mapping)");
		const auto target_position = detail::mapped_position_for_source_id(source_positions, source_query_id);
		auto result = stats::search::range(mapping.space, mapping.space.id(target_position), radius);
		detail::annotate_mapped_search_result(mapping, source_query_id, result);
		results.push_back(std::move(result));
	}
	return results;
}

template <typename MappingResult, typename Radius, typename Strategy,
		  typename std::enable_if<core::MappingResultLike_v<MappingResult> &&
									  core::RecordMetricSpaceLike_v<typename MappingResult::space_type>,
								  int>::type = 0>
auto range(const MappingResult &mapping, RecordId source_query_id, Radius radius, Strategy strategy)
	-> NeighborSet<typename MappingResult::space_type::distance_type>
{
	core::require_mapping_result_contract(mapping, "range(mapping,strategy)");
	const auto target_position = detail::mapped_position_for_source_id(mapping, source_query_id);
	auto result = range(mapping.space, mapping.space.id(target_position), radius, strategy);
	detail::annotate_mapped_search_result(mapping, source_query_id, result);
	return result;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius, space::storage::policy runtime_policy)
	-> NeighborSet<typename Space::distance_type>
{
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	(void)space.record(query_id);
	auto runtime = ::mtrc::space::storage::runtime_guard(runtime_policy);
	if (runtime_policy.is_approximate()) {
		if (detail::uses_landmark_search(runtime_policy)) {
			return detail::approximate_landmark_range(space, query_id, radius, runtime_policy, runtime);
		}
		return detail::approximate_sampled_range(space, query_id, radius, runtime_policy, runtime);
	}
	space::storage::require_exact_neighbors(runtime_policy);
	const auto plan = detail::require_search_budget_plan(space, "range", runtime_policy);
	if (!plan.exact) {
		if (detail::uses_chunked_search_plan(plan)) {
			return detail::chunked_plan_range(space, query_id, radius, plan, runtime_policy, runtime);
		}
		const auto fallback_policy = detail::approximate_search_fallback_policy(runtime_policy);
		if (detail::uses_landmark_search(fallback_policy)) {
			return detail::approximate_landmark_range(space, query_id, radius, fallback_policy, runtime);
		}
		return detail::approximate_sampled_range(space, query_id, radius, fallback_policy, runtime);
	}
	if (runtime_policy.representation_mode() == space::storage::representation::distance_table ||
		(runtime_policy.representation_mode() == space::storage::representation::automatic &&
		 runtime_policy.uses_materialization())) {
		const auto materialized_plan =
			space::storage::require_materialized_operator_plan(space, runtime_policy, "range");
		if (space::storage::uses_blocked_exact_fallback(materialized_plan)) {
			const auto provider = space::storage::make_blocked_distance_table(space, runtime_policy, runtime);
			auto result = stats::search::range(provider, query_id, radius, runtime);
			detail::set_search_representation(
				result, space::storage::materialized_operator_representation(materialized_plan));
			return result;
		}
		const auto provider = space::storage::make_distance_table(
			space, runtime_policy, space::storage::distance_table_mode::eager, runtime);
		auto result = stats::search::range(provider, query_id, radius, runtime);
		detail::set_search_representation(
			result, space::storage::materialized_operator_representation(materialized_plan));
		return result;
	}

	if (runtime_policy.representation_mode() == space::storage::representation::cover_tree) {
		runtime.throw_if_cancelled("cover_tree index construction");
		space::storage::CoverTreeIndex<Space> index(
			space, space::storage::cover_tree_index_options_from_policy(runtime_policy, runtime));
		runtime.throw_if_cancelled("cover_tree range search");
		auto neighbors = detail::exclude_neighbor_id(index.knn(space.record(query_id), index.record_count()), query_id);
		auto result = detail::range_from_neighbors(std::move(neighbors), radius, index.record_count(),
												   "cover_tree_index");
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		return result;
	}

	if (runtime_policy.representation_mode() == space::storage::representation::knn_graph) {
		runtime.throw_if_cancelled("knn_graph range search");
		auto result = range(space, query_id, radius, stats::search::knn_graph(runtime_policy.graph_neighbors()));
		detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
		return result;
	}

	if (runtime_policy.representation_mode() == space::storage::representation::landmark_index) {
		throw InvalidRuntimePolicyError("landmark range runtime policy requires approximate accuracy");
	}

	auto result = stats::search::range(space, query_id, radius, runtime);
	detail::set_search_representation(result, space::storage::neighbor_representation(runtime_policy));
	return result;
}

} // namespace mtrc::stats::search

namespace mtrc {
using stats::search::find_neighbors;
} // namespace mtrc

#endif
