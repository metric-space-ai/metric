// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RUNTIME_EXECUTION_HPP
#define _METRIC_RUNTIME_EXECUTION_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

#include <metric/core/errors.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/space/chunked.hpp>
#include "blocked_distance_table.hpp"
#include "distance_table.hpp"
#include "policy.hpp"

namespace mtrc::space::storage {

enum class budget_decision {
	allowed,
	downgraded,
	refused,
};

inline auto budget_decision_name(budget_decision decision) -> std::string
{
	switch (decision) {
	case budget_decision::allowed:
		return "allowed";
	case budget_decision::downgraded:
		return "downgraded";
	case budget_decision::refused:
		return "refused";
	}
	return "refused";
}

struct RuntimeDiagnostics {
	std::string policy_name;
	bool exact{true};
	bool parallel{false};
	bool materialized{false};
	std::string representation;
	std::string intent;
	bool supported{true};
	std::string reason;
	std::size_t record_count{};
	std::size_t dense_distance_slots{};
	std::size_t estimated_distance_evaluations{};
	bool chunked_plan{false};
	std::size_t chunk_size{};
	std::size_t chunk_count{};
	std::size_t max_local_record_count{};
	std::size_t local_pair_distance_evaluations{};
	std::size_t dense_pair_distance_evaluations{};
	std::size_t representative_count{};
	std::size_t representative_pair_distance_evaluations{};
	std::size_t bounded_pair_distance_evaluations{};
	bool out_of_core_plan{false};
	bool spill_enabled{false};
	bool explicit_spill_policy_required{false};
	bool memory_bounded{false};
	std::size_t spill_block_size{};
	std::size_t spill_block_count{};
	std::size_t planned_spill_blocks{};
	std::size_t max_resident_blocks{};
	std::size_t spill_block_bytes_estimate{};
	std::size_t max_resident_bytes_estimate{};
	std::size_t spill_bytes_estimate{};
	std::size_t memory_bytes_estimate{};
	std::size_t max_dense_records{};
	std::size_t max_memory_bytes{};
	std::size_t max_distance_evaluations{};
	std::size_t max_runtime_ms{};
	std::size_t max_spill_bytes{};
	bool allow_approximate{false};
	bool allow_chunking{false};
	bool allow_out_of_core_spill{false};
	bool budget_exceeded{false};
	resource_budget budget;
	budget_decision decision{budget_decision::allowed};
	bool allowed{true};
	bool downgraded{false};
	bool refused{false};
	std::string fallback_hint;
};

struct execution_plan {
	std::string policy_name;
	std::string intent;
	std::size_t record_count{};
	std::size_t query_count{};
	std::size_t estimated_distance_evaluations{};
	std::size_t dense_distance_slots{};
	bool chunked_plan{false};
	std::size_t chunk_size{};
	std::size_t chunk_count{};
	std::size_t max_local_record_count{};
	std::size_t local_pair_distance_evaluations{};
	std::size_t dense_pair_distance_evaluations{};
	std::size_t representative_count{};
	std::size_t representative_pair_distance_evaluations{};
	std::size_t bounded_pair_distance_evaluations{};
	bool out_of_core_plan{false};
	bool spill_enabled{false};
	bool explicit_spill_policy_required{false};
	bool memory_bounded{false};
	std::size_t spill_block_size{};
	std::size_t spill_block_count{};
	std::size_t planned_spill_blocks{};
	std::size_t max_resident_blocks{};
	std::size_t spill_block_bytes_estimate{};
	std::size_t max_resident_bytes_estimate{};
	std::size_t spill_bytes_estimate{};
	std::size_t memory_bytes_estimate{};
	bool exact{true};
	std::string exactness{"exact"};
	std::string requested_representation;
	std::string representation;
	resource_budget budget;
	std::size_t max_runtime_ms{};
	std::size_t max_spill_bytes{};
	budget_decision decision{budget_decision::allowed};
	bool budget_exceeded{false};
	bool allowed{true};
	bool downgraded{false};
	bool refused{false};
	std::string reason;
	std::string fallback_hint;
};

template <typename Space>
inline auto estimate_cost(const Space &space, std::string intent, policy runtime_policy = exact()) -> execution_plan;

inline auto require_exact_neighbors(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw InvalidRuntimePolicyError("approximate neighbor runtime policy is not implemented");
	}
}

inline auto require_exact_groups(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw InvalidRuntimePolicyError("approximate grouping runtime policy is not implemented");
	}
}

inline auto require_exact_outliers(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw InvalidRuntimePolicyError("approximate outlier runtime policy is not implemented");
	}
}

inline auto require_exact_density_filter(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw InvalidRuntimePolicyError("approximate density-filter runtime policy is not implemented");
	}
}

inline auto require_exact_representatives(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw InvalidRuntimePolicyError("approximate representative runtime policy is not implemented");
	}
}

inline auto require_exact_compress(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw InvalidRuntimePolicyError("approximate compression runtime policy is not implemented");
	}
}

inline auto require_exact_describe(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw InvalidRuntimePolicyError("approximate structure-description runtime policy is not implemented");
	}
}

inline auto require_exact_compare(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw InvalidRuntimePolicyError("approximate comparison runtime policy is not implemented");
	}
}

inline auto require_exact_embed(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw InvalidRuntimePolicyError("approximate embedding runtime policy is not implemented");
	}
}

inline auto require_exact_reduce(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw InvalidRuntimePolicyError("approximate reduction runtime policy is not implemented");
	}
}

inline auto require_exact_map(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw InvalidRuntimePolicyError("approximate mapping runtime policy is not implemented");
	}
}

inline auto require_lazy_embed(policy runtime_policy) -> void
{
	if (runtime_policy.uses_materialization()) {
		throw InvalidRuntimePolicyError("materialized embedding runtime policy is not implemented");
	}
}

inline auto require_lazy_reduce(policy runtime_policy) -> void
{
	if (runtime_policy.uses_materialization()) {
		throw InvalidRuntimePolicyError("materialized reduction runtime policy is not implemented");
	}
}

inline auto require_lazy_map(policy runtime_policy) -> void
{
	if (runtime_policy.uses_materialization()) {
		throw InvalidRuntimePolicyError("materialized mapping runtime policy is not implemented");
	}
}

template <typename Metric> constexpr auto supports_parallel_metric(policy runtime_policy) noexcept -> bool
{
	return !runtime_policy.uses_parallel_execution() || core::metric_thread_safe_v<Metric>;
}

template <typename Metric> inline auto require_parallel_metric(policy runtime_policy) -> void
{
	if (!supports_parallel_metric<Metric>(runtime_policy)) {
		throw InvalidRuntimePolicyError("parallel runtime policy requires a thread-safe metric");
	}
}

inline auto execution_representation(policy runtime_policy) -> std::string
{
	switch (runtime_policy.representation_mode()) {
	case representation::implicit:
		return "implicit";
	case representation::distance_table:
		return "distance_table";
	case representation::cover_tree:
		return "cover_tree_index";
	case representation::knn_graph:
		return "knn_graph_index";
	case representation::landmark_index:
		return "landmark_index";
	case representation::automatic:
		return runtime_policy.uses_materialization() ? "distance_table" : "metric_space";
	}
	return "metric_space";
}

inline auto neighbor_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto group_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto outlier_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto density_filter_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto representative_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto compression_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto describe_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto compare_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto embed_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto reduce_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto map_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto uses_distance_table_materialization(policy runtime_policy) noexcept -> bool
{
	return runtime_policy.representation_mode() == representation::distance_table ||
		   (runtime_policy.representation_mode() == representation::automatic &&
			runtime_policy.uses_materialization());
}

inline auto fallback_hint_for_distance_budget(const resource_budget &budget) -> std::string
{
	if (budget.allow_chunking) {
		return budget.allow_out_of_core_spill
				   ? "use blocked/chunked exact execution with disk spill before constructing a dense distance table"
				   : "use blocked/chunked exact execution before constructing a dense distance table";
	}
	if (budget.allow_approximate) {
		return "use an approximate or sampled representation instead of a dense distance table";
	}
	return "use space::storage::using_implicit(), an exact scan, or raise the resource_budget";
}

inline auto bounded_block_size_for_budget(std::size_t requested_block_size, std::size_t max_memory_bytes,
										  std::size_t distance_size, std::size_t max_cached_blocks)
	-> std::size_t
{
	auto block_size = requested_block_size == 0 ? std::size_t{1} : requested_block_size;
	if (max_memory_bytes == 0 || distance_size == 0 || max_cached_blocks == 0) {
		return block_size;
	}

	const auto max_distance_cells = max_memory_bytes / distance_size / max_cached_blocks;
	if (max_distance_cells == 0) {
		return 1;
	}

	std::size_t bounded = 1;
	while (bounded < block_size) {
		const auto next = bounded + 1;
		if (next > max_distance_cells / next) {
			break;
		}
		bounded = next;
	}
	return bounded;
}

template <typename Distance>
inline auto blocked_distance_table_options_for(policy runtime_policy, const resource_budget &budget,
											   runtime_guard runtime = {}) -> blocked_distance_table_options
{
	constexpr auto default_block_size = std::size_t{256};
	constexpr auto default_cached_blocks = std::size_t{4};
	auto block_size = default_block_size;
	(void)runtime_policy;
	if (budget.max_dense_records > 0) {
		block_size = std::min(block_size, budget.max_dense_records);
	}
	block_size = bounded_block_size_for_budget(
		block_size, budget.max_memory_bytes, sizeof(Distance), default_cached_blocks);
	auto options = blocked_distance_table_options{block_size, default_cached_blocks, runtime};
	options.spill_to_disk = budget.allow_out_of_core_spill;
	options.max_spill_bytes = budget.max_spill_bytes;
	return options;
}

template <typename Distance>
inline auto blocked_distance_table_options_for(policy runtime_policy,
											   runtime_guard runtime = {}) -> blocked_distance_table_options
{
	return blocked_distance_table_options_for<Distance>(
		runtime_policy, budget_from_policy(runtime_policy), runtime);
}

inline auto estimate_live_distance_evaluations(std::size_t record_count, const std::string &intent,
											   std::size_t query_count = 0) -> std::size_t
{
	if (intent == "compare") {
		return detail::checked_distance_table_size_product(
			detail::distance_table_dense_slot_count(record_count), 2,
			"compare distance-evaluation estimate exceeds size_t capacity");
	}
	if (intent == "groups" || intent == "outliers" || intent == "density_filter" ||
		intent == "describe" ||
		intent == "workflow" || intent == "workflow_context") {
		return detail::distance_table_dense_slot_count(record_count);
	}
	if (intent == "embed" || intent == "reduce" || intent == "map") {
		return record_count;
	}
	if ((intent == "neighbors" || intent == "range") && query_count > 1) {
		return detail::checked_distance_table_size_product(
			record_count, query_count, "search distance-evaluation estimate exceeds size_t capacity");
	}
	return record_count;
}

inline auto estimate_knn_graph_build_distance_evaluations(std::size_t record_count) -> std::size_t
{
	if (record_count < 2) {
		return 0;
	}
	return detail::checked_distance_table_size_product(
		record_count, record_count - 1, "knn graph build distance-evaluation estimate exceeds size_t capacity");
}

inline auto estimate_knn_graph_distance_evaluations(std::size_t record_count, const std::string &intent,
													std::size_t query_count) -> std::size_t
{
	const auto build = estimate_knn_graph_build_distance_evaluations(record_count);
	const auto queries = query_count == 0 ? std::size_t{1} : query_count;
	const auto query_evaluations =
		(intent == "neighbors" || intent == "range")
			? detail::checked_distance_table_size_product(
				  record_count, queries, "knn graph query distance-evaluation estimate exceeds size_t capacity")
			: std::size_t{0};
	return detail::checked_distance_table_size_sum(
		build, query_evaluations, "knn graph distance-evaluation estimate exceeds size_t capacity");
}

template <typename Distance>
inline auto estimate_knn_graph_memory_bytes(std::size_t record_count, std::size_t graph_neighbors) -> std::size_t
{
	if (record_count == 0) {
		return 0;
	}
	const auto available_neighbors = record_count > 0 ? record_count - 1 : std::size_t{0};
	const auto stored_neighbors =
		graph_neighbors == 0 ? available_neighbors : std::min(graph_neighbors, available_neighbors);
	const auto edge_count = detail::checked_distance_table_size_product(
		record_count, stored_neighbors, "knn graph edge-count estimate exceeds size_t capacity");
	const auto edge_bytes = sizeof(Distance) + sizeof(std::size_t);
	return detail::checked_distance_table_size_product(
		edge_count, edge_bytes, "knn graph memory estimate exceeds size_t capacity");
}

inline auto estimate_approximate_compare_distance_evaluations(std::size_t record_count) -> std::size_t
{
	const auto sample_count = record_count < 250 ? record_count : std::size_t{250};
	if (sample_count == 0) {
		return 0;
	}
	const auto iterations = sample_count >= record_count ? std::size_t{1} : record_count / sample_count;
	const auto sample_slots = detail::distance_table_dense_slot_count(sample_count);
	return detail::checked_distance_table_size_product(
		detail::checked_distance_table_size_product(
			sample_slots, iterations == 0 ? std::size_t{1} : iterations,
			"compare distance-evaluation estimate exceeds size_t capacity"),
			2, "compare distance-evaluation estimate exceeds size_t capacity");
}

inline auto estimate_approximate_search_distance_evaluations(std::size_t record_count, std::size_t query_count = 0)
	-> std::size_t
{
	const auto per_query = record_count < 4096 ? record_count : std::size_t{4096};
	const auto queries = query_count == 0 ? std::size_t{1} : query_count;
	return detail::checked_distance_table_size_product(
		per_query, queries, "approximate search distance-evaluation estimate exceeds size_t capacity");
}

template <typename Metric> constexpr auto supports_landmark_search_metric() noexcept -> bool
{
	constexpr auto law = core::metric_traits<Metric>::law;
	return law == core::metric_law::metric || law == core::metric_law::pseudo_metric;
}

inline constexpr std::size_t default_landmark_calibration_sample_queries = 4;
inline constexpr std::size_t default_landmark_calibration_reference_candidates = 1024;

inline auto landmark_integer_sqrt(std::size_t value) -> std::size_t
{
	std::size_t root = 0;
	while ((root + 1) <= value / (root + 1)) {
		++root;
	}
	return root;
}

inline auto landmark_index_landmark_count(std::size_t record_count, policy runtime_policy,
										  std::size_t query_count = 0) -> std::size_t
{
	if (record_count == 0) {
		return 0;
	}

	auto landmarks = std::min(default_landmark_index_landmarks, record_count);
	if (record_count > 8) {
		landmarks = std::min(landmarks, std::max(std::size_t{8}, landmark_integer_sqrt(record_count)));
	}

	const auto budget = runtime_policy.max_distance_evaluations();
	if (budget == 0) {
		return landmarks;
	}

	const auto queries = query_count == 0 ? std::size_t{1} : query_count;
	while (landmarks > 1) {
		if (record_count <= budget / landmarks && queries <= budget / landmarks) {
			break;
		}
		landmarks = (landmarks + 1) / 2;
	}
	return std::min(landmarks, record_count);
}

inline auto landmark_index_candidate_limit(std::size_t record_count, policy runtime_policy,
										   bool excludes_query_record = false,
										   std::size_t requested_count = 0,
										   std::size_t query_count = 0) -> std::size_t
{
	const auto available = excludes_query_record && record_count > 0 ? record_count - 1 : record_count;
	if (available == 0) {
		return 0;
	}
	if (runtime_policy.graph_neighbors() != 0) {
		return std::min(runtime_policy.graph_neighbors(), available);
	}

	const auto root = landmark_integer_sqrt(record_count);
	auto scaled = root <= default_landmark_index_candidates / 16
					  ? root * 16
					  : default_landmark_index_candidates;
	auto candidate_limit = std::min(default_landmark_index_candidates,
									std::max(std::size_t{64}, scaled));
	candidate_limit = std::min(std::max(candidate_limit, requested_count), available);

	const auto budget = runtime_policy.max_distance_evaluations();
	if (budget == 0) {
		return candidate_limit;
	}

	const auto landmarks = landmark_index_landmark_count(record_count, runtime_policy, query_count);
	if (landmarks == 0 || record_count > budget / landmarks) {
		return candidate_limit;
	}
	const auto build = record_count * landmarks;
	if (build >= budget) {
		return candidate_limit;
	}
	const auto queries = query_count == 0 ? std::size_t{1} : query_count;
	const auto per_query_budget = (budget - build) / queries;
	if (per_query_budget <= landmarks) {
		return std::min(candidate_limit, requested_count);
	}
	const auto budgeted_candidates = per_query_budget - landmarks;
	return std::min(candidate_limit, std::max(requested_count, budgeted_candidates));
}

inline auto landmark_calibration_sample_query_count(std::size_t record_count, policy runtime_policy,
													std::size_t query_count = 0) -> std::size_t
{
	(void)runtime_policy;
	(void)query_count;
	return std::min(default_landmark_calibration_sample_queries, record_count);
}

inline auto landmark_calibration_reference_candidate_limit(std::size_t candidate_universe,
														   std::size_t candidate_limit) -> std::size_t
{
	if (candidate_universe == 0) {
		return 0;
	}
	const auto extra = std::min(default_landmark_calibration_reference_candidates, candidate_limit);
	const auto expanded = candidate_limit > candidate_universe - std::min(candidate_limit, candidate_universe)
							  ? candidate_universe
							  : candidate_limit + extra;
	const auto preferred = candidate_limit == 0 ? default_landmark_calibration_reference_candidates
												: std::max(expanded, default_landmark_calibration_reference_candidates);
	return std::min(preferred, candidate_universe);
}

template <typename Metric>
inline auto should_use_automatic_landmark_search(policy runtime_policy, const std::string &intent,
												 std::size_t query_count) noexcept -> bool
{
	return runtime_policy.is_approximate() &&
		   runtime_policy.representation_mode() == representation::automatic &&
		   (intent == "neighbors" || intent == "range" || intent == "workflow_context") &&
		   query_count > 1 && supports_landmark_search_metric<Metric>();
}

inline auto estimate_landmark_search_distance_evaluations(std::size_t record_count, std::size_t query_count,
														  policy runtime_policy) -> std::size_t
{
	const auto landmarks = landmark_index_landmark_count(record_count, runtime_policy, query_count);
	const auto queries = query_count == 0 ? std::size_t{1} : query_count;
	const auto build = detail::checked_distance_table_size_product(
		record_count, landmarks, "landmark search distance-evaluation estimate exceeds size_t capacity");
	const auto candidate_limit = landmark_index_candidate_limit(record_count, runtime_policy, false, 0, query_count);
	const auto per_query = detail::checked_distance_table_size_sum(
		landmarks, candidate_limit, "landmark search distance-evaluation estimate exceeds size_t capacity");
	const auto calibration_queries =
		landmark_calibration_sample_query_count(record_count, runtime_policy, query_count);
	const auto calibration_reference_candidates = landmark_calibration_reference_candidate_limit(
		record_count > 0 ? record_count - 1 : 0, candidate_limit);
	const auto per_calibration_query = detail::checked_distance_table_size_sum(
		detail::checked_distance_table_size_sum(
			landmarks, candidate_limit, "landmark search distance-evaluation estimate exceeds size_t capacity"),
		calibration_reference_candidates, "landmark search distance-evaluation estimate exceeds size_t capacity");
	const auto calibration = detail::checked_distance_table_size_product(
		per_calibration_query, calibration_queries,
		"landmark search distance-evaluation estimate exceeds size_t capacity");
	return detail::checked_distance_table_size_sum(
		detail::checked_distance_table_size_sum(
			build,
			detail::checked_distance_table_size_product(
				per_query, queries, "landmark search distance-evaluation estimate exceeds size_t capacity"),
			"landmark search distance-evaluation estimate exceeds size_t capacity"),
		calibration,
		"landmark search distance-evaluation estimate exceeds size_t capacity");
}

inline auto estimate_approximate_structural_distance_evaluations(std::size_t record_count) -> std::size_t
{
	const auto sample_count = record_count < 512 ? record_count : std::size_t{512};
	const auto assignment_evaluations = detail::checked_distance_table_size_product(
		record_count, sample_count, "approximate structural distance-evaluation estimate exceeds size_t capacity");
	const auto sample_evaluations = detail::checked_distance_table_size_product(
		sample_count, sample_count, "approximate structural distance-evaluation estimate exceeds size_t capacity");
	return detail::checked_distance_table_size_sum(
		assignment_evaluations, sample_evaluations,
		"approximate structural distance-evaluation estimate exceeds size_t capacity");
}

inline auto approximate_runtime_intent_supported(const std::string &intent) -> bool
{
	return intent == "compare" || intent == "neighbors" || intent == "range" ||
		   intent == "groups" || intent == "outliers" || intent == "density_filter" ||
		   intent == "compress";
}

inline auto sampled_fallback_representation_for_intent(const std::string &intent) -> std::string
{
	if (intent == "compare") {
		return "metric_space_sample";
	}
	return "sampled_metric_space";
}

inline auto chunked_workflow_intent_supported(const std::string &intent) -> bool
{
	return intent == "workflow" || intent == "workflow_context";
}

inline auto chunked_search_intent_supported(const std::string &intent) -> bool
{
	return intent == "neighbors" || intent == "range";
}

template <typename Distance>
inline auto chunk_size_for_chunked_plan(std::size_t record_count, policy runtime_policy, const resource_budget &budget)
	-> std::size_t
{
	if (record_count == 0) {
		return 1;
	}
	if (budget.max_dense_records > 0) {
		return std::min(record_count, budget.max_dense_records);
	}
	const auto options = blocked_distance_table_options_for<Distance>(runtime_policy, budget);
	return std::min(record_count, options.block_size == 0 ? std::size_t{1} : options.block_size);
}

inline auto chunked_search_candidate_chunks_per_chunk(std::size_t chunk_count, policy runtime_policy) -> std::size_t
{
	if (chunk_count < 2) {
		return 0;
	}
	const auto requested = runtime_policy.graph_neighbors() == 0 ? std::size_t{1} : runtime_policy.graph_neighbors();
	return std::min(requested, chunk_count - 1);
}

inline auto estimate_chunked_search_refinement_distance_evaluations(
	const mtrc::space::chunked_work_diagnostics &diagnostics, std::size_t candidate_chunks_per_chunk) -> std::size_t
{
	if (diagnostics.chunk_count < 2 || candidate_chunks_per_chunk == 0) {
		return 0;
	}
	const auto candidate_limit = std::min(candidate_chunks_per_chunk, diagnostics.chunk_count - 1);
	const auto chunk_pairs = detail::distance_table_dense_slot_count(diagnostics.chunk_count);
	const auto candidate_chunk_pairs_upper = std::min(
		chunk_pairs, detail::checked_distance_table_size_product(
						 diagnostics.chunk_count, candidate_limit,
						 "chunked search refinement distance-evaluation estimate exceeds size_t capacity"));
	const auto per_chunk_pair_upper = detail::checked_distance_table_size_product(
		diagnostics.max_local_record_count, diagnostics.max_local_record_count,
		"chunked search refinement distance-evaluation estimate exceeds size_t capacity");
	return detail::checked_distance_table_size_product(
		candidate_chunk_pairs_upper, per_chunk_pair_upper,
		"chunked search refinement distance-evaluation estimate exceeds size_t capacity");
}

template <typename Distance>
inline auto apply_out_of_core_plan_diagnostics(execution_plan &plan, policy runtime_policy,
											   const resource_budget &budget) -> void
{
	(void)runtime_policy;
	const auto options = blocked_distance_table_options_for<Distance>(runtime_policy, budget);
	const auto block_size = options.block_size == 0 ? std::size_t{1} : options.block_size;
	const auto record_block_count = blocked_detail::checked_block_count(plan.record_count, block_size);
	const auto block_count = detail::checked_distance_table_size_product(
		record_block_count, record_block_count, "out-of-core block plan exceeds size_t capacity");
	const auto block_cells = detail::checked_distance_table_size_product(
		block_size, block_size, "out-of-core block byte estimate exceeds size_t capacity");
	const auto block_bytes = detail::checked_distance_table_size_product(
		block_cells, sizeof(Distance), "out-of-core block byte estimate exceeds size_t capacity");
	const auto resident_blocks = std::min(options.max_cached_blocks, block_count);

	plan.out_of_core_plan = true;
	plan.spill_enabled = budget.allow_out_of_core_spill;
	plan.explicit_spill_policy_required = true;
	plan.spill_block_size = block_size;
	plan.spill_block_count = block_count;
	plan.max_resident_blocks = options.max_cached_blocks;
	plan.planned_spill_blocks = block_count > options.max_cached_blocks ? block_count - options.max_cached_blocks : 0;
	plan.spill_block_bytes_estimate = block_bytes;
	plan.max_resident_bytes_estimate = detail::checked_distance_table_size_product(
		resident_blocks, block_bytes, "out-of-core resident byte estimate exceeds size_t capacity");
	plan.spill_bytes_estimate = detail::checked_distance_table_size_product(
		plan.planned_spill_blocks, block_bytes, "out-of-core spill byte estimate exceeds size_t capacity");
	plan.memory_bounded = options.max_cached_blocks > 0 &&
						  (budget.max_memory_bytes == 0 ||
						   plan.max_resident_bytes_estimate <= budget.max_memory_bytes);
}

inline auto apply_chunked_plan_diagnostics(execution_plan &plan,
										   const mtrc::space::chunked_work_diagnostics &diagnostics,
										   bool use_bounded_work_as_estimate) -> void
{
	plan.chunked_plan = true;
	plan.chunk_size = diagnostics.chunk_size;
	plan.chunk_count = diagnostics.chunk_count;
	plan.max_local_record_count = diagnostics.max_local_record_count;
	plan.local_pair_distance_evaluations = diagnostics.local_pair_distance_evaluations;
	plan.dense_pair_distance_evaluations = diagnostics.dense_pair_distance_evaluations;
	plan.representative_count = diagnostics.representative_count;
	plan.representative_pair_distance_evaluations = diagnostics.representative_pair_distance_evaluations;
	plan.bounded_pair_distance_evaluations = diagnostics.bounded_pair_distance_evaluations;
	if (use_bounded_work_as_estimate) {
		plan.estimated_distance_evaluations = diagnostics.bounded_pair_distance_evaluations;
		plan.exactness = "exact_local_pairs";
	}
}

inline auto apply_chunked_search_plan_diagnostics(execution_plan &plan,
												 const mtrc::space::chunked_work_diagnostics &diagnostics,
												 policy runtime_policy) -> void
{
	apply_chunked_plan_diagnostics(plan, diagnostics, false);
	const auto refinement_distance_evaluations = estimate_chunked_search_refinement_distance_evaluations(
		diagnostics, chunked_search_candidate_chunks_per_chunk(diagnostics.chunk_count, runtime_policy));
	plan.bounded_pair_distance_evaluations = detail::checked_distance_table_size_sum(
		diagnostics.bounded_pair_distance_evaluations, refinement_distance_evaluations,
		"chunked search bounded distance-evaluation estimate exceeds size_t capacity");
	plan.estimated_distance_evaluations = plan.bounded_pair_distance_evaluations;
	plan.exact = false;
	plan.exactness = "non_exact_bounded_refinement";
}

inline auto apply_budget_refusal(execution_plan &plan, const std::string &reason, std::string fallback_hint) -> void
{
	plan.reason = reason;
	plan.fallback_hint = std::move(fallback_hint);
	plan.decision = budget_decision::refused;
	plan.allowed = false;
	plan.downgraded = false;
	plan.refused = true;
}

inline auto apply_budget_downgrade(execution_plan &plan, const std::string &representation,
								   const std::string &reason, std::string fallback_hint) -> void
{
	plan.representation = representation;
	plan.reason = reason;
	plan.fallback_hint = std::move(fallback_hint);
	plan.decision = budget_decision::downgraded;
	plan.allowed = true;
	plan.downgraded = true;
	plan.refused = false;
}

inline auto distance_table_options_for(policy runtime_policy,
									   distance_table_mode mode = distance_table_mode::eager,
									   runtime_guard runtime = {}) -> distance_table_options
{
	return distance_table_options{mode, runtime_policy.max_dense_records(), runtime_policy.max_memory_bytes(),
								  runtime};
}

template <typename Space>
inline auto make_distance_table(const Space &space, policy runtime_policy,
								distance_table_mode mode = distance_table_mode::eager,
								runtime_guard runtime = {}) -> DistanceTable<Space>
{
	return DistanceTable<Space>(space, distance_table_options_for(runtime_policy, mode, runtime));
}

template <typename Space>
inline auto make_blocked_distance_table(const Space &space, policy runtime_policy,
										runtime_guard runtime = {}) -> BlockedDistanceTable<Space>
{
	return BlockedDistanceTable<Space>(
		space, blocked_distance_table_options_for<typename Space::distance_type>(runtime_policy, runtime));
}

inline auto uses_blocked_exact_fallback(const execution_plan &plan) -> bool
{
	return plan.allowed && plan.downgraded && plan.exact &&
		   (plan.representation == "chunked_distance_table" || plan.representation == "blocked_distance_table");
}

inline auto materialized_operator_representation(const execution_plan &plan) -> std::string
{
	return uses_blocked_exact_fallback(plan) ? "blocked_distance_table" : plan.representation;
}

template <typename Space>
inline auto require_materialized_operator_plan(const Space &space, policy runtime_policy,
											   const char *intent) -> execution_plan
{
	auto plan = estimate_cost(space, intent, runtime_policy);
	if (!plan.refused && runtime_policy.uses_materialization() &&
		!uses_distance_table_materialization(runtime_policy)) {
		plan = estimate_cost(space, intent, using_distance_table(runtime_policy));
	}
	if (plan.refused) {
		auto message = plan.reason.empty() ? std::string("runtime policy refused by resource budget") : plan.reason;
		if (!plan.fallback_hint.empty()) {
			message += "; fallback: " + plan.fallback_hint;
		}
		throw RepresentationError(message);
	}
	if (plan.downgraded && !uses_blocked_exact_fallback(plan)) {
		auto message = plan.reason.empty() ? std::string("runtime policy downgraded by resource budget") : plan.reason;
		if (!plan.fallback_hint.empty()) {
			message += "; fallback: " + plan.fallback_hint;
		}
		message += "; executor integration for this downgraded route is not implemented";
		throw RepresentationError(message);
	}
	return plan;
}

template <typename Space, typename Operation>
inline auto with_materialized_distance_provider(const Space &space, policy runtime_policy, const char *intent,
												Operation &&operation)
	-> decltype(operation(std::declval<const DistanceTable<Space> &>(), std::declval<const execution_plan &>()))
{
	const auto plan = require_materialized_operator_plan(space, runtime_policy, intent);
	auto runtime = runtime_guard(runtime_policy);
	if (uses_blocked_exact_fallback(plan)) {
		auto provider = make_blocked_distance_table(space, runtime_policy, runtime);
		return operation(provider, plan);
	}

	auto provider = make_distance_table(space, runtime_policy, distance_table_mode::eager, runtime);
	return operation(provider, plan);
}

inline auto diagnostics(policy runtime_policy = exact(), std::string representation = {}, std::string intent = {})
	-> RuntimeDiagnostics
{
	const auto supported = runtime_policy.is_exact() ||
						   (runtime_policy.is_approximate() && approximate_runtime_intent_supported(intent));
	RuntimeDiagnostics result;
	result.policy_name = runtime_policy.name();
	result.exact = runtime_policy.is_exact();
	result.parallel = runtime_policy.uses_parallel_execution();
	result.materialized = runtime_policy.uses_materialization();
	result.representation =
		representation.empty() ? execution_representation(runtime_policy) : std::move(representation);
	result.intent = std::move(intent);
	result.supported = supported;
	result.max_dense_records = runtime_policy.max_dense_records();
	result.max_memory_bytes = runtime_policy.max_memory_bytes();
	result.max_distance_evaluations = runtime_policy.max_distance_evaluations();
	result.max_runtime_ms = runtime_policy.max_runtime_ms();
	result.max_spill_bytes = runtime_policy.max_spill_bytes();
	result.allow_approximate = runtime_policy.allows_approximate_fallback();
	result.allow_chunking = runtime_policy.allows_chunking_fallback();
	result.allow_out_of_core_spill = runtime_policy.allows_out_of_core_spill();
	result.budget = budget_from_policy(runtime_policy);
	if (!supported) {
		result.reason = "approximate runtime policies are not implemented for promoted C++ execution yet";
		result.decision = budget_decision::refused;
		result.allowed = false;
		result.refused = true;
	}
	return result;
}

template <typename Metric>
inline auto diagnostics_for_metric(policy runtime_policy = exact(), std::string representation = {},
								   std::string intent = {}) -> RuntimeDiagnostics
{
	auto result = diagnostics(runtime_policy, std::move(representation), std::move(intent));
	if (result.supported && !supports_parallel_metric<Metric>(runtime_policy)) {
		result.supported = false;
		result.reason = "parallel runtime policy requires a thread-safe metric";
	}
	return result;
}

template <typename Space>
inline auto estimate_cost(const Space &space, std::string intent, policy runtime_policy,
						  resource_budget budget, std::size_t query_count = 0) -> execution_plan
{
	execution_plan plan;
	plan.policy_name = runtime_policy.name();
	plan.intent = std::move(intent);
	plan.record_count = space.size();
	plan.query_count = query_count;
	plan.exact = runtime_policy.is_exact();
	plan.exactness = plan.exact ? "exact" : "approximate";
	plan.requested_representation = execution_representation(runtime_policy);
	plan.representation = plan.requested_representation;
	plan.budget = budget;
	plan.max_runtime_ms = budget.max_runtime_ms;
	plan.max_spill_bytes = budget.max_spill_bytes;
	const auto apply_chunked_plan = [&](const std::string &representation, bool use_bounded_work_as_estimate) {
		const auto diagnostics = mtrc::space::chunked_plan_diagnostics_for_count(
			plan.record_count,
			chunk_size_for_chunked_plan<typename Space::distance_type>(plan.record_count, runtime_policy, budget),
			representation);
		apply_chunked_plan_diagnostics(plan, diagnostics, use_bounded_work_as_estimate);
	};
	const auto apply_chunked_search_plan = [&] {
		const auto representation = std::string("chunked_space_view");
		const auto diagnostics = mtrc::space::chunked_plan_diagnostics_for_count(
			plan.record_count,
			chunk_size_for_chunked_plan<typename Space::distance_type>(plan.record_count, runtime_policy, budget),
			representation);
		apply_chunked_search_plan_diagnostics(plan, diagnostics, runtime_policy);
	};

	if (!runtime_policy.is_exact()) {
		if (plan.intent == "compare") {
			plan.representation = "metric_space_sample";
			plan.estimated_distance_evaluations =
				estimate_approximate_compare_distance_evaluations(plan.record_count);
		} else if (plan.intent == "neighbors" || plan.intent == "range") {
			const auto explicit_landmark =
				runtime_policy.representation_mode() == representation::landmark_index;
			const auto automatic_landmark =
				should_use_automatic_landmark_search<typename Space::metric_type>(
					runtime_policy, plan.intent, plan.query_count);
			const auto landmark_estimate = (explicit_landmark || automatic_landmark)
											   ? estimate_landmark_search_distance_evaluations(
													 plan.record_count, plan.query_count, runtime_policy)
											   : std::size_t{};
			const auto sampled_estimate =
				estimate_approximate_search_distance_evaluations(plan.record_count, plan.query_count);
			if (explicit_landmark || (automatic_landmark && landmark_estimate <= sampled_estimate)) {
				plan.representation = "landmark_index";
				plan.estimated_distance_evaluations = landmark_estimate;
				plan.memory_bytes_estimate = detail::checked_distance_table_size_product(
					plan.record_count,
					detail::checked_distance_table_size_product(
						landmark_index_landmark_count(plan.record_count, runtime_policy, plan.query_count),
						sizeof(typename Space::distance_type),
						"landmark search memory estimate exceeds size_t capacity"),
					"landmark search memory estimate exceeds size_t capacity");
			} else {
				plan.representation = "sampled_metric_space";
				plan.estimated_distance_evaluations = sampled_estimate;
			}
		} else if (plan.intent == "groups" || plan.intent == "outliers" || plan.intent == "density_filter" ||
				   plan.intent == "compress") {
			plan.representation = "sampled_metric_space";
			plan.estimated_distance_evaluations =
				estimate_approximate_structural_distance_evaluations(plan.record_count);
		} else {
			apply_budget_refusal(plan, "approximate runtime policies are not implemented for promoted C++ execution yet",
								 "use space::storage::exact() or implement an approximate provider for this intent");
			return plan;
		}
	}
	if (!supports_parallel_metric<typename Space::metric_type>(runtime_policy)) {
		apply_budget_refusal(plan, "parallel runtime policy requires a thread-safe metric",
							 "use space::storage::serial() or mark the metric thread-safe when it is safe");
		return plan;
	}
	if (runtime_policy.is_exact() && uses_distance_table_materialization(runtime_policy)) {
		plan.dense_distance_slots = detail::distance_table_dense_slot_count(plan.record_count);
		plan.estimated_distance_evaluations =
			plan.intent == "compare" ? estimate_live_distance_evaluations(plan.record_count, plan.intent)
									 : plan.dense_distance_slots;
		plan.memory_bytes_estimate =
			estimate_distance_table_memory_bytes<typename Space::distance_type>(plan.record_count);
		if (plan.intent == "compare") {
			plan.memory_bytes_estimate = detail::checked_distance_table_size_product(
				plan.memory_bytes_estimate, 2, "compare memory estimate exceeds size_t capacity");
		}
		if (budget.max_dense_records > 0 && plan.record_count > budget.max_dense_records) {
			plan.budget_exceeded = true;
			const auto reason = "distance table materialization exceeds max_dense_records: records=" +
								std::to_string(plan.record_count) +
								" max_dense_records=" + std::to_string(budget.max_dense_records);
			if (budget.allow_chunking && budget.allow_approximate && chunked_search_intent_supported(plan.intent)) {
				apply_budget_downgrade(plan, "chunked_space_view", reason,
									   "use chunked search refinement, a sampled representation, or raise the resource_budget");
				apply_chunked_search_plan();
			} else if (budget.allow_chunking) {
				const auto workflow_plan = chunked_workflow_intent_supported(plan.intent);
				const auto representation = workflow_plan ? "chunked_workflow_plan" : "chunked_distance_table";
				apply_budget_downgrade(plan, representation, reason, fallback_hint_for_distance_budget(budget));
				apply_chunked_plan(representation, workflow_plan);
				if (budget.allow_out_of_core_spill) {
					apply_out_of_core_plan_diagnostics<typename Space::distance_type>(plan, runtime_policy, budget);
				}
			} else if (budget.allow_approximate) {
				plan.exact = false;
				plan.exactness = "approximate";
				apply_budget_downgrade(plan, sampled_fallback_representation_for_intent(plan.intent), reason,
									   fallback_hint_for_distance_budget(budget));
			} else {
				apply_budget_refusal(plan, reason, fallback_hint_for_distance_budget(budget));
			}
		} else if (budget.max_memory_bytes > 0 && plan.memory_bytes_estimate > budget.max_memory_bytes) {
			plan.budget_exceeded = true;
			const auto reason = "distance table materialization exceeds max_memory_bytes: records=" +
								std::to_string(plan.record_count) +
								" estimated_bytes=" + std::to_string(plan.memory_bytes_estimate) +
								" max_memory_bytes=" + std::to_string(budget.max_memory_bytes);
			if (budget.allow_chunking && budget.allow_approximate && chunked_search_intent_supported(plan.intent)) {
				apply_budget_downgrade(plan, "chunked_space_view", reason,
									   "use chunked search refinement, a sampled representation, or raise the resource_budget");
				apply_chunked_search_plan();
			} else if (budget.allow_chunking) {
				const auto workflow_plan = chunked_workflow_intent_supported(plan.intent);
				const auto representation = workflow_plan ? "chunked_workflow_plan" : "chunked_distance_table";
				apply_budget_downgrade(plan, representation, reason, fallback_hint_for_distance_budget(budget));
				apply_chunked_plan(representation, workflow_plan);
				if (budget.allow_out_of_core_spill) {
					apply_out_of_core_plan_diagnostics<typename Space::distance_type>(plan, runtime_policy, budget);
				}
			} else if (budget.allow_approximate) {
				plan.exact = false;
				plan.exactness = "approximate";
				apply_budget_downgrade(plan, sampled_fallback_representation_for_intent(plan.intent), reason,
									   fallback_hint_for_distance_budget(budget));
			} else {
				apply_budget_refusal(plan, reason, fallback_hint_for_distance_budget(budget));
			}
		}
	} else if (runtime_policy.is_exact() && runtime_policy.representation_mode() == representation::knn_graph) {
		plan.estimated_distance_evaluations =
			estimate_knn_graph_distance_evaluations(plan.record_count, plan.intent, plan.query_count);
		plan.memory_bytes_estimate = estimate_knn_graph_memory_bytes<typename Space::distance_type>(
			plan.record_count, runtime_policy.graph_neighbors());
	} else if (runtime_policy.is_exact()) {
		plan.estimated_distance_evaluations =
			estimate_live_distance_evaluations(plan.record_count, plan.intent, plan.query_count);
	}

	if (plan.allowed && budget.max_distance_evaluations > 0 &&
		plan.estimated_distance_evaluations > budget.max_distance_evaluations) {
		plan.budget_exceeded = true;
		const auto reason = "execution exceeds max_distance_evaluations: estimated_distance_evaluations=" +
							std::to_string(plan.estimated_distance_evaluations) +
							" max_distance_evaluations=" + std::to_string(budget.max_distance_evaluations);
		if (budget.allow_chunking && budget.allow_approximate && chunked_search_intent_supported(plan.intent)) {
			const auto representation = std::string("chunked_space_view");
			const auto diagnostics = mtrc::space::chunked_plan_diagnostics_for_count(
				plan.record_count,
				chunk_size_for_chunked_plan<typename Space::distance_type>(
					plan.record_count, runtime_policy, budget),
				representation);
			apply_chunked_search_plan_diagnostics(plan, diagnostics, runtime_policy);
			if (plan.estimated_distance_evaluations <= budget.max_distance_evaluations) {
				apply_budget_downgrade(
					plan, representation, reason,
					"use chunked search refinement to bound candidate work, or raise the resource_budget");
			} else {
				plan.exact = false;
				plan.exactness = "approximate";
				apply_budget_downgrade(plan, sampled_fallback_representation_for_intent(plan.intent), reason,
									   "use an approximate or sampled representation, or raise the resource_budget");
			}
		} else if (budget.allow_chunking && chunked_workflow_intent_supported(plan.intent)) {
			const auto representation = std::string("chunked_workflow_plan");
			const auto diagnostics = mtrc::space::chunked_plan_diagnostics_for_count(
				plan.record_count,
				chunk_size_for_chunked_plan<typename Space::distance_type>(
					plan.record_count, runtime_policy, budget),
				representation);
			apply_chunked_plan_diagnostics(plan, diagnostics, false);
			if (diagnostics.bounded_pair_distance_evaluations <= budget.max_distance_evaluations) {
				apply_budget_downgrade(
					plan, representation, reason,
					"use chunked workflow planning to bound local pair work, or raise the resource_budget");
				apply_chunked_plan_diagnostics(plan, diagnostics, true);
				if (budget.allow_out_of_core_spill) {
					apply_out_of_core_plan_diagnostics<typename Space::distance_type>(plan, runtime_policy, budget);
				}
			} else if (budget.allow_approximate) {
				plan.exact = false;
				plan.exactness = "approximate";
				apply_budget_downgrade(plan, sampled_fallback_representation_for_intent(plan.intent), reason,
									   "use an approximate or sampled representation, or raise the resource_budget");
			} else {
				apply_budget_refusal(
					plan,
					reason + "; chunked bounded_pair_distance_evaluations=" +
						std::to_string(diagnostics.bounded_pair_distance_evaluations),
					"use a larger chunk/evaluation budget, an approximate representation, or raise the resource_budget");
			}
		} else if (budget.allow_approximate) {
			plan.exact = false;
			plan.exactness = "approximate";
			apply_budget_downgrade(plan, sampled_fallback_representation_for_intent(plan.intent), reason,
								   "use an approximate or sampled representation, or raise the resource_budget");
		} else {
			apply_budget_refusal(plan, reason,
								 "use a smaller query batch, an approximate representation, or raise the resource_budget");
		}
	}

	return plan;
}

template <typename Space>
inline auto estimate_cost(const Space &space, std::string intent, policy runtime_policy) -> execution_plan
{
	return estimate_cost(space, std::move(intent), runtime_policy, budget_from_policy(runtime_policy));
}

template <typename Space>
inline auto estimate_cost(const Space &space, std::string intent, std::size_t query_count,
						  policy runtime_policy = exact()) -> execution_plan
{
	return estimate_cost(space, std::move(intent), runtime_policy, budget_from_policy(runtime_policy), query_count);
}

template <typename Space>
inline auto estimate_cost(const Space &space, std::string intent, policy runtime_policy,
						  std::size_t query_count) -> execution_plan
{
	return estimate_cost(space, std::move(intent), runtime_policy, budget_from_policy(runtime_policy), query_count);
}

template <typename Space>
inline auto diagnostics_for_space(const Space &space, policy runtime_policy = exact(), std::string representation = {},
								  std::string intent = {}, std::size_t query_count = 0) -> RuntimeDiagnostics
{
	auto result = diagnostics_for_metric<typename Space::metric_type>(runtime_policy, representation, intent);
	const auto plan = estimate_cost(space, std::move(intent), runtime_policy, budget_from_policy(runtime_policy),
									query_count);
	result.record_count = plan.record_count;
	result.dense_distance_slots = plan.dense_distance_slots;
	result.estimated_distance_evaluations = plan.estimated_distance_evaluations;
	result.chunked_plan = plan.chunked_plan;
	result.chunk_size = plan.chunk_size;
	result.chunk_count = plan.chunk_count;
	result.max_local_record_count = plan.max_local_record_count;
	result.local_pair_distance_evaluations = plan.local_pair_distance_evaluations;
	result.dense_pair_distance_evaluations = plan.dense_pair_distance_evaluations;
	result.representative_count = plan.representative_count;
	result.representative_pair_distance_evaluations = plan.representative_pair_distance_evaluations;
	result.bounded_pair_distance_evaluations = plan.bounded_pair_distance_evaluations;
	result.out_of_core_plan = plan.out_of_core_plan;
	result.spill_enabled = plan.spill_enabled;
	result.explicit_spill_policy_required = plan.explicit_spill_policy_required;
	result.memory_bounded = plan.memory_bounded;
	result.spill_block_size = plan.spill_block_size;
	result.spill_block_count = plan.spill_block_count;
	result.planned_spill_blocks = plan.planned_spill_blocks;
	result.max_resident_blocks = plan.max_resident_blocks;
	result.spill_block_bytes_estimate = plan.spill_block_bytes_estimate;
	result.max_resident_bytes_estimate = plan.max_resident_bytes_estimate;
	result.spill_bytes_estimate = plan.spill_bytes_estimate;
	result.memory_bytes_estimate = plan.memory_bytes_estimate;
	result.exact = plan.exact;
	result.max_spill_bytes = plan.max_spill_bytes;
	result.budget = plan.budget;
	result.decision = plan.decision;
	result.allowed = plan.allowed;
	result.downgraded = plan.downgraded;
	result.refused = plan.refused;
	result.fallback_hint = plan.fallback_hint;
	result.budget_exceeded = plan.budget_exceeded;
	if (representation.empty()) {
		result.representation = plan.representation;
	}
	if (!plan.allowed && result.supported) {
		result.supported = false;
		result.reason = plan.reason;
	}
	if (plan.allowed && result.supported && result.reason.empty()) {
		result.reason = plan.reason;
	}
	return result;
}

} // namespace mtrc::space::storage

#endif
