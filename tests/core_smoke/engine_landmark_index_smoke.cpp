#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "metric/engine.hpp"

struct CountingLineMetric {
	explicit CountingLineMetric(std::shared_ptr<std::size_t> calls)
		: calls(std::move(calls))
	{
	}

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		const auto delta = lhs - rhs;
		return delta < 0 ? -delta : delta;
	}

	std::shared_ptr<std::size_t> calls;
};

namespace mtrc::core {

template <> struct metric_traits<CountingLineMetric> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

} // namespace mtrc::core

static auto line_records(std::size_t count) -> std::vector<int>
{
	std::vector<int> records;
	records.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		records.push_back(static_cast<int>(index));
	}
	return records;
}

template <typename Result> static auto contains_id(const Result &result, mtrc::RecordId id) -> bool
{
	for (const auto &neighbor : result.neighbors) {
		if (neighbor.id == id) {
			return true;
		}
	}
	return false;
}

static auto test_landmark_policy_is_bounded_and_diagnostic() -> void
{
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(line_records(1000), CountingLineMetric(calls));
	const auto policy = mtrc::space::storage::using_landmark_index(32);

	const auto plan = mtrc::space::storage::estimate_cost(space, "neighbors", policy);
	assert(!plan.exact);
	assert(plan.representation == "landmark_index");
	assert(plan.estimated_distance_evaluations < space.size() * space.size() / 10);

	*calls = 0;
	const auto query_id = space.id(500);
	const auto neighbors = mtrc::find_neighbors(space, query_id, 4, policy);
	assert(!neighbors.exact);
	assert(neighbors.representation == "landmark_index");
	assert(neighbors.provenance.route_kind == mtrc::search_route_kind::approximate_sampled_candidates);
	assert(neighbors.provenance.domain_kind == mtrc::result_domain_kind::neighbor_index);
	assert(neighbors.approximation_quality.diagnostic == "landmark_search_approximation");
	assert(neighbors.approximation_quality.candidate_policy == "landmark_lower_bound_refinement");
	assert(neighbors.approximation_quality.candidate_count == 32);
	assert(neighbors.approximation_quality.candidate_universe == space.size() - 1);
	assert(neighbors.approximation_quality.recall_measured);
	assert(neighbors.approximation_quality.recall_sample_query_count ==
		   mtrc::space::storage::default_landmark_calibration_sample_queries);
	assert(neighbors.approximation_quality.recall_distance_evaluations <=
		   neighbors.approximation_quality.recall_sample_query_count * (space.size() - 1));
	assert(neighbors.approximation_quality.recall_reference_count >=
		   neighbors.approximation_quality.recall_matched_count);
	assert(neighbors.approximation_quality.recall >= 0.0);
	assert(neighbors.approximation_quality.recall <= 1.0);
	assert(neighbors.approximation_quality.standard_error >= 0.0);
	assert(neighbors.approximation_quality.confidence_radius_95 >= 0.0);
	assert(neighbors.approximation_quality.confidence_radius_95 <= 1.0);
	assert(neighbors.approximation_quality.reason.find("bounded holdout") != std::string::npos);
	assert(neighbors.size() == 4);
	assert(!contains_id(neighbors, query_id));
	assert(contains_id(neighbors, space.id(499)));
	assert(contains_id(neighbors, space.id(501)));
	assert(*calls < space.size() * space.size() / 10);

	*calls = 0;
	const auto external_neighbors = mtrc::find_neighbors(space, 500, 3, policy);
	assert(!external_neighbors.exact);
	assert(external_neighbors.representation == "landmark_index");
	assert(external_neighbors.approximation_quality.recall_measured);
	assert(external_neighbors.approximation_quality.recall_sample_query_count ==
		   mtrc::space::storage::default_landmark_calibration_sample_queries);
	assert(external_neighbors.size() == 3);
	assert(external_neighbors[0].id == query_id);
	assert(*calls < space.size() * space.size() / 10);

	*calls = 0;
	const auto range = mtrc::stats::search::range(space, query_id, 2, policy);
	assert(!range.exact);
	assert(range.representation == "landmark_index");
	assert(range.approximation_quality.candidate_count == 32);
	assert(range.approximation_quality.recall_measured);
	assert(range.approximation_quality.recall_sample_query_count ==
		   mtrc::space::storage::default_landmark_calibration_sample_queries);
	assert(range.approximation_quality.recall_distance_evaluations <=
		   range.approximation_quality.recall_sample_query_count * (space.size() - 1));
	assert(range.approximation_quality.standard_error >= 0.0);
	assert(range.approximation_quality.confidence_radius_95 >= 0.0);
	assert(range.approximation_quality.confidence_radius_95 <= 1.0);
	assert(range.size() == 4);
	assert(contains_id(range, space.id(498)));
	assert(contains_id(range, space.id(499)));
	assert(contains_id(range, space.id(501)));
	assert(contains_id(range, space.id(502)));
	assert(*calls < space.size() * space.size() / 10);
}

static auto test_execution_context_reuses_landmark_index() -> void
{
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(line_records(1000), CountingLineMetric(calls));
	const auto policy = mtrc::space::storage::using_landmark_index(16);

	*calls = 0;
	auto context = mtrc::space::execution_context(space, policy);
	const auto build_calls = *calls;
	assert(context.shared_provider());
	assert(context.provider_build_count() == 1);
	assert(context.provider_representation() == "landmark_index");
	assert(build_calls < space.size() * space.size() / 10);

	const auto status = context.diagnostics();
	assert(status.runtime.representation == "landmark_index");
	assert(status.sample_plan_policy == "landmark_lower_bound_refinement");
	assert(status.cached_distances == space.size() * mtrc::space::storage::default_landmark_index_landmarks);
	assert(status.memory_bytes_estimate > 0);

	const auto query_id = space.id(250);
	const auto first = context.neighbors(query_id, 3);
	const auto calls_after_first_query = *calls;
	const auto second = context.neighbors(query_id, 3);
	const auto calls_after_second_query = *calls;
	assert(first.representation == "landmark_index");
	assert(second.representation == "landmark_index");
	assert(context.provider_build_count() == 1);
	assert(calls_after_first_query - build_calls <=
		   mtrc::space::storage::default_landmark_index_landmarks + 16);
	assert(calls_after_second_query - calls_after_first_query <=
		   mtrc::space::storage::default_landmark_index_landmarks + 16);

	const auto range = context.range(query_id, 1);
	assert(!range.exact);
	assert(range.representation == "landmark_index");
	assert(range.approximation_quality.candidate_count == 16);
	assert(contains_id(range, space.id(249)));
	assert(contains_id(range, space.id(251)));
}

static auto test_landmark_calibration_is_budget_gated() -> void
{
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(line_records(1000), CountingLineMetric(calls));
	const auto policy = mtrc::space::storage::with_distance_evaluation_budget(
		mtrc::space::storage::using_landmark_index(32), 17000);

	*calls = 0;
	const auto neighbors = mtrc::find_neighbors(space, space.id(500), 4, policy);
	assert(!neighbors.exact);
	assert(neighbors.representation == "landmark_index");
	assert(neighbors.approximation_quality.candidate_count == 32);
	assert(!neighbors.approximation_quality.recall_measured);
	assert(neighbors.approximation_quality.recall_sample_query_count == 0);
	assert(neighbors.approximation_quality.recall_distance_evaluations == 0);
	assert(neighbors.approximation_quality.standard_error == 0.0);
	assert(neighbors.approximation_quality.confidence_radius_95 == 0.0);
	assert(neighbors.approximation_quality.reason.find("max_distance_evaluations") != std::string::npos);
	assert(*calls < space.size() * space.size() / 10);
}

static auto test_landmark_refresh_after_streaming_append_is_bounded() -> void
{
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(line_records(200), CountingLineMetric(calls));
	auto index = mtrc::space::storage::landmark_index(space, 8, 32);
	assert(index.landmark_count() == 8);
	assert(!index.is_stale());

	*calls = 0;
	const auto append = mtrc::space::append_batch(space, std::vector<int>{200, 201, 202, 203, 204});
	assert(append.appended == 5);
	assert(append.distance_evaluations == 0);
	assert(*calls == 0);
	assert(index.is_stale());

	const auto refresh = mtrc::space::refresh_after_append(index, append);
	assert(refresh.refreshed);
	assert(!refresh.rebuild_required);
	assert(refresh.appended == append.appended);
	assert(refresh.distance_evaluations == append.appended * refresh.landmark_count);
	assert(refresh.distance_evaluations == 5 * 8);
	assert(*calls == refresh.distance_evaluations);
	assert(!index.is_stale());
	assert(index.record_count() == space.size());
	assert(index.version() == space.version());
	assert(index.maintenance_distance_evaluations() == refresh.distance_evaluations);
	assert(index.contains(append.appended_ids.back()));
	assert(index.source_record_ids().back() == append.appended_ids.back());
	assert(refresh.distance_evaluations < space.size() * space.size() / 10);

	*calls = 0;
	const auto appended_query = append.appended_ids.back();
	const auto neighbors = index.knn(appended_query, 3, 32);
	assert(neighbors.size() == 3);
	assert(*calls <= index.landmark_count() + 32);

	*calls = 0;
	space.replace(space.id(0), -100);
	const auto refused = index.refresh_after_append();
	assert(!refused.refreshed);
	assert(refused.rebuild_required);
	assert(refused.reason.find("rebuild required") != std::string::npos);
	assert(*calls == 0);
}

int main()
{
	test_landmark_policy_is_bounded_and_diagnostic();
	test_execution_context_reuses_landmark_index();
	test_landmark_calibration_is_budget_gated();
	test_landmark_refresh_after_streaming_append_is_bounded();
	return 0;
}
