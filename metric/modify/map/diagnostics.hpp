// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_DIAGNOSTICS_HPP
#define _METRIC_MAPPINGS_DIAGNOSTICS_HPP

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/neighbor.hpp>
#include <metric/record/id.hpp>
#include <metric/stats/search/nearest.hpp>

namespace mtrc::modify::map {

inline constexpr std::size_t default_mapping_diagnostic_max_distance_evaluations = 1'000'000;

struct MappingDiagnosticsOptions {
	// Set to 0 only when the caller intentionally opts into an unbounded exact diagnostic scan.
	std::size_t max_distance_evaluations{default_mapping_diagnostic_max_distance_evaluations};
	std::size_t max_evaluated_items{};
	bool allow_sampling{true};
};

namespace diagnostics_detail {

template <typename Transform, typename Space, typename = void> struct DerivedSpaceTransformFor : std::false_type {};

template <typename Transform, typename Space>
struct DerivedSpaceTransformFor<
	Transform, Space,
	std::enable_if_t<::mtrc::MappingResultLike_v<decltype(std::declval<const Transform &>().transform(
		std::declval<const Space &>()))>>> : std::true_type {};

template <typename Transform, typename Space>
inline constexpr bool DerivedSpaceTransformFor_v = DerivedSpaceTransformFor<Transform, Space>::value;

struct diagnostic_work_plan {
	std::size_t evaluated_items{};
	bool exact{true};
	std::string reason;
};

inline auto checked_product(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
{
	if (rhs != 0 && lhs > std::numeric_limits<std::size_t>::max() / rhs) {
		throw RepresentationError(message);
	}
	return lhs * rhs;
}

inline auto sample_position(std::size_t sample_index, std::size_t sample_count, std::size_t total_count) -> std::size_t
{
	if (sample_count >= total_count) {
		return sample_index;
	}
	if (sample_count <= 1) {
		return total_count / 2;
	}
	return (sample_index * (total_count - 1)) / (sample_count - 1);
}

inline auto plan_diagnostic_work(std::size_t total_items, std::size_t distance_evaluations_per_item,
								 MappingDiagnosticsOptions options, const char *diagnostic_name) -> diagnostic_work_plan
{
	diagnostic_work_plan plan;
	plan.evaluated_items = total_items;

	if (options.max_evaluated_items > 0 && options.max_evaluated_items < plan.evaluated_items) {
		if (!options.allow_sampling) {
			throw RepresentationError(std::string(diagnostic_name) +
									  " diagnostics exceed max_evaluated_items: items=" +
									  std::to_string(total_items) + " max_evaluated_items=" +
									  std::to_string(options.max_evaluated_items));
		}
		plan.evaluated_items = options.max_evaluated_items;
		plan.exact = false;
		plan.reason = "sampled because diagnostic items exceed max_evaluated_items";
	}

	if (options.max_distance_evaluations == 0 || distance_evaluations_per_item == 0) {
		return plan;
	}

	const auto planned_evaluations = checked_product(
		plan.evaluated_items, distance_evaluations_per_item,
		"mapping diagnostics distance-evaluation estimate exceeds size_t capacity");
	if (planned_evaluations <= options.max_distance_evaluations) {
		return plan;
	}

	const auto budgeted_items = options.max_distance_evaluations / distance_evaluations_per_item;
	if (!options.allow_sampling || budgeted_items == 0) {
		throw RepresentationError(std::string(diagnostic_name) +
								  " diagnostics exceed max_distance_evaluations: items=" +
								  std::to_string(total_items) + " evaluations_per_item=" +
								  std::to_string(distance_evaluations_per_item) + " max_distance_evaluations=" +
								  std::to_string(options.max_distance_evaluations) +
								  "; allow sampling or raise the diagnostic budget");
	}

	plan.evaluated_items = std::min(plan.evaluated_items, budgeted_items);
	plan.exact = plan.evaluated_items == total_items;
	if (!plan.exact) {
		plan.reason = "sampled because diagnostic work exceeds max_distance_evaluations";
	}
	return plan;
}

template <typename Distance>
auto select_nearest_prefix(std::vector<core::Neighbor<Distance>> &neighbors, std::size_t count) -> std::size_t
{
	const auto selected_count = std::min(count, neighbors.size());
	if (selected_count == 0) {
		return 0;
	}
	if (selected_count >= neighbors.size()) {
		core::sort_neighbors(neighbors);
		return selected_count;
	}
	if (selected_count == 1) {
		const auto nearest = std::min_element(neighbors.begin(), neighbors.end(), core::NeighborLess<Distance>{});
		std::iter_swap(neighbors.begin(), nearest);
		return selected_count;
	}

	std::nth_element(
		neighbors.begin(), neighbors.begin() + selected_count, neighbors.end(), core::NeighborLess<Distance>{});
	std::sort(neighbors.begin(), neighbors.begin() + selected_count, core::NeighborLess<Distance>{});
	return selected_count;
}

template <typename Distance>
auto neighbor_ids_prefix(const std::vector<core::Neighbor<Distance>> &neighbors, std::size_t count)
	-> std::vector<RecordId>
{
	std::vector<RecordId> ids;
	ids.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		ids.push_back(neighbors[index].id);
	}
	return ids;
}

template <typename Distance>
auto neighbor_rank_by_distance_order(const std::vector<core::Neighbor<Distance>> &neighbors, RecordId id)
	-> std::size_t
{
	const auto target_position = core::neighbor_id_position(neighbors, id);
	if (target_position == neighbors.size()) {
		return neighbors.size() + 1;
	}
	const auto &target = neighbors[target_position];
	std::size_t rank = 1;
	const core::NeighborLess<Distance> less;
	for (const auto &neighbor : neighbors) {
		if (less(neighbor, target)) {
			++rank;
		}
	}
	return rank;
}

} // namespace diagnostics_detail

struct NeighborPreservationDiagnostics {
	std::size_t source_record_count{};
	std::size_t mapped_record_count{};
	std::size_t requested_neighbor_count{};
	std::size_t evaluated_neighbor_count{};
	std::size_t evaluated_rows{};
	std::size_t matched_neighbors{};
	std::size_t possible_neighbors{};
	std::size_t source_distance_evaluations{};
	std::size_t mapped_distance_evaluations{};
	std::size_t max_distance_evaluations{};
	double recall{};
	double average_row_recall{};
	double minimum_row_recall{};
	double maximum_row_recall{};
	bool exact{true};
	std::string diagnostic{"neighbor_preservation"};
	std::string source_representation{"metric_space"};
	std::string mapped_representation{"metric_space"};
	std::string reason;
};

struct OutOfSampleStabilityDiagnostics {
	std::size_t source_record_count{};
	std::size_t query_record_count{};
	std::size_t requested_neighbor_count{};
	std::size_t evaluated_neighbor_count{};
	std::size_t evaluated_queries{};
	std::size_t matched_anchor_neighbors{};
	std::size_t possible_anchor_neighbors{};
	std::size_t first_anchor_matches{};
	std::size_t source_distance_evaluations{};
	std::size_t mapped_distance_evaluations{};
	std::size_t max_distance_evaluations{};
	double anchor_recall{};
	double average_query_recall{};
	double minimum_query_recall{};
	double maximum_query_recall{};
	double first_anchor_match_rate{};
	double average_source_best_distance{};
	double average_mapped_best_source_distance{};
	double average_best_distance_penalty{};
	double maximum_best_distance_penalty{};
	double average_mapped_best_source_rank{};
	std::size_t maximum_mapped_best_source_rank{};
	bool transform_supported{true};
	bool exact{true};
	std::string diagnostic{"out_of_sample_neighbor_stability"};
	std::string mapping;
	std::string strategy;
	std::string source_representation{"metric_space"};
	std::string mapped_source_representation{"metric_space"};
	std::string mapped_query_representation{"metric_space"};
	std::string reason;
};

template <typename SourceSpace, typename MappingResult,
		  typename std::enable_if<
			  MetricSpaceLike_v<SourceSpace> && MetricSpaceLike_v<typename MappingResult::space_type>, int>::type = 0>
auto neighbor_preservation(const SourceSpace &source_space, const MappingResult &mapping_result,
						   std::size_t neighbor_count,
						   MappingDiagnosticsOptions options = {}) -> NeighborPreservationDiagnostics
{
	if (source_space.size() < 2) {
		throw std::invalid_argument("neighbor preservation diagnostics require at least two source records");
	}
	if (neighbor_count == 0) {
		throw std::invalid_argument("neighbor preservation diagnostics require a positive neighbor count");
	}
	if (mapping_result.space.size() != source_space.size()) {
		throw std::invalid_argument("neighbor preservation diagnostics require same-size one-to-one mappings");
	}

	const auto source_ids = mtrc::one_to_one_lineage_source_ids_in_space(
		source_space, mapping_result.source_records, mapping_result.space.size(),
		"neighbor preservation diagnostics require one source lineage row per mapped record",
		"neighbor preservation diagnostics require one-to-one mapping lineage",
		"neighbor preservation diagnostics require unique source record ids",
		"neighbor preservation diagnostics found lineage outside the source space");

	const auto effective_neighbors = std::min(neighbor_count, source_space.size() - 1);
	const auto source_evaluations_per_row = source_space.size() - 1;
	const auto mapped_evaluations_per_row = mapping_result.space.size() - 1;
	const auto evaluations_per_row = diagnostics_detail::checked_product(
		2, source_evaluations_per_row, "neighbor preservation distance-evaluation estimate exceeds size_t capacity");
	const auto work_plan = diagnostics_detail::plan_diagnostic_work(
		mapping_result.space.size(), evaluations_per_row, options, "neighbor preservation");

	NeighborPreservationDiagnostics diagnostics;
	diagnostics.source_record_count = source_space.size();
	diagnostics.mapped_record_count = mapping_result.space.size();
	diagnostics.requested_neighbor_count = neighbor_count;
	diagnostics.evaluated_neighbor_count = effective_neighbors;
	diagnostics.evaluated_rows = work_plan.evaluated_items;
	diagnostics.source_distance_evaluations = diagnostics_detail::checked_product(
		diagnostics.evaluated_rows, source_evaluations_per_row,
		"neighbor preservation source distance-evaluation estimate exceeds size_t capacity");
	diagnostics.mapped_distance_evaluations = diagnostics_detail::checked_product(
		diagnostics.evaluated_rows, mapped_evaluations_per_row,
		"neighbor preservation mapped distance-evaluation estimate exceeds size_t capacity");
	diagnostics.max_distance_evaluations = options.max_distance_evaluations;
	diagnostics.exact = work_plan.exact;
	diagnostics.reason = work_plan.reason;
	diagnostics.mapped_representation = mapping_result.representation;
	if (!diagnostics.exact) {
		diagnostics.source_representation = "metric_space_sample";
		diagnostics.mapped_representation = mapping_result.representation + "_sample";
	}

	core::RecallAccumulator recall;
	for (std::size_t sample_index = 0; sample_index < diagnostics.evaluated_rows; ++sample_index) {
		const auto row = diagnostics_detail::sample_position(
			sample_index, diagnostics.evaluated_rows, mapping_result.space.size());
		const auto source_id = source_ids[row];
		const auto mapped_id = mapping_result.space.id(row);
		const auto source_neighbors = stats::search::knn(source_space, source_id, effective_neighbors);
		const auto mapped_neighbors = stats::search::knn(mapping_result.space, mapped_id, effective_neighbors);

		const auto source_neighbor_ids =
			core::take_nearest_neighbor_ids(source_neighbors.neighbors, effective_neighbors);

		const auto mapped_source_neighbor_ids = core::source_ids_for_neighbors(
			source_ids, mapped_neighbors.neighbors,
			[&mapping_result](RecordId id) { return mapping_result.space.position_of(id); },
			"neighbor preservation diagnostics mapped neighbor is outside source lineage");
		const auto row_matches = mtrc::record_id_overlap_count(source_neighbor_ids, mapped_source_neighbor_ids);

		const auto possible = source_neighbor_ids.size();
		(void)recall.add(row_matches, possible);
	}

	diagnostics.matched_neighbors = recall.matched;
	diagnostics.possible_neighbors = recall.possible;
	diagnostics.recall = recall.total_recall();
	diagnostics.average_row_recall = recall.average_recall();
	diagnostics.minimum_row_recall = recall.minimum_recall_or();
	diagnostics.maximum_row_recall = recall.maximum_recall_or();
	return diagnostics;
}

template <typename Transform, typename SourceSpace, typename QuerySpace,
		  typename std::enable_if<
			  MetricSpaceLike_v<SourceSpace> && MetricSpaceLike_v<QuerySpace> &&
				  std::is_same<typename SourceSpace::record_type, typename QuerySpace::record_type>::value &&
				  diagnostics_detail::DerivedSpaceTransformFor_v<Transform, SourceSpace> &&
				  diagnostics_detail::DerivedSpaceTransformFor_v<Transform, QuerySpace>,
			  int>::type = 0>
auto out_of_sample_neighbor_stability(const Transform &mapping_artifact, const SourceSpace &source_space,
									  const QuerySpace &query_space, std::size_t neighbor_count,
									  MappingDiagnosticsOptions options = {})
	-> OutOfSampleStabilityDiagnostics
{
	if (source_space.empty()) {
		throw std::invalid_argument("out-of-sample stability diagnostics require a non-empty source space");
	}
	if (query_space.empty()) {
		throw std::invalid_argument("out-of-sample stability diagnostics require a non-empty query space");
	}
	if (neighbor_count == 0) {
		throw std::invalid_argument("out-of-sample stability diagnostics require a positive neighbor count");
	}

	const auto mapped_source = mapping_artifact.transform(source_space);
	const auto mapped_query = mapping_artifact.transform(query_space);
	if (mapped_source.space.size() != source_space.size()) {
		throw std::invalid_argument("out-of-sample stability diagnostics require same-size source transform output");
	}
	if (mapped_query.space.size() != query_space.size()) {
		throw std::invalid_argument("out-of-sample stability diagnostics require same-size query transform output");
	}

	const auto source_ids = mtrc::one_to_one_lineage_source_ids_in_space(
		source_space, mapped_source.source_records, mapped_source.space.size(),
		"out-of-sample stability diagnostics require one source lineage row per mapped record",
		"out-of-sample stability diagnostics require one-to-one mapping lineage",
		"out-of-sample stability diagnostics require unique source record ids",
		"out-of-sample stability diagnostics found lineage outside the source space");
	(void)mtrc::one_to_one_lineage_source_ids(
		mapped_query.source_records, mapped_query.space.size(),
		"out-of-sample stability diagnostics require one source lineage row per mapped record",
		"out-of-sample stability diagnostics require one-to-one mapping lineage",
		"out-of-sample stability diagnostics require unique source record ids");

	const auto effective_neighbors = std::min(neighbor_count, source_space.size());
	const auto source_evaluations_per_query = source_space.size();
	const auto mapped_evaluations_per_query = mapped_source.space.size();
	const auto evaluations_per_query =
		diagnostics_detail::checked_product(2, source_evaluations_per_query,
											"out-of-sample stability distance-evaluation estimate exceeds size_t capacity");
	const auto work_plan = diagnostics_detail::plan_diagnostic_work(
		query_space.size(), evaluations_per_query, options, "out-of-sample stability");

	OutOfSampleStabilityDiagnostics diagnostics;
	diagnostics.source_record_count = source_space.size();
	diagnostics.query_record_count = query_space.size();
	diagnostics.requested_neighbor_count = neighbor_count;
	diagnostics.evaluated_neighbor_count = effective_neighbors;
	diagnostics.evaluated_queries = work_plan.evaluated_items;
	diagnostics.source_distance_evaluations = diagnostics_detail::checked_product(
		diagnostics.evaluated_queries, source_evaluations_per_query,
		"out-of-sample stability source distance-evaluation estimate exceeds size_t capacity");
	diagnostics.mapped_distance_evaluations = diagnostics_detail::checked_product(
		diagnostics.evaluated_queries, mapped_evaluations_per_query,
		"out-of-sample stability mapped distance-evaluation estimate exceeds size_t capacity");
	diagnostics.max_distance_evaluations = options.max_distance_evaluations;
	diagnostics.exact = work_plan.exact;
	diagnostics.reason = work_plan.reason;
	diagnostics.mapping = mapped_source.mapping;
	diagnostics.strategy = mapped_source.strategy;
	diagnostics.mapped_source_representation = mapped_source.representation;
	diagnostics.mapped_query_representation = mapped_query.representation;
	if (!diagnostics.exact) {
		diagnostics.source_representation = "metric_space_sample";
		diagnostics.mapped_source_representation = mapped_source.representation + "_sample";
		diagnostics.mapped_query_representation = mapped_query.representation + "_sample";
	}

	core::RecallAccumulator recall;
	core::ScalarAccumulator<double> source_best_distance_summary;
	core::ScalarAccumulator<double> mapped_best_source_distance_summary;
	core::ScalarAccumulator<double> best_distance_penalty_summary;
	core::ScalarAccumulator<std::size_t> mapped_best_source_rank_summary;
	for (std::size_t sample_index = 0; sample_index < diagnostics.evaluated_queries; ++sample_index) {
		const auto query_position = diagnostics_detail::sample_position(
			sample_index, diagnostics.evaluated_queries, query_space.size());
		const auto query_id = query_space.id(query_position);
		const auto mapped_query_id = mapped_query.space.id(query_position);

		const auto &query_record = query_space.record(query_id);
		auto source_candidates = core::neighbor_candidates<typename SourceSpace::distance_type>(
			source_space.size(), [&source_space](std::size_t source_position) { return source_space.id(source_position); },
			[&source_space, &query_record](RecordId source_id, std::size_t) {
					return source_space.metric()(query_record, source_space.record(source_id));
				});
			const auto source_neighbor_count =
				diagnostics_detail::select_nearest_prefix(source_candidates, effective_neighbors);
			const auto source_neighbor_ids =
				diagnostics_detail::neighbor_ids_prefix(source_candidates, source_neighbor_count);

			const auto &mapped_query_record = mapped_query.space.record(mapped_query_id);
			auto mapped_candidates =
			core::neighbor_candidates<typename std::decay<decltype(mapped_source.space)>::type::distance_type>(
				mapped_source.space.size(),
				[&source_ids](std::size_t source_position) { return source_ids[source_position]; },
				[&mapped_source, &mapped_query_record](RecordId, std::size_t source_position) {
						const auto mapped_source_id = mapped_source.space.id(source_position);
						return mapped_source.space.metric()(mapped_query_record, mapped_source.space.record(mapped_source_id));
					});
			const auto mapped_neighbor_count =
				diagnostics_detail::select_nearest_prefix(mapped_candidates, effective_neighbors);
			const auto mapped_neighbor_ids =
				diagnostics_detail::neighbor_ids_prefix(mapped_candidates, mapped_neighbor_count);

		const auto query_matches = mtrc::record_id_overlap_count(source_neighbor_ids, mapped_neighbor_ids);

		const auto possible = source_neighbor_ids.size();
		(void)recall.add(query_matches, possible);

			const auto source_best_id = source_candidates.front().id;
			const auto mapped_best_id = mapped_candidates.front().id;
		if (source_best_id == mapped_best_id) {
			++diagnostics.first_anchor_matches;
		}
		const auto source_best_distance = static_cast<double>(source_candidates.front().distance);
			const auto mapped_best_source_distance =
				static_cast<double>(core::neighbor_distance_or_throw(
					source_candidates, mapped_best_id, "diagnostics could not find source anchor distance"));
			const auto mapped_best_source_rank =
				diagnostics_detail::neighbor_rank_by_distance_order(source_candidates, mapped_best_id);
		const auto distance_penalty = mapped_best_source_distance - source_best_distance;
		source_best_distance_summary.add(source_best_distance);
		mapped_best_source_distance_summary.add(mapped_best_source_distance);
		best_distance_penalty_summary.add(distance_penalty);
		mapped_best_source_rank_summary.add(mapped_best_source_rank);
	}

	diagnostics.matched_anchor_neighbors = recall.matched;
	diagnostics.possible_anchor_neighbors = recall.possible;
	diagnostics.anchor_recall = recall.total_recall();
	diagnostics.average_query_recall = recall.average_recall();
	diagnostics.minimum_query_recall = recall.minimum_recall_or();
	diagnostics.maximum_query_recall = recall.maximum_recall_or();
	if (diagnostics.evaluated_queries > 0) {
		diagnostics.first_anchor_match_rate =
			static_cast<double>(diagnostics.first_anchor_matches) / static_cast<double>(diagnostics.evaluated_queries);
		diagnostics.average_source_best_distance = source_best_distance_summary.average_or();
		diagnostics.average_mapped_best_source_distance = mapped_best_source_distance_summary.average_or();
		diagnostics.average_best_distance_penalty = best_distance_penalty_summary.average_or();
		diagnostics.maximum_best_distance_penalty = best_distance_penalty_summary.maximum_or();
		diagnostics.average_mapped_best_source_rank = mapped_best_source_rank_summary.average_or();
		diagnostics.maximum_mapped_best_source_rank = mapped_best_source_rank_summary.maximum_or();
	}
	return diagnostics;
}

} // namespace mtrc::modify::map

#endif
