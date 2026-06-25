#include <cassert>
#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "metric/core/errors.hpp"
#include "metric/core/metric_space.hpp"
#include "metric/core/metric_traits.hpp"
#include "metric/space/execution_context.hpp"
#include "metric/space/sample_plan.hpp"
#include "metric/space/storage/policy.hpp"

struct CountingAbsoluteDistance {
	std::shared_ptr<std::size_t> calls;

	explicit CountingAbsoluteDistance(std::shared_ptr<std::size_t> call_counter)
		: calls(std::move(call_counter))
	{
	}

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

struct SlowCountingAbsoluteDistance {
	std::shared_ptr<std::size_t> calls;

	explicit SlowCountingAbsoluteDistance(std::shared_ptr<std::size_t> call_counter)
		: calls(std::move(call_counter))
	{
	}

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

namespace mtrc::core {
template <> struct metric_traits<CountingAbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

template <> struct metric_traits<SlowCountingAbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};
} // namespace mtrc::core

template <typename Callable> auto assert_representation_error(Callable &&call, const std::string &expected) -> void
{
	bool rejected = false;
	try {
		call();
	} catch (const mtrc::RepresentationError &error) {
		rejected = true;
		const std::string message = error.what();
		assert(message.find(expected) != std::string::npos);
	}
	assert(rejected);
}

int main()
{
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(std::vector<int>{0, 2, 5, 9}, CountingAbsoluteDistance(calls));
	const auto query_id = space.id(0);

	const auto matrix_policy = mtrc::space::storage::using_distance_table();
	auto ctx = mtrc::space::execution_context(space, matrix_policy);
	const auto symmetric_slots = space.size() * (space.size() - 1) / 2;
	assert(*calls == symmetric_slots);
	assert(ctx.shared_provider());
	assert(ctx.provider_build_count() == 1);
	assert(ctx.provider_representation() == "symmetric_distance_table");
	const auto materialized_status = ctx.diagnostics();
	assert(materialized_status.shared_provider);
	assert(materialized_status.materialized_provider);
	assert(materialized_status.provider_build_count == 1);
	assert(materialized_status.provider_representation == "symmetric_distance_table");
	assert(materialized_status.cached_distances == symmetric_slots);
	assert(materialized_status.dense_distance_slots == symmetric_slots);
	assert(materialized_status.runtime.materialized);
	assert(materialized_status.runtime.representation == "symmetric_distance_table");

	const auto calls_after_build = *calls;
	const auto neighbors = ctx.neighbors(query_id, 2);
	assert(neighbors.representation == "symmetric_distance_table");
	assert(neighbors.provenance.route_kind == mtrc::search_route_kind::pairwise_provider);
	assert(neighbors.provenance.domain_kind == mtrc::result_domain_kind::pairwise_provider);
	assert(neighbors.size() == 2);
	assert(neighbors[0].id == space.id(1));
	assert(*calls == calls_after_build);

	const auto groups = ctx.groups(mtrc::stats::structural_analysis::dbscan_options(3.0, 2));
	assert(groups.representation == "symmetric_distance_table");
	assert(groups.record_count == space.size());
	assert(*calls == calls_after_build);

	const auto outliers = ctx.nearest_neighbor_outliers(1);
	assert(outliers.representation == "symmetric_distance_table");
	assert(outliers.record_count == space.size());
	assert(*calls == calls_after_build);

	auto describe_options = mtrc::describe_options{};
	describe_options.allow_approximate_intrinsic_dimension = false;
	const auto description = ctx.describe(describe_options);
	assert(description.representation == "symmetric_distance_table");
	assert(description.record_count == space.size());
	assert(description.pair_count == 6);
	assert(*calls == calls_after_build);

	const auto lazy_calls = std::make_shared<std::size_t>(0);
	auto lazy_space = mtrc::make_space(std::vector<int>{0, 2, 5, 9}, CountingAbsoluteDistance(lazy_calls));
	auto live_ctx = mtrc::space::execution_context(lazy_space, mtrc::space::storage::exact());
	assert(*lazy_calls == 0);
	assert(!live_ctx.shared_provider());
	assert(live_ctx.provider_build_count() == 0);
	assert(live_ctx.provider_representation() == "metric_space");
	const auto live_status = live_ctx.diagnostics();
	assert(!live_status.shared_provider);
	assert(!live_status.materialized_provider);
	assert(live_status.provider_build_count == 0);
	assert(live_status.provider_representation == "metric_space");
	assert(!live_status.runtime.materialized);

	const auto live_neighbors = live_ctx.neighbors(lazy_space.id(0), 2);
	assert(live_neighbors.representation == "metric_space");
	assert(live_neighbors.provenance.route_kind == mtrc::search_route_kind::source_metric);
	assert(live_neighbors.provenance.domain_kind == mtrc::result_domain_kind::metric_space);
	assert(live_neighbors[0].id == lazy_space.id(1));
	assert(*lazy_calls > 0);

	std::vector<int> approximate_records;
	approximate_records.reserve(4101);
	for (int value = 0; value < 4101; ++value) {
		approximate_records.push_back(value);
	}
	const auto approximate_calls = std::make_shared<std::size_t>(0);
	auto approximate_space = mtrc::make_space(
		approximate_records, CountingAbsoluteDistance(approximate_calls));
	const auto approximate_policy =
		mtrc::space::storage::using_knn_graph(4, mtrc::space::storage::approximate());
	auto approximate_ctx = mtrc::space::execution_context(approximate_space, approximate_policy);
	assert(!approximate_ctx.shared_provider());
	assert(approximate_ctx.provider_build_count() == 0);
	assert(approximate_ctx.provider_representation() == "sampled_metric_space");
	const auto approximate_initial_status = approximate_ctx.diagnostics();
	assert(approximate_initial_status.runtime.supported);
	assert(approximate_initial_status.runtime.representation == "sampled_metric_space");
	assert(approximate_initial_status.sample_planning_enabled);
	assert(approximate_initial_status.sample_plan_policy == "regular_sample");
	assert(!approximate_initial_status.sample_plan_cached);
	assert(approximate_initial_status.sample_plan_build_count == 0);

	const auto approximate_query = approximate_space.id(2);
	const auto expected_plan =
		mtrc::space::regular_sample_positions_excluding(approximate_space.size(), 2, 4);
	const auto approximate_neighbors = approximate_ctx.neighbors(approximate_query, 2);
	assert(!approximate_neighbors.exact);
	assert(approximate_neighbors.representation == "sampled_metric_space");
	assert(approximate_neighbors.provenance.route_kind ==
		   mtrc::search_route_kind::approximate_sampled_candidates);
	assert(approximate_neighbors.provenance.domain_kind == mtrc::result_domain_kind::sampled_metric_space);
	assert(approximate_neighbors.approximation_quality.candidate_count == expected_plan.size());
	assert(approximate_neighbors.approximation_quality.candidate_universe == expected_plan.candidate_universe);
	assert(!approximate_neighbors.approximation_quality.recall_measured);
	assert(approximate_neighbors.size() == 2);
	assert(approximate_neighbors[0].id == approximate_space.id(expected_plan.positions[0]));
	assert(approximate_neighbors[1].id == approximate_space.id(expected_plan.positions[1]));
	assert(*approximate_calls == expected_plan.size());
	assert(approximate_ctx.sample_plan_build_count() == 1);
	assert(approximate_ctx.has_cached_sample_plan());
	const auto cached_plan = approximate_ctx.cached_sample_plan();
	assert(cached_plan.has_value());
	assert(cached_plan->positions == expected_plan.positions);

	const auto approximate_cached_status = approximate_ctx.diagnostics();
	assert(approximate_cached_status.sample_plan_cached);
	assert(approximate_cached_status.sample_plan_build_count == 1);
	assert(approximate_cached_status.sample_plan_record_count == approximate_space.size());
	assert(approximate_cached_status.sample_plan_requested_count == expected_plan.requested_count);
	assert(approximate_cached_status.sample_plan_candidate_count == expected_plan.size());
	assert(approximate_cached_status.sample_plan_candidate_universe == expected_plan.candidate_universe);
	assert(approximate_cached_status.sample_plan_excludes_position);
	assert(approximate_cached_status.sample_plan_excluded_position == 2);

	const auto calls_after_approximate_neighbors = *approximate_calls;
	const auto approximate_range = approximate_ctx.range(approximate_query, 3);
	assert(!approximate_range.exact);
	assert(approximate_range.representation == "sampled_metric_space");
	assert(approximate_range.size() == 1);
	assert(approximate_range[0].id == approximate_space.id(expected_plan.positions[0]));
	assert(approximate_ctx.sample_plan_build_count() == 1);
	assert(*approximate_calls == calls_after_approximate_neighbors + expected_plan.size());

	const auto small_automatic_calls = std::make_shared<std::size_t>(0);
	auto small_automatic_space = mtrc::make_space(
		std::vector<int>{0, 2, 5, 9}, CountingAbsoluteDistance(small_automatic_calls));
	auto small_automatic_ctx = mtrc::space::execution_context(
		small_automatic_space, mtrc::space::storage::approximate());
	assert(!small_automatic_ctx.shared_provider());
	assert(small_automatic_ctx.provider_build_count() == 0);
	assert(small_automatic_ctx.provider_representation() == "sampled_metric_space");
	assert(*small_automatic_calls == 0);
	const auto small_automatic_status = small_automatic_ctx.diagnostics();
	assert(small_automatic_status.runtime.representation == "sampled_metric_space");
	assert(small_automatic_status.sample_plan_policy == "regular_sample");

	const auto automatic_calls = std::make_shared<std::size_t>(0);
	auto automatic_space = mtrc::make_space(
		approximate_records, CountingAbsoluteDistance(automatic_calls));
	auto automatic_ctx = mtrc::space::execution_context(
		automatic_space, mtrc::space::storage::approximate());
	const auto automatic_build_calls = *automatic_calls;
	assert(automatic_ctx.shared_provider());
	assert(automatic_ctx.provider_build_count() == 1);
	assert(automatic_ctx.provider_representation() == "landmark_index");
	assert(automatic_build_calls < automatic_space.size() * automatic_space.size() / 10);
	const auto automatic_initial_status = automatic_ctx.diagnostics();
	assert(automatic_initial_status.runtime.representation == "landmark_index");
	assert(automatic_initial_status.sample_planning_enabled);
	assert(automatic_initial_status.sample_plan_policy == "landmark_lower_bound_refinement");
	assert(!automatic_initial_status.sample_plan_cached);
	const auto automatic_landmark_count = mtrc::space::storage::landmark_index_landmark_count(
		automatic_space.size(), mtrc::space::storage::approximate());
	const auto automatic_candidate_limit = mtrc::space::storage::landmark_index_candidate_limit(
		automatic_space.size(), mtrc::space::storage::approximate(), true, 3);
	assert(automatic_initial_status.cached_distances ==
		   automatic_space.size() * automatic_landmark_count);

	const auto automatic_query = automatic_space.id(2048);
	const auto automatic_neighbors = automatic_ctx.neighbors(automatic_query, 3);
	const auto automatic_calls_after_neighbors = *automatic_calls;
	assert(!automatic_neighbors.exact);
	assert(automatic_neighbors.representation == "landmark_index");
	assert(automatic_neighbors.approximation_quality.candidate_policy == "landmark_lower_bound_refinement");
	assert(automatic_neighbors.approximation_quality.candidate_count == automatic_candidate_limit);
	assert(automatic_neighbors.size() == 3);
	assert(automatic_ctx.provider_build_count() == 1);
	assert(automatic_calls_after_neighbors - automatic_build_calls <=
		   automatic_landmark_count + automatic_candidate_limit);

	const auto chunked_calls = std::make_shared<std::size_t>(0);
	auto chunked_space = mtrc::make_space(
		std::vector<int>{0, 2, 5, 9}, CountingAbsoluteDistance(chunked_calls));
	auto chunked_policy = mtrc::space::storage::using_distance_table();
	chunked_policy = mtrc::space::storage::with_distance_table_budget(chunked_policy, 0, 1);
	chunked_policy = mtrc::space::storage::allow_chunking_fallback(chunked_policy);
	auto chunked_ctx = mtrc::space::execution_context(chunked_space, chunked_policy);
	assert(*chunked_calls == 0);
	assert(chunked_ctx.shared_provider());
	assert(chunked_ctx.provider_build_count() == 1);
	assert(chunked_ctx.provider_representation() == "blocked_distance_table");
	const auto chunked_status_before_queries = chunked_ctx.diagnostics();
	assert(chunked_status_before_queries.shared_provider);
	assert(chunked_status_before_queries.provider_representation == "blocked_distance_table");
	assert(chunked_status_before_queries.cached_distances == 0);
	assert(chunked_status_before_queries.dense_distance_slots == 0);
	assert(chunked_status_before_queries.runtime.budget_exceeded);
	assert(chunked_status_before_queries.runtime.downgraded);
	assert(!chunked_status_before_queries.runtime.refused);
	assert(chunked_status_before_queries.runtime.representation == "blocked_distance_table");

	const auto chunked_neighbors = chunked_ctx.neighbors(chunked_space.id(0), 2);
	assert(chunked_neighbors.representation == "blocked_distance_table");
	assert(chunked_neighbors.provenance.route_kind == mtrc::search_route_kind::pairwise_provider);
	assert(chunked_neighbors.provenance.domain_kind == mtrc::result_domain_kind::pairwise_provider);
	assert(chunked_neighbors.size() == 2);
	assert(chunked_neighbors[0].id == chunked_space.id(1));
	assert(*chunked_calls > 0);
	const auto calls_after_chunked_first_query = *chunked_calls;
	const auto chunked_status_after_query = chunked_ctx.diagnostics();
	assert(chunked_status_after_query.cached_distances > 0);
	assert(chunked_status_after_query.dense_distance_slots == 0);

	const auto repeated_chunked_neighbors = chunked_ctx.neighbors(chunked_space.id(0), 2);
	assert(repeated_chunked_neighbors.representation == "blocked_distance_table");
	assert(repeated_chunked_neighbors.size() == chunked_neighbors.size());
	assert(repeated_chunked_neighbors[0].id == chunked_neighbors[0].id);
	assert(*chunked_calls == calls_after_chunked_first_query);
	assert(chunked_ctx.diagnostics().dense_distance_slots == 0);

	const auto refusing_calls = std::make_shared<std::size_t>(0);
	auto refusing_space = mtrc::make_space(
		std::vector<int>{0, 2, 5, 9}, CountingAbsoluteDistance(refusing_calls));
	const auto refusing_policy = mtrc::space::storage::with_distance_table_budget(
		mtrc::space::storage::using_distance_table(), 1, 0);
	assert_representation_error([&] {
		(void)mtrc::space::execution_context(refusing_space, refusing_policy);
	}, "max_dense_records");
	assert(*refusing_calls == 0);

	auto assert_runtime_cancelled = [](auto operation, const std::shared_ptr<std::size_t> &calls,
									   std::size_t full_work_calls) {
		*calls = 0;
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
		assert(*calls < full_work_calls);
	};

	const auto context_runtime_calls = std::make_shared<std::size_t>(0);
	auto context_runtime_space = mtrc::make_space(
		std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7}, SlowCountingAbsoluteDistance(context_runtime_calls));
	const auto runtime_limited_matrix_policy =
		mtrc::space::storage::with_runtime_budget(mtrc::space::storage::using_distance_table(), 1);
	const auto context_runtime_slots =
		context_runtime_space.size() * (context_runtime_space.size() - 1) / 2;
	assert_runtime_cancelled([&] {
		(void)mtrc::space::execution_context(context_runtime_space, runtime_limited_matrix_policy);
	}, context_runtime_calls, context_runtime_slots);

	const auto live_runtime_calls = std::make_shared<std::size_t>(0);
	auto live_runtime_space = mtrc::make_space(
		std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7}, SlowCountingAbsoluteDistance(live_runtime_calls));
	auto live_runtime_ctx = mtrc::space::execution_context(
		live_runtime_space, mtrc::space::storage::with_runtime_budget(mtrc::space::storage::exact(), 1));
	assert_runtime_cancelled([&] {
		(void)live_runtime_ctx.neighbors(live_runtime_space.id(0), 2);
	}, live_runtime_calls, live_runtime_space.size() - 1);

	return 0;
}
