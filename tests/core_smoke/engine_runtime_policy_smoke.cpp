#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"

struct StringLengthDistance {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> int
	{
		const auto difference = static_cast<int>(lhs.size()) - static_cast<int>(rhs.size());
		return difference < 0 ? -difference : difference;
	}
};

static auto nearly_equal(double lhs, double rhs, double epsilon = 1e-12) -> bool
{
	return std::abs(lhs - rhs) <= epsilon;
}

struct DoubleAbsoluteDistance {
	auto operator()(double lhs, double rhs) const -> double
	{
		const auto difference = lhs - rhs;
		return difference < 0.0 ? -difference : difference;
	}
};

struct CountingDoubleAbsoluteDistance {
	explicit CountingDoubleAbsoluteDistance(std::shared_ptr<std::size_t> calls)
		: calls(calls)
	{
	}

	auto operator()(double lhs, double rhs) const -> double
	{
		++(*calls);
		const auto difference = lhs - rhs;
		return difference < 0.0 ? -difference : difference;
	}

	std::shared_ptr<std::size_t> calls;
};

struct StatefulLengthDistance {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> int
	{
		const auto difference = static_cast<int>(lhs.size()) - static_cast<int>(rhs.size());
		return difference < 0 ? -difference : difference;
	}
};

struct CountingLengthDistance {
	explicit CountingLengthDistance(std::shared_ptr<std::size_t> calls)
		: calls(calls)
	{
	}

	auto operator()(const std::string &lhs, const std::string &rhs) const -> int
	{
		++(*calls);
		const auto difference = static_cast<int>(lhs.size()) - static_cast<int>(rhs.size());
		return difference < 0 ? -difference : difference;
	}

	std::shared_ptr<std::size_t> calls;
};

struct SlowCountingLengthDistance {
	explicit SlowCountingLengthDistance(std::shared_ptr<std::size_t> calls)
		: calls(calls)
	{
	}

	auto operator()(const std::string &lhs, const std::string &rhs) const -> int
	{
		++(*calls);
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
		const auto difference = static_cast<int>(lhs.size()) - static_cast<int>(rhs.size());
		return difference < 0 ? -difference : difference;
	}

	std::shared_ptr<std::size_t> calls;
};

namespace mtrc::core {

template <> struct metric_traits<StatefulLengthDistance> {
	static constexpr auto law = metric_law::distance;
	static constexpr auto records = record_kind::string;
	static constexpr bool thread_safe = false;
};

template <> struct metric_traits<DoubleAbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

template <> struct metric_traits<CountingDoubleAbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

template <> struct metric_traits<SlowCountingLengthDistance> {
	static constexpr auto law = metric_law::pseudo_metric;
	static constexpr auto records = record_kind::string;
	static constexpr bool thread_safe = true;
};

} // namespace mtrc::core

int main()
{
	auto space = mtrc::make_space(std::vector<std::string>{"a", "bb", "ccc", "dddd"}, StringLengthDistance{});

	const auto lazy_policy = mtrc::space::storage::exact();
	assert(lazy_policy.name() == "exact_lazy_serial");
	assert(lazy_policy.representation_preference() == "implicit");
	const auto lazy_diagnostics = mtrc::space::storage::diagnostics(lazy_policy, {}, "neighbors");
	assert(lazy_diagnostics.policy_name == "exact_lazy_serial");
	assert(lazy_diagnostics.exact);
	assert(!lazy_diagnostics.parallel);
	assert(!lazy_diagnostics.materialized);
	assert(lazy_diagnostics.representation == "metric_space");
	assert(lazy_diagnostics.intent == "neighbors");
	assert(lazy_diagnostics.supported);
	assert(lazy_diagnostics.reason.empty());

	const auto lazy_neighbors = mtrc::find_neighbors(space, space.id(0), 2, lazy_policy);
	assert(lazy_neighbors.exact);
	assert(lazy_neighbors.representation == "metric_space");
	assert(lazy_neighbors.size() == 2);
	assert(lazy_neighbors[0].id == space.id(1));

	const auto explicit_implicit_policy = mtrc::space::storage::using_implicit();
	assert(explicit_implicit_policy.name() == "exact_lazy_serial");
	assert(explicit_implicit_policy.representation_preference() == "implicit");
	const auto explicit_implicit_diagnostics = mtrc::space::storage::diagnostics(explicit_implicit_policy, {}, "neighbors");
	assert(explicit_implicit_diagnostics.representation == "implicit");
	const auto explicit_implicit_neighbors =
		mtrc::find_neighbors(space, std::string("ee"), 2, explicit_implicit_policy);
	assert(explicit_implicit_neighbors.representation == "implicit");
	assert(explicit_implicit_neighbors[0].id == space.id(1));

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	assert(materialized_policy.name() == "exact_materialized_serial");
	assert(materialized_policy.representation_preference() == "distance_table");
	const auto materialized_diagnostics = mtrc::space::storage::diagnostics(materialized_policy, {}, "neighbors");
	assert(materialized_diagnostics.policy_name == "exact_materialized_serial");
	assert(materialized_diagnostics.materialized);
	assert(materialized_diagnostics.representation == "distance_table");

	const auto materialized_neighbors = mtrc::find_neighbors(space, space.id(0), 2, materialized_policy);
	assert(materialized_neighbors.exact);
	assert(materialized_neighbors.representation == "distance_table");
	assert(materialized_neighbors.size() == lazy_neighbors.size());
	assert(materialized_neighbors[0].id == lazy_neighbors[0].id);
	assert(materialized_neighbors[0].distance == lazy_neighbors[0].distance);

	const auto counted_materialized_neighbors =
		mtrc::find_neighbors(space, space.id(0), mtrc::count{2}, materialized_policy);
	assert(counted_materialized_neighbors.representation == "distance_table");
	assert(counted_materialized_neighbors[0].id == materialized_neighbors[0].id);

	const auto matrix_policy = mtrc::space::storage::using_distance_table();
	assert(matrix_policy.name() == "exact_materialized_serial");
	assert(matrix_policy.representation_preference() == "distance_table");
	const auto matrix_policy_neighbors = mtrc::find_neighbors(space, space.id(0), 2, matrix_policy);
	assert(matrix_policy_neighbors.representation == "distance_table");
	assert(matrix_policy_neighbors[0].id == materialized_neighbors[0].id);

	const auto matrix_memory_bytes = mtrc::space::storage::estimate_distance_table_memory_bytes<int>(space.size());
	const auto budgeted_matrix_policy =
		mtrc::space::storage::with_distance_table_budget(matrix_policy, space.size(), matrix_memory_bytes);
	assert(budgeted_matrix_policy.max_dense_records() == space.size());
	assert(budgeted_matrix_policy.max_memory_bytes() == matrix_memory_bytes);
	assert(budgeted_matrix_policy.max_distance_evaluations() == 0);
	assert(!budgeted_matrix_policy.allows_approximate_fallback());
	assert(!budgeted_matrix_policy.allows_chunking_fallback());
	assert(mtrc::space::storage::parallel(budgeted_matrix_policy).max_memory_bytes() == matrix_memory_bytes);
	assert(mtrc::space::storage::materialized(budgeted_matrix_policy).max_dense_records() == space.size());
	const auto budget_only_diagnostics =
		mtrc::space::storage::diagnostics(budgeted_matrix_policy, {}, "neighbors");
	assert(budget_only_diagnostics.max_dense_records == space.size());
	assert(budget_only_diagnostics.max_memory_bytes == matrix_memory_bytes);
	assert(budget_only_diagnostics.max_distance_evaluations == 0);
	assert(!budget_only_diagnostics.allow_approximate);
	assert(!budget_only_diagnostics.allow_chunking);
	assert(budget_only_diagnostics.memory_bytes_estimate == 0);
	assert(budget_only_diagnostics.decision == mtrc::space::storage::budget_decision::allowed);
	const auto budgeted_matrix_diagnostics =
		mtrc::space::storage::diagnostics_for_space(space, budgeted_matrix_policy, {}, "neighbors");
	assert(budgeted_matrix_diagnostics.max_dense_records == space.size());
	assert(budgeted_matrix_diagnostics.max_memory_bytes == matrix_memory_bytes);
	assert(budgeted_matrix_diagnostics.record_count == space.size());
	assert(budgeted_matrix_diagnostics.dense_distance_slots == space.size() * space.size());
	assert(budgeted_matrix_diagnostics.estimated_distance_evaluations == space.size() * space.size());
	assert(budgeted_matrix_diagnostics.memory_bytes_estimate == matrix_memory_bytes);
	assert(!budgeted_matrix_diagnostics.budget_exceeded);
	assert(budgeted_matrix_diagnostics.allowed);
	assert(!budgeted_matrix_diagnostics.downgraded);
	assert(!budgeted_matrix_diagnostics.refused);
	assert(budgeted_matrix_diagnostics.decision == mtrc::space::storage::budget_decision::allowed);
	assert(budgeted_matrix_diagnostics.supported);
	const auto three_query_plan = mtrc::space::storage::estimate_cost(space, "neighbors", 3, lazy_policy);
	assert(three_query_plan.query_count == 3);
	assert(three_query_plan.estimated_distance_evaluations == space.size() * 3);
	const auto three_query_diagnostics =
		mtrc::space::storage::diagnostics_for_space(space, lazy_policy, {}, "neighbors", 3);
	assert(three_query_diagnostics.estimated_distance_evaluations == space.size() * 3);
	const auto budgeted_matrix_plan =
		mtrc::space::storage::estimate_cost(space, "neighbors", budgeted_matrix_policy);
	assert(budgeted_matrix_plan.intent == "neighbors");
	assert(budgeted_matrix_plan.record_count == space.size());
	assert(budgeted_matrix_plan.estimated_distance_evaluations == space.size() * space.size());
	assert(budgeted_matrix_plan.memory_bytes_estimate == matrix_memory_bytes);
	assert(budgeted_matrix_plan.exact);
	assert(budgeted_matrix_plan.exactness == "exact");
	assert(budgeted_matrix_plan.requested_representation == "distance_table");
	assert(budgeted_matrix_plan.representation == "distance_table");
	assert(budgeted_matrix_plan.allowed);
	assert(!budgeted_matrix_plan.downgraded);
	assert(!budgeted_matrix_plan.refused);
	assert(mtrc::space::storage::budget_decision_name(budgeted_matrix_plan.decision) == "allowed");
	const auto budgeted_matrix_neighbors = mtrc::find_neighbors(space, space.id(0), 2, budgeted_matrix_policy);
	assert(budgeted_matrix_neighbors.representation == "distance_table");
	assert(budgeted_matrix_neighbors[0].id == materialized_neighbors[0].id);

	const auto dense_limited_policy =
		mtrc::space::storage::with_distance_table_budget(matrix_policy, space.size() - 1, 0);
	const auto dense_limited_diagnostics =
		mtrc::space::storage::diagnostics_for_space(space, dense_limited_policy, {}, "neighbors");
	assert(dense_limited_diagnostics.budget_exceeded);
	assert(!dense_limited_diagnostics.supported);
	assert(!dense_limited_diagnostics.allowed);
	assert(dense_limited_diagnostics.refused);
	assert(dense_limited_diagnostics.decision == mtrc::space::storage::budget_decision::refused);
	assert(dense_limited_diagnostics.reason.find("max_dense_records") != std::string::npos);
	assert(dense_limited_diagnostics.fallback_hint.find("using_implicit") != std::string::npos);
	bool rejected_dense_policy_budget = false;
	try {
		(void)mtrc::find_neighbors(space, space.id(0), 2, dense_limited_policy);
	} catch (const mtrc::RepresentationError &) {
		rejected_dense_policy_budget = true;
	}
	assert(rejected_dense_policy_budget);

	const auto memory_limited_policy = mtrc::space::storage::with_distance_table_budget(
		mtrc::space::storage::materialized(), space.size(), matrix_memory_bytes - 1);
	const auto memory_limited_diagnostics =
		mtrc::space::storage::diagnostics_for_space(space, memory_limited_policy, {}, "groups");
	assert(memory_limited_diagnostics.budget_exceeded);
	assert(!memory_limited_diagnostics.supported);
	assert(!memory_limited_diagnostics.allowed);
	assert(memory_limited_diagnostics.refused);
	assert(memory_limited_diagnostics.decision == mtrc::space::storage::budget_decision::refused);
	assert(memory_limited_diagnostics.reason.find("max_memory_bytes") != std::string::npos);
	assert(memory_limited_diagnostics.fallback_hint.find("using_implicit") != std::string::npos);
	bool rejected_memory_policy_budget = false;
	try {
		(void)mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2),
								memory_limited_policy);
	} catch (const mtrc::RepresentationError &) {
		rejected_memory_policy_budget = true;
	}
	assert(rejected_memory_policy_budget);

	auto materialized_refusal_calls = std::make_shared<std::size_t>(0);
	auto counting_space = mtrc::make_space(
		std::vector<std::string>{"a", "bb", "ccc", "dddd"}, CountingLengthDistance(materialized_refusal_calls));
	const auto counting_matrix_memory_bytes =
		mtrc::space::storage::estimate_distance_table_memory_bytes<int>(counting_space.size());
	const auto refusing_materialized_policy = mtrc::space::storage::with_distance_table_budget(
		mtrc::space::storage::materialized(), counting_space.size(), counting_matrix_memory_bytes - 1);
	auto assert_refused_before_operator_work = [&](auto operation) {
		*materialized_refusal_calls = 0;
		bool rejected = false;
		try {
			operation();
		} catch (const mtrc::RepresentationError &error) {
			rejected = true;
			const std::string message = error.what();
			assert(message.find("max_memory_bytes") != std::string::npos);
			assert(message.find("fallback") != std::string::npos);
			assert(message.find("using_implicit") != std::string::npos);
		}
		assert(rejected);
		assert(*materialized_refusal_calls == 0);
	};
	assert_refused_before_operator_work([&] {
		(void)mtrc::find_groups(counting_space, mtrc::stats::structural_analysis::k_medoids_options(2),
								refusing_materialized_policy);
	});
	assert_refused_before_operator_work([&] {
		(void)mtrc::find_outliers(counting_space, mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
								  refusing_materialized_policy);
	});
	assert_refused_before_operator_work([&] {
		(void)mtrc::density_filter(counting_space, mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
							refusing_materialized_policy);
	});
	assert_refused_before_operator_work([&] {
		(void)mtrc::compress(counting_space, 2, refusing_materialized_policy);
	});

	const auto describe_limited_policy =
		mtrc::space::storage::with_distance_evaluation_budget(mtrc::space::storage::exact(), 1);
	const auto describe_limited_plan =
		mtrc::space::storage::estimate_cost(counting_space, "describe", describe_limited_policy);
	assert(describe_limited_plan.refused);
	assert(describe_limited_plan.reason.find("max_distance_evaluations") != std::string::npos);
	*materialized_refusal_calls = 0;
	bool rejected_live_describe_budget = false;
	try {
		(void)mtrc::describe_structure(counting_space, describe_limited_policy);
	} catch (const mtrc::RepresentationError &error) {
		rejected_live_describe_budget = true;
		const std::string message = error.what();
		assert(message.find("describe_structure") != std::string::npos);
		assert(message.find("distance_evaluation_budget") != std::string::npos);
	}
	assert(rejected_live_describe_budget);
	assert(*materialized_refusal_calls == 0);

	const auto blocked_materialized_policy = mtrc::space::storage::allow_chunking_fallback(
		mtrc::space::storage::with_distance_table_budget(
			mtrc::space::storage::materialized(), 1, counting_matrix_memory_bytes - 1));
	const auto blocked_operator_plan =
		mtrc::space::storage::estimate_cost(counting_space, "groups", blocked_materialized_policy);
	assert(blocked_operator_plan.budget_exceeded);
	assert(blocked_operator_plan.allowed);
	assert(blocked_operator_plan.downgraded);
	assert(!blocked_operator_plan.refused);
	assert(blocked_operator_plan.exact);
	assert(mtrc::space::storage::uses_blocked_exact_fallback(blocked_operator_plan));
	assert(mtrc::space::storage::materialized_operator_representation(blocked_operator_plan) ==
		   "blocked_distance_table");
	const auto blocked_neighbor_plan =
		mtrc::space::storage::estimate_cost(counting_space, "neighbors", blocked_materialized_policy);
	assert(blocked_neighbor_plan.budget_exceeded);
	assert(blocked_neighbor_plan.allowed);
	assert(blocked_neighbor_plan.downgraded);
	assert(!blocked_neighbor_plan.refused);
	assert(blocked_neighbor_plan.exact);
	assert(mtrc::space::storage::uses_blocked_exact_fallback(blocked_neighbor_plan));
	assert(mtrc::space::storage::materialized_operator_representation(blocked_neighbor_plan) ==
		   "blocked_distance_table");
	const auto blocked_range_plan =
		mtrc::space::storage::estimate_cost(counting_space, "range", blocked_materialized_policy);
	assert(blocked_range_plan.budget_exceeded);
	assert(blocked_range_plan.allowed);
	assert(blocked_range_plan.downgraded);
	assert(!blocked_range_plan.refused);
	assert(blocked_range_plan.exact);
	assert(mtrc::space::storage::uses_blocked_exact_fallback(blocked_range_plan));
	assert(mtrc::space::storage::materialized_operator_representation(blocked_range_plan) ==
		   "blocked_distance_table");

	*materialized_refusal_calls = 0;
	const auto blocked_probe =
		mtrc::space::storage::make_blocked_distance_table(counting_space, blocked_materialized_policy);
	assert(*materialized_refusal_calls == 0);
	assert(blocked_probe.dense_distance_slots() == 0);
	assert(blocked_probe.cached_distances() == 0);

	const auto spill_materialized_policy =
		mtrc::space::storage::allow_out_of_core_spill(blocked_materialized_policy);
	*materialized_refusal_calls = 0;
	const auto spill_plan = mtrc::space::storage::estimate_cost(counting_space, "neighbors", spill_materialized_policy);
	assert(*materialized_refusal_calls == 0);
	assert(spill_plan.allowed);
	assert(spill_plan.budget.allow_out_of_core_spill);
	assert(spill_plan.out_of_core_plan);
	assert(spill_plan.spill_enabled);
	assert(spill_plan.explicit_spill_policy_required);
	assert(spill_plan.memory_bounded);
	assert(spill_plan.spill_block_size == 1);
	assert(spill_plan.spill_block_count == counting_space.size() * counting_space.size());
	assert(spill_plan.max_resident_blocks == 4);
	assert(spill_plan.planned_spill_blocks == spill_plan.spill_block_count - spill_plan.max_resident_blocks);
	assert(spill_plan.spill_block_bytes_estimate == sizeof(int));
	assert(spill_plan.max_resident_bytes_estimate == spill_plan.max_resident_blocks * sizeof(int));
	assert(spill_plan.spill_bytes_estimate == spill_plan.planned_spill_blocks * sizeof(int));
	const auto spill_diagnostics =
		mtrc::space::storage::diagnostics_for_space(counting_space, spill_materialized_policy, {}, "neighbors");
	assert(*materialized_refusal_calls == 0);
	assert(spill_diagnostics.allow_out_of_core_spill);
	assert(spill_diagnostics.budget.allow_out_of_core_spill);
	assert(spill_diagnostics.out_of_core_plan);
	assert(spill_diagnostics.spill_enabled);
	assert(spill_diagnostics.explicit_spill_policy_required);
	assert(spill_diagnostics.memory_bounded);
	assert(spill_diagnostics.spill_block_size == spill_plan.spill_block_size);
	assert(spill_diagnostics.spill_block_count == spill_plan.spill_block_count);
	assert(spill_diagnostics.max_resident_blocks == spill_plan.max_resident_blocks);
	assert(spill_diagnostics.planned_spill_blocks == spill_plan.planned_spill_blocks);
	assert(spill_diagnostics.spill_block_bytes_estimate == spill_plan.spill_block_bytes_estimate);
	assert(spill_diagnostics.max_resident_bytes_estimate == spill_plan.max_resident_bytes_estimate);
	assert(spill_diagnostics.spill_bytes_estimate == spill_plan.spill_bytes_estimate);
	*materialized_refusal_calls = 0;
	const auto spill_probe =
		mtrc::space::storage::make_blocked_distance_table(counting_space, spill_materialized_policy);
	assert(*materialized_refusal_calls == 0);
	assert(spill_probe.spill_enabled());
	assert(!spill_probe.spill_directory().empty());
	assert(spill_probe.dense_distance_slots() == 0);
	assert(spill_probe.cached_distances() == 0);

	auto assert_same_neighbor_set = [](const auto &actual, const auto &expected) {
		assert(actual.size() == expected.size());
		for (std::size_t index = 0; index < actual.size(); ++index) {
			assert(actual[index].id == expected[index].id);
			assert(actual[index].distance == expected[index].distance);
		}
	};

	const auto exact_counting_neighbors =
		mtrc::find_neighbors(counting_space, counting_space.id(0), 2, lazy_policy);
	*materialized_refusal_calls = 0;
	const auto blocked_neighbors =
		mtrc::find_neighbors(counting_space, counting_space.id(0), 2, blocked_materialized_policy);
	assert(blocked_neighbors.exact);
	assert(blocked_neighbors.representation == "blocked_distance_table");
	assert(blocked_neighbors.record_count == exact_counting_neighbors.record_count);
	assert(blocked_neighbors.requested_count == exact_counting_neighbors.requested_count);
	assert_same_neighbor_set(blocked_neighbors, exact_counting_neighbors);
	assert(*materialized_refusal_calls > 0);

	const auto exact_counting_range =
		mtrc::stats::search::range(counting_space, counting_space.id(0), 1, lazy_policy);
	*materialized_refusal_calls = 0;
	const auto blocked_range =
		mtrc::stats::search::range(counting_space, counting_space.id(0), 1, blocked_materialized_policy);
	assert(blocked_range.exact);
	assert(blocked_range.representation == "blocked_distance_table");
	assert(blocked_range.record_count == exact_counting_range.record_count);
	assert_same_neighbor_set(blocked_range, exact_counting_range);
	assert(*materialized_refusal_calls > 0);

	auto assert_rejects_external_materialized_query = [](auto operation) {
		bool rejected = false;
		try {
			operation();
		} catch (const mtrc::InvalidRuntimePolicyError &error) {
			rejected = true;
			const std::string message = error.what();
			assert(message.find("materialized") != std::string::npos);
			assert(message.find("RecordId") != std::string::npos);
		}
		assert(rejected);
	};
	assert_rejects_external_materialized_query([&] {
		(void)mtrc::find_neighbors(counting_space, std::string("ee"), 2, blocked_materialized_policy);
	});
	assert_rejects_external_materialized_query([&] {
		(void)mtrc::stats::search::range(counting_space, std::string("ee"), 1,
										 blocked_materialized_policy);
	});

	const auto exact_counting_groups =
		mtrc::find_groups(counting_space, mtrc::stats::structural_analysis::k_medoids_options(2, 8),
						  lazy_policy);
	*materialized_refusal_calls = 0;
	const auto blocked_groups =
		mtrc::find_groups(counting_space, mtrc::stats::structural_analysis::k_medoids_options(2, 8),
						  blocked_materialized_policy);
	assert(blocked_groups.exact);
	assert(blocked_groups.representation == "blocked_distance_table");
	assert(blocked_groups.assignments == exact_counting_groups.assignments);
	assert(blocked_groups.medoids == exact_counting_groups.medoids);
	assert(*materialized_refusal_calls > 0);

	const auto exact_counting_outliers =
		mtrc::find_outliers(counting_space, mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
							lazy_policy);
	*materialized_refusal_calls = 0;
	const auto blocked_outliers =
		mtrc::find_outliers(counting_space, mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
							blocked_materialized_policy);
	assert(blocked_outliers.exact);
	assert(blocked_outliers.representation == "blocked_distance_table");
	assert(blocked_outliers.size() == exact_counting_outliers.size());
	assert(blocked_outliers.unassigned_count == exact_counting_outliers.unassigned_count);
	assert(*materialized_refusal_calls > 0);

	const auto exact_counting_density_filter =
		mtrc::density_filter(counting_space, mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
					  lazy_policy);
	*materialized_refusal_calls = 0;
	const auto blocked_density_filter =
		mtrc::density_filter(counting_space, mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
					  blocked_materialized_policy);
	assert(blocked_density_filter.representation == "blocked_distance_table");
	assert(blocked_density_filter.source_records == exact_counting_density_filter.source_records);
	assert(blocked_density_filter.representative_records == exact_counting_density_filter.representative_records);
	assert(*materialized_refusal_calls > 0);

	const auto exact_counting_compress = mtrc::compress(counting_space, 2, lazy_policy);
	*materialized_refusal_calls = 0;
	const auto blocked_compress = mtrc::compress(counting_space, 2, blocked_materialized_policy);
	assert(blocked_compress.exact);
	assert(blocked_compress.representation == "blocked_distance_table");
	assert(blocked_compress.source_record_ids == exact_counting_compress.source_record_ids);
	assert(blocked_compress.assignments == exact_counting_compress.assignments);
	assert(blocked_compress.nearest_representative_distances ==
		   exact_counting_compress.nearest_representative_distances);
	assert(*materialized_refusal_calls > 0);

	mtrc::space::storage::resource_budget chunked_budget;
	chunked_budget.max_memory_bytes = matrix_memory_bytes - 1;
	chunked_budget.max_dense_records = space.size();
	chunked_budget.allow_chunking = true;
	const auto chunked_plan = mtrc::space::storage::estimate_cost(space, "neighbors", matrix_policy, chunked_budget);
	assert(chunked_plan.budget_exceeded);
	assert(chunked_plan.allowed);
	assert(chunked_plan.downgraded);
	assert(!chunked_plan.refused);
	assert(chunked_plan.decision == mtrc::space::storage::budget_decision::downgraded);
	assert(chunked_plan.representation == "chunked_distance_table");
	assert(chunked_plan.reason.find("max_memory_bytes") != std::string::npos);
	assert(chunked_plan.fallback_hint.find("chunked") != std::string::npos);

	mtrc::space::storage::resource_budget live_eval_budget;
	live_eval_budget.max_distance_evaluations = space.size() - 1;
	const auto live_eval_plan = mtrc::space::storage::estimate_cost(space, "neighbors", lazy_policy, live_eval_budget);
	assert(live_eval_plan.budget_exceeded);
	assert(!live_eval_plan.allowed);
	assert(live_eval_plan.refused);
	assert(live_eval_plan.decision == mtrc::space::storage::budget_decision::refused);
	assert(live_eval_plan.estimated_distance_evaluations == space.size());
	assert(live_eval_plan.memory_bytes_estimate == 0);
	assert(live_eval_plan.reason.find("max_distance_evaluations") != std::string::npos);
	const auto policy_eval_limited =
		mtrc::space::storage::with_distance_evaluation_budget(lazy_policy, space.size() - 1);
	const auto policy_eval_limited_plan =
		mtrc::space::storage::estimate_cost(space, "neighbors", policy_eval_limited);
	assert(policy_eval_limited_plan.refused);
	assert(policy_eval_limited_plan.reason.find("max_distance_evaluations") != std::string::npos);
	auto assert_search_eval_refused_before_metric_calls = [&](auto operation) {
		*materialized_refusal_calls = 0;
		bool rejected = false;
		try {
			operation();
		} catch (const mtrc::RepresentationError &error) {
			rejected = true;
			const std::string message = error.what();
			assert(message.find("max_distance_evaluations") != std::string::npos);
			assert(message.find("fallback") != std::string::npos);
		}
		assert(rejected);
		assert(*materialized_refusal_calls == 0);
	};
	assert_search_eval_refused_before_metric_calls([&] {
		(void)mtrc::find_neighbors(counting_space, counting_space.id(0), 2, policy_eval_limited);
	});
	assert_search_eval_refused_before_metric_calls([&] {
		(void)mtrc::stats::search::range(counting_space, counting_space.id(0), 1, policy_eval_limited);
	});

	auto slow_runtime_calls = std::make_shared<std::size_t>(0);
	auto slow_runtime_space = mtrc::make_space(
		std::vector<std::string>{"a", "bb", "ccc", "dddd", "eeeee", "ffffff", "ggggggg", "hhhhhhhh"},
		SlowCountingLengthDistance(slow_runtime_calls));
	const auto runtime_limited_policy = mtrc::space::storage::with_runtime_budget(lazy_policy, 1);
	assert(runtime_limited_policy.max_runtime_ms() == 1);
	const auto runtime_limited_diagnostics =
		mtrc::space::storage::diagnostics_for_space(slow_runtime_space, runtime_limited_policy, {}, "neighbors");
	assert(runtime_limited_diagnostics.max_runtime_ms == 1);
	assert(runtime_limited_diagnostics.budget.max_runtime_ms == 1);
	const auto runtime_limited_plan =
		mtrc::space::storage::estimate_cost(slow_runtime_space, "neighbors", runtime_limited_policy);
	assert(runtime_limited_plan.max_runtime_ms == 1);
	assert(runtime_limited_plan.budget.max_runtime_ms == 1);
	auto assert_runtime_cancelled_before_full_scan = [&](auto operation, std::size_t full_scan_calls) {
		*slow_runtime_calls = 0;
		bool cancelled = false;
		try {
			operation();
		} catch (const mtrc::RepresentationError &error) {
			cancelled = true;
			const std::string message = error.what();
			assert(message.find("max_runtime_ms") != std::string::npos);
			assert(message.find("cancelled") != std::string::npos);
		}
		assert(cancelled);
		assert(*slow_runtime_calls < full_scan_calls);
	};
	assert_runtime_cancelled_before_full_scan([&] {
		(void)mtrc::find_neighbors(slow_runtime_space, slow_runtime_space.id(0), 2, runtime_limited_policy);
	}, slow_runtime_space.size() - 1);
	const auto dense_runtime_limited_policy =
		mtrc::space::storage::with_runtime_budget(materialized_policy, 1);
	assert_runtime_cancelled_before_full_scan([&] {
		(void)mtrc::find_neighbors(slow_runtime_space, slow_runtime_space.id(0), 2,
								   dense_runtime_limited_policy);
	}, slow_runtime_space.size() * slow_runtime_space.size());
	assert_runtime_cancelled_before_full_scan([&] {
		(void)mtrc::find_groups(slow_runtime_space,
								mtrc::stats::structural_analysis::k_medoids_options(2, 8),
								runtime_limited_policy);
	}, slow_runtime_space.size() * slow_runtime_space.size());
	assert_runtime_cancelled_before_full_scan([&] {
		(void)mtrc::find_groups(slow_runtime_space,
								mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
								runtime_limited_policy);
	}, slow_runtime_space.size() * slow_runtime_space.size());
	assert_runtime_cancelled_before_full_scan([&] {
		(void)mtrc::find_outliers(slow_runtime_space,
								  mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
								  runtime_limited_policy);
	}, slow_runtime_space.size() * slow_runtime_space.size());
	assert_runtime_cancelled_before_full_scan([&] {
		(void)mtrc::density_filter(slow_runtime_space,
							mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
							runtime_limited_policy);
	}, slow_runtime_space.size() * slow_runtime_space.size());
	const auto sampled_runtime_limited_policy =
		mtrc::space::storage::with_runtime_budget(mtrc::space::storage::approximate(), 1);
	assert_runtime_cancelled_before_full_scan([&] {
		(void)mtrc::find_groups(slow_runtime_space,
								mtrc::stats::structural_analysis::k_medoids_options(2, 8),
								sampled_runtime_limited_policy);
	}, slow_runtime_space.size() * 2);
	assert_runtime_cancelled_before_full_scan([&] {
		(void)mtrc::find_groups(slow_runtime_space,
								mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
								sampled_runtime_limited_policy);
	}, slow_runtime_space.size() * slow_runtime_space.size());
	assert_runtime_cancelled_before_full_scan([&] {
		(void)mtrc::find_outliers(slow_runtime_space,
								  mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
								  sampled_runtime_limited_policy);
	}, slow_runtime_space.size() * slow_runtime_space.size());
	assert_runtime_cancelled_before_full_scan([&] {
		(void)mtrc::density_filter(slow_runtime_space,
							mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
							sampled_runtime_limited_policy);
	}, slow_runtime_space.size() * slow_runtime_space.size());

	mtrc::space::storage::resource_budget approximate_budget;
	approximate_budget.max_distance_evaluations = space.size() - 1;
	approximate_budget.allow_approximate = true;
	const auto approximate_fallback_policy =
		mtrc::space::storage::with_resource_budget(lazy_policy, approximate_budget);
	assert(approximate_fallback_policy.max_distance_evaluations() == space.size() - 1);
	assert(approximate_fallback_policy.allows_approximate_fallback());
	const auto approximate_fallback_plan =
		mtrc::space::storage::estimate_cost(space, "neighbors", approximate_fallback_policy);
	assert(approximate_fallback_plan.budget_exceeded);
	assert(approximate_fallback_plan.allowed);
	assert(approximate_fallback_plan.downgraded);
	assert(!approximate_fallback_plan.exact);
	assert(approximate_fallback_plan.representation == "sampled_metric_space");
	*materialized_refusal_calls = 0;
	const auto eval_bounded_approximate_neighbors =
		mtrc::find_neighbors(counting_space, counting_space.id(0), 2, approximate_fallback_policy);
	assert(!eval_bounded_approximate_neighbors.exact);
	assert(eval_bounded_approximate_neighbors.representation == "sampled_metric_space");
	assert(eval_bounded_approximate_neighbors.approximation_quality.candidate_count == counting_space.size() - 1);
	assert(!eval_bounded_approximate_neighbors.approximation_quality.recall_measured);
	assert(*materialized_refusal_calls == counting_space.size() - 1);
	*materialized_refusal_calls = 0;
	const auto eval_bounded_approximate_range =
		mtrc::stats::search::range(counting_space, counting_space.id(0), 1, approximate_fallback_policy);
	assert(!eval_bounded_approximate_range.exact);
	assert(eval_bounded_approximate_range.representation == "sampled_metric_space");
	assert(eval_bounded_approximate_range.approximation_quality.candidate_count == counting_space.size() - 1);
	assert(!eval_bounded_approximate_range.approximation_quality.recall_measured);
	assert(*materialized_refusal_calls == counting_space.size() - 1);
	const auto materialized_approximate_fallback_policy = mtrc::space::storage::allow_approximate_fallback(
		mtrc::space::storage::with_distance_table_budget(
			matrix_policy, counting_space.size(), counting_matrix_memory_bytes - 1));
	const auto materialized_approximate_fallback_plan =
		mtrc::space::storage::estimate_cost(counting_space, "neighbors", materialized_approximate_fallback_policy);
	assert(materialized_approximate_fallback_plan.budget_exceeded);
	assert(materialized_approximate_fallback_plan.allowed);
	assert(materialized_approximate_fallback_plan.downgraded);
	assert(!materialized_approximate_fallback_plan.exact);
	assert(materialized_approximate_fallback_plan.representation == "sampled_metric_space");
	*materialized_refusal_calls = 0;
	const auto materialized_approximate_neighbors =
		mtrc::find_neighbors(counting_space, counting_space.id(0), 2, materialized_approximate_fallback_policy);
	assert(!materialized_approximate_neighbors.exact);
	assert(materialized_approximate_neighbors.representation == "sampled_metric_space");
	assert(materialized_approximate_neighbors.approximation_quality.candidate_count == counting_space.size() - 1);
	assert(materialized_approximate_neighbors.approximation_quality.recall_measured);
	assert(materialized_approximate_neighbors.approximation_quality.recall == 1.0);
	assert(*materialized_refusal_calls == 2 * (counting_space.size() - 1));
	*materialized_refusal_calls = 0;
	const auto materialized_approximate_range =
		mtrc::stats::search::range(counting_space, counting_space.id(0), 1,
								   materialized_approximate_fallback_policy);
	assert(!materialized_approximate_range.exact);
	assert(materialized_approximate_range.representation == "sampled_metric_space");
	assert(materialized_approximate_range.approximation_quality.candidate_count == counting_space.size() - 1);
	assert(materialized_approximate_range.approximation_quality.recall_measured);
	assert(materialized_approximate_range.approximation_quality.recall == 1.0);
	assert(*materialized_refusal_calls == 2 * (counting_space.size() - 1));
	const auto chunking_fallback_policy =
		mtrc::space::storage::allow_chunking_fallback(mtrc::space::storage::allow_approximate_fallback(lazy_policy));
	assert(chunking_fallback_policy.allows_approximate_fallback());
	assert(chunking_fallback_policy.allows_chunking_fallback());

	const auto tree_policy = mtrc::space::storage::using_cover_tree();
	assert(tree_policy.name() == "exact_materialized_serial");
	assert(tree_policy.representation_preference() == "cover_tree_index");
		const auto tree_diagnostics = mtrc::space::storage::diagnostics(tree_policy, {}, "neighbors");
		assert(tree_diagnostics.materialized);
		assert(tree_diagnostics.representation == "cover_tree_index");
		const auto tree_plan = mtrc::space::storage::estimate_cost(space, "neighbors", tree_policy);
		assert(tree_plan.estimated_distance_evaluations >= space.size() * space.size());
		assert(tree_plan.memory_bytes_estimate > 0);
		auto edit_space =
			mtrc::make_space(std::vector<std::string>{"metric", "metrics", "matrix", "tree"}, mtrc::Edit<char>{});
		const auto edit_lazy_neighbors = mtrc::find_neighbors(edit_space, std::string("metricks"), 2, lazy_policy);
	const auto tree_query_neighbors = mtrc::find_neighbors(edit_space, std::string("metricks"), 2, tree_policy);
	assert(tree_query_neighbors.representation == "cover_tree_index");
	assert(tree_query_neighbors[0].id == edit_lazy_neighbors[0].id);
	assert(tree_query_neighbors[0].distance == edit_lazy_neighbors[0].distance);
	const auto tree_id_neighbors = mtrc::find_neighbors(edit_space, edit_space.id(0), 2, tree_policy);
	assert(tree_id_neighbors.representation == "cover_tree_index");
	assert(tree_id_neighbors.size() == edit_lazy_neighbors.size());
	const auto edit_lazy_id_neighbors = mtrc::find_neighbors(edit_space, edit_space.id(0), 2, lazy_policy);
	assert(tree_id_neighbors[0].id == edit_lazy_id_neighbors[0].id);
	assert(tree_id_neighbors[0].distance == edit_lazy_id_neighbors[0].distance);
		const auto empty_tree_id_neighbors = mtrc::find_neighbors(edit_space, edit_space.id(0), 0, tree_policy);
		assert(empty_tree_id_neighbors.representation == "cover_tree_index");
		assert(empty_tree_id_neighbors.empty());
		const auto cover_tree_refusal_calls = std::make_shared<std::size_t>(0);
		auto cover_tree_counting_space = mtrc::make_space(
			std::vector<double>{0.0, 1.0, 2.0, 3.0}, CountingDoubleAbsoluteDistance(cover_tree_refusal_calls));
		const auto cover_tree_build_limited_policy = mtrc::space::storage::with_distance_evaluation_budget(
			mtrc::space::storage::using_cover_tree(), cover_tree_counting_space.size());
		const auto cover_tree_build_limited_plan =
			mtrc::space::storage::estimate_cost(cover_tree_counting_space, "neighbors",
												cover_tree_build_limited_policy);
		assert(cover_tree_build_limited_plan.refused);
		assert(cover_tree_build_limited_plan.estimated_distance_evaluations >=
			   cover_tree_counting_space.size() * cover_tree_counting_space.size());
		assert(cover_tree_build_limited_plan.reason.find("max_distance_evaluations") != std::string::npos);
		auto assert_cover_tree_eval_refused_before_metric_calls = [&](auto operation) {
			*cover_tree_refusal_calls = 0;
			bool rejected = false;
			try {
				operation();
			} catch (const mtrc::RepresentationError &error) {
				rejected = true;
				const std::string message = error.what();
				assert(message.find("max_distance_evaluations") != std::string::npos);
			}
			assert(rejected);
			assert(*cover_tree_refusal_calls == 0);
		};
		assert_cover_tree_eval_refused_before_metric_calls([&] {
			(void)mtrc::find_neighbors(cover_tree_counting_space, 1.5, 2, cover_tree_build_limited_policy);
		});
		assert_cover_tree_eval_refused_before_metric_calls([&] {
			(void)mtrc::find_neighbors(cover_tree_counting_space, cover_tree_counting_space.id(0), 2,
									   cover_tree_build_limited_policy);
		});
		assert_cover_tree_eval_refused_before_metric_calls([&] {
			(void)mtrc::stats::search::range(cover_tree_counting_space, 1.5, 1.0,
											 cover_tree_build_limited_policy);
		});
		assert_cover_tree_eval_refused_before_metric_calls([&] {
			(void)mtrc::stats::search::range(cover_tree_counting_space, cover_tree_counting_space.id(0), 1.0,
											 cover_tree_build_limited_policy);
		});

		const auto graph_policy = mtrc::space::storage::using_knn_graph(2);
	assert(graph_policy.name() == "exact_materialized_serial");
	assert(graph_policy.graph_neighbors() == 2);
	assert(graph_policy.representation_preference() == "knn_graph_index");
	const auto graph_plan = mtrc::space::storage::estimate_cost(space, "neighbors", graph_policy);
	assert(graph_plan.estimated_distance_evaluations >= space.size() * (space.size() - 1));
	const auto graph_diagnostics = mtrc::space::storage::diagnostics(graph_policy, {}, "neighbors");
	assert(graph_diagnostics.representation == "knn_graph_index");
	const auto graph_query_neighbors = mtrc::find_neighbors(space, std::string("ee"), 2, graph_policy);
	assert(graph_query_neighbors.representation == "knn_graph_index");
	assert(graph_query_neighbors[0].id == space.id(1));
	const auto graph_id_neighbors = mtrc::find_neighbors(space, space.id(0), 2, graph_policy);
	assert(graph_id_neighbors.representation == "knn_graph_index");
	assert(graph_id_neighbors.exact);
	assert(graph_id_neighbors[0].id == materialized_neighbors[0].id);
	const auto default_graph_policy = mtrc::space::storage::using_knn_graph();
	const auto default_graph_id_neighbors = mtrc::find_neighbors(space, space.id(0), 2, default_graph_policy);
	assert(default_graph_id_neighbors.representation == "knn_graph_index");
	assert(default_graph_id_neighbors.exact);
	bool rejected_underprovisioned_graph_policy = false;
	try {
		(void)mtrc::find_neighbors(space, space.id(0), 2, mtrc::space::storage::using_knn_graph(1));
	} catch (const mtrc::RepresentationError &) {
		rejected_underprovisioned_graph_policy = true;
	}
	assert(rejected_underprovisioned_graph_policy);

	const auto graph_build_limited_policy = mtrc::space::storage::with_distance_evaluation_budget(
		mtrc::space::storage::using_knn_graph(2), counting_space.size());
	const auto graph_build_limited_plan =
		mtrc::space::storage::estimate_cost(counting_space, "neighbors", graph_build_limited_policy);
	assert(graph_build_limited_plan.refused);
	assert(graph_build_limited_plan.estimated_distance_evaluations >=
		   counting_space.size() * (counting_space.size() - 1));
	assert(graph_build_limited_plan.reason.find("max_distance_evaluations") != std::string::npos);
	assert_search_eval_refused_before_metric_calls([&] {
		(void)mtrc::find_neighbors(counting_space, counting_space.id(0), 2, graph_build_limited_policy);
	});
	assert_search_eval_refused_before_metric_calls([&] {
		(void)mtrc::stats::search::range(counting_space, counting_space.id(0), 1, graph_build_limited_policy);
	});

	const auto parallel_policy = mtrc::space::storage::parallel(materialized_policy);
	assert(parallel_policy.name() == "exact_materialized_parallel");
	assert(parallel_policy.uses_parallel_execution());
	const auto explicit_diagnostics = mtrc::space::storage::diagnostics(parallel_policy, "cover_tree_index", "neighbors");
	assert(explicit_diagnostics.parallel);
	assert(explicit_diagnostics.materialized);
	assert(explicit_diagnostics.representation == "cover_tree_index");
	const auto parallel_neighbors = mtrc::find_neighbors(space, space.id(0), 2, parallel_policy);
	assert(parallel_neighbors.representation == "distance_table");

	auto stateful_space =
		mtrc::make_space(std::vector<std::string>{"a", "bb", "ccc", "dddd"}, StatefulLengthDistance{});
	const auto stateful_policy = mtrc::space::storage::parallel(mtrc::space::storage::exact());
	static_assert(!mtrc::metric_thread_safe_v<StatefulLengthDistance>);
	const auto stateful_diagnostics =
		mtrc::space::storage::diagnostics_for_metric<StatefulLengthDistance>(stateful_policy, {}, "neighbors");
	assert(stateful_diagnostics.parallel);
	assert(!stateful_diagnostics.supported);
	assert(stateful_diagnostics.reason.find("thread-safe") != std::string::npos);
	const auto stateful_space_diagnostics =
		mtrc::space::storage::diagnostics_for_space(stateful_space, stateful_policy, {}, "neighbors");
	assert(!stateful_space_diagnostics.supported);

	bool rejected_stateful_parallel_neighbors = false;
	try {
		(void)mtrc::find_neighbors(stateful_space, stateful_space.id(0), 2, stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_neighbors = true;
	}
	assert(rejected_stateful_parallel_neighbors);

	bool rejected_stateful_parallel_groups = false;
	try {
		(void)mtrc::find_groups(stateful_space, mtrc::stats::structural_analysis::k_medoids_options(2), stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_groups = true;
	}
	assert(rejected_stateful_parallel_groups);

	bool rejected_stateful_parallel_describe = false;
	try {
		(void)mtrc::describe_structure(stateful_space, stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_describe = true;
	}
	assert(rejected_stateful_parallel_describe);

	bool rejected_stateful_parallel_compare = false;
	try {
		(void)mtrc::compare(stateful_space, stateful_space, stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_compare = true;
	}
	assert(rejected_stateful_parallel_compare);

	auto size_distance = [](std::size_t lhs, std::size_t rhs) -> std::size_t {
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	};
	bool rejected_stateful_parallel_map = false;
	try {
		(void)mtrc::map(
			stateful_space, [](const std::string &record) -> std::size_t { return record.size(); }, size_distance,
			stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_map = true;
	}
	assert(rejected_stateful_parallel_map);

	const auto lazy_groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2), lazy_policy);
	assert(lazy_groups.algorithm == "kmedoids");
	assert(lazy_groups.representation == "metric_space");
	assert(lazy_groups.cluster_count == 2);

	const auto materialized_groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2), materialized_policy);
	assert(materialized_groups.algorithm == "kmedoids");
	assert(materialized_groups.representation == "distance_table");
	assert(materialized_groups.assignments == lazy_groups.assignments);
	assert(materialized_groups.medoids == lazy_groups.medoids);

	const auto counted_materialized_groups = mtrc::find_groups(space, 2, materialized_policy);
	assert(counted_materialized_groups.representation == "distance_table");
	assert(counted_materialized_groups.assignments == materialized_groups.assignments);

	const auto density_groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::dbscan_options(1.0, 2), materialized_policy);
	assert(density_groups.algorithm == "dbscan");
	assert(density_groups.representation == "distance_table");
	assert(density_groups.record_count == space.size());

	auto continuous_space = mtrc::make_space(std::vector<double>{0.0, 0.1, 10.0, 10.1}, DoubleAbsoluteDistance{});
	const auto affinity_groups =
		mtrc::find_groups(continuous_space, mtrc::stats::structural_analysis::affinity_propagation_options(0.7), materialized_policy);
	assert(affinity_groups.algorithm == "affinity_propagation");
	assert(affinity_groups.representation == "distance_table");
	assert(affinity_groups.record_count == continuous_space.size());
	assert(affinity_groups.cluster_count == affinity_groups.medoids.size());

	auto cached_matrix = mtrc::space::storage::cache(mtrc::space::storage::DistanceTable<decltype(space)>(space));
	assert(!cached_matrix.is_stale());
	assert(cached_matrix.version() == space.version());
	space.touch();
	assert(cached_matrix.is_stale());

	const auto approximate_neighbors =
		mtrc::find_neighbors(space, space.id(0), 2, mtrc::space::storage::approximate());
	assert(!approximate_neighbors.exact);
	assert(approximate_neighbors.operator_name == "knn");
	assert(approximate_neighbors.representation == "sampled_metric_space");
	assert(approximate_neighbors.provenance.route == "approximate_sampled_candidates");
	assert(approximate_neighbors.provenance.route_kind == mtrc::search_route_kind::approximate_sampled_candidates);
	assert(approximate_neighbors.provenance.domain_kind == mtrc::result_domain_kind::sampled_metric_space);
	assert(approximate_neighbors.approximation_quality.diagnostic == "search_approximation");
	assert(approximate_neighbors.approximation_quality.candidate_policy == "regular_sample");
	assert(approximate_neighbors.approximation_quality.candidate_count == space.size() - 1);
	assert(approximate_neighbors.approximation_quality.candidate_universe == space.size() - 1);
	assert(approximate_neighbors.approximation_quality.distance_evaluations == 2 * (space.size() - 1));
	assert(approximate_neighbors.approximation_quality.requested_count == 2);
	assert(approximate_neighbors.approximation_quality.candidate_fraction == 1.0);
	assert(approximate_neighbors.approximation_quality.recall_measured);
	assert(approximate_neighbors.approximation_quality.recall == 1.0);
	assert(approximate_neighbors.approximation_quality.recall_reference_count == 2);
	assert(approximate_neighbors.approximation_quality.recall_matched_count == 2);
	assert(approximate_neighbors.approximation_quality.recall_distance_evaluations == space.size() - 1);
	assert(approximate_neighbors.approximation_quality.recall_sample_query_count == 1);
	assert(approximate_neighbors.approximation_quality.standard_error == 0.0);
	assert(approximate_neighbors.approximation_quality.confidence_radius_95 == 0.0);
	assert(!approximate_neighbors.approximation_quality.reason.empty());
	assert(approximate_neighbors.requested_count == 2);
	assert(approximate_neighbors.size() <= 2);
	assert(!approximate_neighbors.empty());
	const auto bounded_approximate_neighbors =
		mtrc::find_neighbors(space, space.id(0), 2,
								 mtrc::space::storage::using_knn_graph(1, mtrc::space::storage::approximate()));
	assert(!bounded_approximate_neighbors.exact);
	assert(bounded_approximate_neighbors.representation == "sampled_metric_space");
	assert(bounded_approximate_neighbors.requested_count == 2);
	assert(bounded_approximate_neighbors.size() == 1);
	assert(bounded_approximate_neighbors.provenance.route == "approximate_sampled_candidates");
	assert(bounded_approximate_neighbors.provenance.route_kind ==
		   mtrc::search_route_kind::approximate_sampled_candidates);
	assert(bounded_approximate_neighbors.provenance.domain_kind == mtrc::result_domain_kind::sampled_metric_space);
	assert(bounded_approximate_neighbors.approximation_quality.candidate_count == 1);
	assert(bounded_approximate_neighbors.approximation_quality.candidate_universe == space.size() - 1);
	assert(bounded_approximate_neighbors.approximation_quality.distance_evaluations == space.size());
	assert(bounded_approximate_neighbors.approximation_quality.candidate_fraction == 1.0 / 3.0);
	assert(bounded_approximate_neighbors.approximation_quality.recall_measured);
	assert(bounded_approximate_neighbors.approximation_quality.recall == 0.5);
	assert(bounded_approximate_neighbors.approximation_quality.recall_reference_count == 2);
	assert(bounded_approximate_neighbors.approximation_quality.recall_matched_count == 1);
	assert(bounded_approximate_neighbors.approximation_quality.recall_distance_evaluations == space.size() - 1);
	assert(nearly_equal(
		bounded_approximate_neighbors.approximation_quality.standard_error,
		std::sqrt(0.5 * 0.5 / 2.0)));
	assert(nearly_equal(
		bounded_approximate_neighbors.approximation_quality.confidence_radius_95,
		1.96 * std::sqrt(0.5 * 0.5 / 2.0)));
	const auto approximate_range =
		mtrc::stats::search::range(space, space.id(0), 1, mtrc::space::storage::approximate());
	assert(!approximate_range.exact);
	assert(approximate_range.operator_name == "range");
	assert(approximate_range.representation == "sampled_metric_space");
	assert(approximate_range.provenance.route == "approximate_sampled_candidates");
	assert(approximate_range.provenance.route_kind == mtrc::search_route_kind::approximate_sampled_candidates);
	assert(approximate_range.provenance.domain_kind == mtrc::result_domain_kind::sampled_metric_space);
	assert(approximate_range.approximation_quality.diagnostic == "search_approximation");
	assert(approximate_range.approximation_quality.candidate_count == space.size() - 1);
	assert(approximate_range.approximation_quality.candidate_universe == space.size() - 1);
	assert(approximate_range.approximation_quality.distance_evaluations == 2 * (space.size() - 1));
	assert(approximate_range.approximation_quality.requested_count == 0);
	assert(approximate_range.approximation_quality.recall_measured);
	assert(approximate_range.approximation_quality.recall == 1.0);
	assert(approximate_range.approximation_quality.recall_reference_count == 1);
	assert(approximate_range.approximation_quality.recall_matched_count == 1);
	assert(approximate_range.approximation_quality.recall_distance_evaluations == space.size() - 1);
	assert(approximate_range.approximation_quality.standard_error == 0.0);
	assert(approximate_range.approximation_quality.confidence_radius_95 == 0.0);
	assert(!approximate_range.empty());
	const auto approximate_diagnostics =
		mtrc::space::storage::diagnostics(mtrc::space::storage::approximate(), {}, "neighbors");
	assert(approximate_diagnostics.policy_name == "approximate_lazy_serial");
	assert(approximate_diagnostics.intent == "neighbors");
	assert(!approximate_diagnostics.exact);
	assert(approximate_diagnostics.supported);
	assert(!approximate_diagnostics.refused);
	assert(approximate_diagnostics.reason.empty());
	const auto approximate_neighbor_plan =
		mtrc::space::storage::estimate_cost(space, "neighbors", mtrc::space::storage::approximate());
	assert(!approximate_neighbor_plan.exact);
	assert(approximate_neighbor_plan.exactness == "approximate");
	assert(approximate_neighbor_plan.allowed);
	assert(!approximate_neighbor_plan.refused);
	assert(approximate_neighbor_plan.representation == "sampled_metric_space");
	assert(approximate_neighbor_plan.estimated_distance_evaluations == space.size());
	const auto approximate_unknown_metric_batch_plan =
		mtrc::space::storage::estimate_cost(space, "neighbors", 3, mtrc::space::storage::approximate());
	assert(!approximate_unknown_metric_batch_plan.exact);
	assert(approximate_unknown_metric_batch_plan.representation == "sampled_metric_space");
	assert(approximate_unknown_metric_batch_plan.estimated_distance_evaluations == 3 * space.size());
	const auto approximate_metric_single_query_plan =
		mtrc::space::storage::estimate_cost(continuous_space, "neighbors", mtrc::space::storage::approximate());
	assert(!approximate_metric_single_query_plan.exact);
	assert(approximate_metric_single_query_plan.representation == "sampled_metric_space");
	const auto approximate_metric_batch_plan =
		mtrc::space::storage::estimate_cost(continuous_space, "neighbors", 3, mtrc::space::storage::approximate());
	assert(!approximate_metric_batch_plan.exact);
	assert(approximate_metric_batch_plan.representation == "sampled_metric_space");
	std::vector<double> large_metric_records;
	large_metric_records.reserve(20000);
	for (std::size_t index = 0; index < 20000; ++index) {
		large_metric_records.push_back(static_cast<double>(index));
	}
	auto large_metric_space = mtrc::make_space(large_metric_records, DoubleAbsoluteDistance{});
	const auto auto_landmark_candidate_limit = mtrc::space::storage::landmark_index_candidate_limit(
		large_metric_space.size(), mtrc::space::storage::approximate(), true, 3, 256);
	assert(auto_landmark_candidate_limit <= mtrc::space::storage::default_landmark_index_candidates);
	assert(auto_landmark_candidate_limit >= 64);
	const auto tuned_metric_batch_plan =
		mtrc::space::storage::estimate_cost(large_metric_space, "neighbors", 256,
											mtrc::space::storage::approximate());
	assert(!tuned_metric_batch_plan.exact);
	assert(tuned_metric_batch_plan.representation == "landmark_index");
	assert(tuned_metric_batch_plan.estimated_distance_evaluations < 4096 * 256);
	const auto explicit_landmark_policy = mtrc::space::storage::using_landmark_index(7);
	assert(mtrc::space::storage::landmark_index_candidate_limit(
			   large_metric_space.size(), explicit_landmark_policy, true, 3, 256) == 7);
	const auto explicit_landmark_plan =
		mtrc::space::storage::estimate_cost(large_metric_space, "neighbors", 256,
											explicit_landmark_policy);
	assert(explicit_landmark_plan.representation == "landmark_index");
	const auto approximate_compare_diagnostics =
		mtrc::space::storage::diagnostics(mtrc::space::storage::approximate(), {}, "compare");
	assert(!approximate_compare_diagnostics.exact);
	assert(approximate_compare_diagnostics.supported);
	assert(!approximate_compare_diagnostics.refused);
	assert(approximate_compare_diagnostics.reason.empty());
	const auto approximate_compare_plan =
		mtrc::space::storage::estimate_cost(space, "compare", mtrc::space::storage::approximate());
	assert(!approximate_compare_plan.exact);
	assert(approximate_compare_plan.exactness == "approximate");
	assert(approximate_compare_plan.allowed);
	assert(!approximate_compare_plan.refused);
	assert(approximate_compare_plan.representation == "metric_space_sample");
	assert(approximate_compare_plan.estimated_distance_evaluations > 0);

	const auto approximate_groups =
		mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2),
						  mtrc::space::storage::approximate());
	assert(!approximate_groups.exact);
	assert(approximate_groups.algorithm == "sampled_kmedoids");
	assert(approximate_groups.representation == "sampled_metric_space");
	assert(approximate_groups.record_count == space.size());
	assert(approximate_groups.cluster_count == 2);
	const auto approximate_group_plan =
		mtrc::space::storage::estimate_cost(space, "groups", mtrc::space::storage::approximate());
	assert(!approximate_group_plan.exact);
	assert(approximate_group_plan.allowed);
	assert(!approximate_group_plan.refused);
	assert(approximate_group_plan.representation == "sampled_metric_space");
	const auto approximate_density_filter_diagnostics =
		mtrc::space::storage::diagnostics(mtrc::space::storage::approximate(), {}, "density_filter");
	assert(!approximate_density_filter_diagnostics.exact);
	assert(approximate_density_filter_diagnostics.supported);
	assert(!approximate_density_filter_diagnostics.refused);
	const auto approximate_density_filter_plan =
		mtrc::space::storage::estimate_cost(space, "density_filter", mtrc::space::storage::approximate());
	assert(!approximate_density_filter_plan.exact);
	assert(approximate_density_filter_plan.allowed);
	assert(!approximate_density_filter_plan.refused);
	assert(approximate_density_filter_plan.representation == "sampled_metric_space");
	assert(approximate_density_filter_plan.estimated_distance_evaluations >= space.size());
	const auto approximate_density_filter =
		mtrc::density_filter(space, mtrc::stats::structural_analysis::dbscan_options(1.0, 2),
					  mtrc::space::storage::approximate());
	assert(approximate_density_filter.strategy == "sampled_dbscan_density_filter");
	assert(approximate_density_filter.representation == "sampled_metric_space");
	assert(approximate_density_filter.source_record_count == space.size());
	const auto approximate_compress_diagnostics =
		mtrc::space::storage::diagnostics(mtrc::space::storage::approximate(), {}, "compress");
	assert(!approximate_compress_diagnostics.exact);
	assert(approximate_compress_diagnostics.supported);
	assert(!approximate_compress_diagnostics.refused);
	const auto approximate_compress_plan =
		mtrc::space::storage::estimate_cost(space, "compress", mtrc::space::storage::approximate());
	assert(!approximate_compress_plan.exact);
	assert(approximate_compress_plan.allowed);
	assert(!approximate_compress_plan.refused);
	assert(approximate_compress_plan.representation == "sampled_metric_space");
	assert(approximate_compress_plan.estimated_distance_evaluations >= space.size());
	const auto approximate_compress = mtrc::compress(space, 2, mtrc::space::storage::approximate());
	assert(!approximate_compress.exact);
	assert(approximate_compress.strategy == "sampled_farthest_first");
	assert(approximate_compress.representation == "sampled_metric_space");
	assert(approximate_compress.source_record_count == space.size());
	std::vector<std::string> large_records;
	large_records.reserve(4100);
	for (std::size_t index = 0; index < 4100; ++index) {
		large_records.push_back(std::string((index % 7) + 1, 'x'));
	}
	auto large_space = mtrc::make_space(large_records, StringLengthDistance{});
	const auto default_large_groups =
		mtrc::find_groups(large_space, mtrc::stats::structural_analysis::k_medoids_options(2));
	assert(!default_large_groups.exact);
	assert(default_large_groups.algorithm == "sampled_kmedoids");
	assert(default_large_groups.representation == "sampled_metric_space");
	assert(default_large_groups.record_count == large_space.size());
	const auto default_large_density_filter =
		mtrc::density_filter(large_space, mtrc::stats::structural_analysis::dbscan_options(1, 2));
	assert(default_large_density_filter.strategy == "sampled_dbscan_density_filter");
	assert(default_large_density_filter.representation == "sampled_metric_space");
	assert(default_large_density_filter.source_record_count == large_space.size());
	const auto group_diagnostics = mtrc::space::storage::diagnostics(materialized_policy, {}, "groups");
	assert(group_diagnostics.representation == "distance_table");
	assert(group_diagnostics.intent == "groups");

	bool rejected_materialized_record_query = false;
	try {
		(void)mtrc::find_neighbors(space, std::string("ee"), 2, materialized_policy);
	} catch (const mtrc::InvalidRuntimePolicyError &) {
		rejected_materialized_record_query = true;
	}
	assert(rejected_materialized_record_query);

	return 0;
}
