#include <cassert>
#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

#include "metric/benchmarks/counting_metric.hpp"
#include "metric/engine.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

} // namespace

namespace mtrc::core {

template <> struct metric_traits<mtrc::benchmarks::CountingMetric<AbsoluteDistance>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

} // namespace mtrc::core

namespace {

auto make_integer_records(std::size_t count) -> std::vector<int>
{
	std::vector<int> records;
	records.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		records.push_back(static_cast<int>(index));
	}
	return records;
}

} // namespace

int main()
{
	const std::vector<int> records{0, 2, 5, 9};
	auto space = mtrc::make_space(records, AbsoluteDistance{});

	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);
	mtrc::space::storage::KnnGraphIndex<decltype(space)> graph(space, 2);

	mtrc::benchmarks::BenchmarkReport report("METRIC Engine Benchmark Report");
	report.set_run_metadata({"smoke suite", "tests/core_smoke/benchmark_report_smoke.cpp", "core preset",
							 "portable deterministic fixture", "CTest benchmark_report_smoke", "shape contract only"});
	report.add_representation_cost(mtrc::benchmarks::representation_cost_row(
		"integer distance table", matrix.diagnostics(), "eager all-pairs materialization"));
	report.add_representation_cost(
		mtrc::benchmarks::representation_cost_row("integer kNN graph", graph.diagnostics(), "sparse neighbor index"));

	mtrc::benchmarks::WorkflowEvidenceRow string_hero;
	string_hero.benchmark = "string edit baseline";
	string_hero.query_count = 4;
	string_hero.metric_matches = 4;
	string_hero.vector_mismatches = 4;
	string_hero.average_metric_margin = 2.0;
	string_hero.reports_average_metric_margin = true;
	string_hero.diagnostics = "edit-distance-vs-anagram-vector";
	report.add_workflow_evidence(string_hero);

	mtrc::benchmarks::WorkflowEvidenceRow diffusion_coordinate_hero;
	diffusion_coordinate_hero.benchmark = "process curve diffusion-coordinate gallery";
	diffusion_coordinate_hero.query_count = 6;
	diffusion_coordinate_hero.metric_matches = 6;
	diffusion_coordinate_hero.vector_mismatches = 6;
	diffusion_coordinate_hero.latent_matches = 6;
	diffusion_coordinate_hero.reports_latent_matches = true;
	diffusion_coordinate_hero.average_metric_margin = 2.16667;
	diffusion_coordinate_hero.reports_average_metric_margin = true;
	diffusion_coordinate_hero.diagnostics = "OOS rank and distance penalty";
	report.add_workflow_evidence(diffusion_coordinate_hero);

	mtrc::benchmarks::PerformanceRow amortization;
	amortization.benchmark = "integer amortization";
	amortization.workload = "shared gallery operators";
	amortization.record_count = 4;
	amortization.shared_operations = 3;
	amortization.naive_distance_evaluations = 36;
	amortization.materialized_distance_evaluations = 16;
	amortization.cache_miss_evaluations = 0;
	amortization.naive_memory_bytes_estimate = 0;
	amortization.materialized_memory_bytes_estimate = matrix.memory_bytes_estimate();
	amortization.evaluation_reduction_factor = 2.25;
	amortization.exact_match = true;
	amortization.notes = "build once, reuse free";
	report.add_performance_row(amortization);

	mtrc::benchmarks::WallClockTrendRow wall_clock_smoke;
	wall_clock_smoke.benchmark = "synthetic wall-clock smoke";
	wall_clock_smoke.workload = "synthetic recorded metadata sample";
	wall_clock_smoke.record_count = records.size();
	wall_clock_smoke.representation = "distance_table";
	wall_clock_smoke.elapsed_ns = 1;
	wall_clock_smoke.distance_evaluations = matrix.diagnostics().distance_evaluations;
	wall_clock_smoke.memory_bytes_estimate = matrix.memory_bytes_estimate();
	wall_clock_smoke.sample_count = 1;
	wall_clock_smoke.platform_label = "synthetic-smoke";
	wall_clock_smoke.automated = true;
	wall_clock_smoke.passed = wall_clock_smoke.elapsed_ns > 0;
	wall_clock_smoke.notes = "synthetic smoke metadata; no wall-clock threshold or regression claim";
	report.add_wall_clock_trend(wall_clock_smoke);

	constexpr std::size_t measured_wall_clock_record_count = 32;
	std::size_t measured_wall_clock_elapsed_ns = 0;
	std::size_t measured_wall_clock_evaluations = 0;
	std::size_t measured_wall_clock_memory_bytes = 0;
	for (std::size_t attempt = 0; attempt < 8 && measured_wall_clock_elapsed_ns == 0; ++attempt) {
		const auto measured_records = make_integer_records(measured_wall_clock_record_count);
		auto measured_metric = mtrc::benchmarks::make_counting_metric(AbsoluteDistance{});
		const auto measured_counter = measured_metric.counter();
		auto measured_space = mtrc::make_space(measured_records, measured_metric);
		const auto started_at = std::chrono::steady_clock::now();
		mtrc::space::storage::DistanceTable<decltype(measured_space)> measured_table(measured_space);
		const auto finished_at = std::chrono::steady_clock::now();
		const auto elapsed_ns =
			std::chrono::duration_cast<std::chrono::nanoseconds>(finished_at - started_at).count();
		assert(elapsed_ns >= 0);
		measured_wall_clock_elapsed_ns = static_cast<std::size_t>(elapsed_ns);
		measured_wall_clock_evaluations = *measured_counter;
		measured_wall_clock_memory_bytes = measured_table.memory_bytes_estimate();
	}
	assert(measured_wall_clock_elapsed_ns > 0);
	assert(measured_wall_clock_evaluations == measured_wall_clock_record_count * measured_wall_clock_record_count);

	mtrc::benchmarks::WallClockTrendRow measured_wall_clock;
	measured_wall_clock.benchmark = "measured wall-clock smoke";
	measured_wall_clock.workload = "small distance-table build";
	measured_wall_clock.record_count = measured_wall_clock_record_count;
	measured_wall_clock.representation = "distance_table";
	measured_wall_clock.elapsed_ns = measured_wall_clock_elapsed_ns;
	measured_wall_clock.distance_evaluations = measured_wall_clock_evaluations;
	measured_wall_clock.memory_bytes_estimate = measured_wall_clock_memory_bytes;
	measured_wall_clock.sample_count = 1;
	measured_wall_clock.platform_label = "steady_clock-smoke";
	measured_wall_clock.automated = true;
	measured_wall_clock.passed = measured_wall_clock.elapsed_ns > 0;
	measured_wall_clock.notes = "measured with std::chrono; CTest asserts only nonzero elapsed and report fields";
	report.add_wall_clock_trend(measured_wall_clock);

	constexpr std::size_t exact_trend_record_count = 32;
	const auto exact_trend_records = make_integer_records(exact_trend_record_count);
	{
		auto oneoff_metric = mtrc::benchmarks::make_counting_metric(AbsoluteDistance{});
		const auto oneoff_counter = oneoff_metric.counter();
		auto oneoff_space = mtrc::make_space(exact_trend_records, oneoff_metric);
		const auto oneoff_neighbors = mtrc::find_neighbors(oneoff_space, oneoff_space.id(0), 4);
		constexpr std::size_t expected_oneoff_evaluations = exact_trend_record_count - 1;
		assert(oneoff_neighbors.exact);
		assert(oneoff_neighbors.representation == "metric_space");
		assert(oneoff_neighbors.size() == 4);
		assert(*oneoff_counter == expected_oneoff_evaluations);

		mtrc::benchmarks::BenchmarkTrendRow oneoff_trend;
		oneoff_trend.benchmark = "one-off kNN exact trend";
		oneoff_trend.workload = "single query over integer smoke space";
		oneoff_trend.record_count = exact_trend_record_count;
		oneoff_trend.query_count = 1;
		oneoff_trend.baseline_distance_evaluations = expected_oneoff_evaluations;
		oneoff_trend.observed_distance_evaluations = *oneoff_counter;
		oneoff_trend.representation = oneoff_neighbors.representation;
		oneoff_trend.exact = oneoff_neighbors.exact;
		oneoff_trend.passed = oneoff_trend.observed_distance_evaluations == oneoff_trend.baseline_distance_evaluations;
		oneoff_trend.notes = "CountingMetric validates self-excluding exact scan";
		report.add_benchmark_trend(oneoff_trend);
	}

	constexpr std::size_t exact_batch_query_count = 5;
	{
		auto batch_metric = mtrc::benchmarks::make_counting_metric(AbsoluteDistance{});
		const auto batch_counter = batch_metric.counter();
		auto batch_space = mtrc::make_space(exact_trend_records, batch_metric);
		std::vector<mtrc::RecordId> query_ids;
		query_ids.reserve(exact_batch_query_count);
		for (std::size_t index = 0; index < exact_batch_query_count; ++index) {
			query_ids.push_back(batch_space.id(index));
		}
		const auto batch_neighbors = mtrc::stats::search::knn_batch(batch_space, query_ids, 3);
		const auto expected_batch_evaluations = exact_batch_query_count * (exact_trend_record_count - 1);
		assert(batch_neighbors.size() == exact_batch_query_count);
		for (const auto &neighbors : batch_neighbors) {
			assert(neighbors.exact);
			assert(neighbors.representation == "metric_space");
			assert(neighbors.size() == 3);
		}
		assert(*batch_counter == expected_batch_evaluations);

		mtrc::benchmarks::BenchmarkTrendRow batch_trend;
		batch_trend.benchmark = "batch kNN exact trend";
		batch_trend.workload = "five query-id kNN calls through batch API";
		batch_trend.record_count = exact_trend_record_count;
		batch_trend.query_count = exact_batch_query_count;
		batch_trend.baseline_distance_evaluations = expected_batch_evaluations;
		batch_trend.observed_distance_evaluations = *batch_counter;
		batch_trend.representation = "metric_space";
		batch_trend.exact = true;
		batch_trend.passed = batch_trend.observed_distance_evaluations == batch_trend.baseline_distance_evaluations;
		batch_trend.notes = "CountingMetric validates native batch query accounting";
		report.add_benchmark_trend(batch_trend);
	}

	constexpr std::size_t provider_reuse_record_count = 16;
	constexpr std::size_t provider_reuse_query_count = 10;
	{
		const auto provider_records = make_integer_records(provider_reuse_record_count);
		auto provider_metric = mtrc::benchmarks::make_counting_metric(AbsoluteDistance{});
		const auto provider_counter = provider_metric.counter();
		auto provider_space = mtrc::make_space(provider_records, provider_metric);
		std::vector<mtrc::RecordId> provider_query_ids;
		provider_query_ids.reserve(provider_reuse_query_count);
		for (std::size_t index = 0; index < provider_reuse_query_count; ++index) {
			provider_query_ids.push_back(provider_space.id(index));
		}

		provider_metric.reset();
		for (const auto query_id : provider_query_ids) {
			(void)mtrc::find_neighbors(provider_space, query_id, 3);
		}
		(void)mtrc::distance_distribution(provider_space);
		(void)mtrc::distance_distribution(provider_space);
		const auto naive_reuse_evaluations = *provider_counter;

		provider_metric.reset();
		mtrc::space::storage::DistanceTable<decltype(provider_space)> provider_table(provider_space);
		for (const auto query_id : provider_query_ids) {
			(void)mtrc::stats::search::knn(provider_table, query_id, 3);
		}
		(void)mtrc::distance_distribution(provider_table);
		(void)mtrc::distance_distribution(provider_table);
		const auto materialized_reuse_evaluations = *provider_counter;
		assert(naive_reuse_evaluations == provider_reuse_query_count * (provider_reuse_record_count - 1) +
											  provider_reuse_record_count * (provider_reuse_record_count - 1));
		assert(materialized_reuse_evaluations == provider_reuse_record_count * provider_reuse_record_count);
		assert(materialized_reuse_evaluations < naive_reuse_evaluations);

		mtrc::benchmarks::BenchmarkTargetRow provider_target;
		provider_target.benchmark = "repeated provider workflow target";
		provider_target.workload = "shared kNN plus two distribution passes";
		provider_target.record_count = provider_reuse_record_count;
		provider_target.query_count = provider_reuse_query_count;
		provider_target.target = "materialize once and keep observed metric calls below naive repeated workflow";
		provider_target.max_distance_evaluations = provider_reuse_record_count * provider_reuse_record_count;
		provider_target.max_dense_slots = provider_table.dense_distance_slots();
		provider_target.expected_representation = "distance_table";
		provider_target.exact = true;
		provider_target.automated = true;
		provider_target.notes = "CountingMetric guards provider reuse accounting";
		report.add_benchmark_target(provider_target);

		mtrc::benchmarks::BenchmarkTrendRow provider_trend;
		provider_trend.benchmark = "repeated provider workflow trend";
		provider_trend.workload = "shared kNN plus two distribution passes";
		provider_trend.record_count = provider_reuse_record_count;
		provider_trend.query_count = provider_reuse_query_count;
		provider_trend.baseline_distance_evaluations = naive_reuse_evaluations;
		provider_trend.observed_distance_evaluations = materialized_reuse_evaluations;
		provider_trend.cached_distances = provider_table.cached_distances();
		provider_trend.dense_distance_slots = provider_table.dense_distance_slots();
		provider_trend.representation = "distance_table";
		provider_trend.exact = true;
		provider_trend.passed = materialized_reuse_evaluations < naive_reuse_evaluations;
		provider_trend.notes = "cached provider serves all post-build reads without metric calls";
		report.add_benchmark_trend(provider_trend);
	}

	constexpr std::size_t lazy_cache_record_count = 12;
	{
		const auto cache_records = make_integer_records(lazy_cache_record_count);
		auto cache_metric = mtrc::benchmarks::make_counting_metric(AbsoluteDistance{});
		const auto cache_counter = cache_metric.counter();
		auto cache_space = mtrc::make_space(cache_records, cache_metric);
		mtrc::space::storage::DistanceTable<decltype(cache_space)> lazy_table(
			cache_space, mtrc::space::storage::distance_table_mode::lazy);
		const auto lhs = cache_space.id(1);
		const auto rhs = cache_space.id(7);
		assert(*cache_counter == 0);
		assert(lazy_table.cached_distances() == 0);
		const auto first_distance = lazy_table.distance(lhs, rhs);
		const auto second_distance = lazy_table.distance(lhs, rhs);
		assert(first_distance == second_distance);
		assert(*cache_counter == 1);
		assert(lazy_table.cached_distances() == 1);
		const auto cache_stats = lazy_table.stats();
		assert(cache_stats.misses == 1);
		assert(cache_stats.hits == 1);

		mtrc::benchmarks::BenchmarkTargetRow cache_target;
		cache_target.benchmark = "lazy sparse cache target";
		cache_target.workload = "repeat one pair lookup";
		cache_target.record_count = lazy_cache_record_count;
		cache_target.query_count = 2;
		cache_target.target = "second lookup is a cache hit";
		cache_target.max_distance_evaluations = 1;
		cache_target.max_dense_slots = lazy_table.dense_distance_slots();
		cache_target.expected_representation = "distance_table";
		cache_target.exact = true;
		cache_target.automated = true;
		cache_target.notes = "lazy table keeps sparse cache instead of dense materialization";
		report.add_benchmark_target(cache_target);

		mtrc::benchmarks::BenchmarkTrendRow cache_trend;
		cache_trend.benchmark = "lazy sparse cache trend";
		cache_trend.workload = "repeat one pair lookup";
		cache_trend.record_count = lazy_cache_record_count;
		cache_trend.query_count = 2;
		cache_trend.baseline_distance_evaluations = 2;
		cache_trend.observed_distance_evaluations = *cache_counter;
		cache_trend.cached_distances = lazy_table.cached_distances();
		cache_trend.dense_distance_slots = lazy_table.dense_distance_slots();
		cache_trend.representation = "distance_table";
		cache_trend.exact = true;
		cache_trend.passed = cache_stats.misses == 1 && cache_stats.hits == 1 && *cache_counter == 1;
		cache_trend.notes = "CountingMetric separates sparse miss from repeated hit";
		report.add_benchmark_trend(cache_trend);
	}

	constexpr std::size_t large_record_count = 5000;
	constexpr std::size_t shared_workflow_operations = 3;
	constexpr std::size_t sampled_search_candidate_evaluations = 4096;
	constexpr std::size_t sampled_batch_query_count = 8;
	constexpr std::size_t sampled_batch_candidate_evaluations =
		sampled_search_candidate_evaluations * sampled_batch_query_count;
	constexpr std::size_t sampled_distribution_pair_evaluations = 4096;
	constexpr std::size_t sampled_describe_pair_evaluations = 512;
	constexpr std::size_t sampled_describe_intrinsic_centers = 4;
	constexpr std::size_t sampled_describe_expected_evaluations =
		sampled_describe_pair_evaluations + sampled_describe_intrinsic_centers * large_record_count;
	constexpr std::size_t exact_distribution_pair_evaluations =
		large_record_count * (large_record_count - 1) / 2;
	const auto large_records = make_integer_records(large_record_count);

	auto counting_metric = mtrc::benchmarks::make_counting_metric(AbsoluteDistance{});
	const auto distance_evaluation_counter = counting_metric.counter();
	auto large_space = mtrc::make_space(large_records, counting_metric);

	const auto exact_dense_plan =
		mtrc::space::storage::estimate_cost(large_space, "neighbors", mtrc::space::storage::using_distance_table());
	assert(exact_dense_plan.estimated_distance_evaluations == large_record_count * large_record_count);
	assert(exact_dense_plan.dense_distance_slots == large_record_count * large_record_count);
	assert(*distance_evaluation_counter == 0);

	constexpr std::size_t out_of_core_memory_budget = 64 * 1024;
	constexpr std::size_t out_of_core_spill_block_bytes = 1024 * 1024;
	counting_metric.reset();
	mtrc::space::storage::resource_budget spill_refusal_budget;
	spill_refusal_budget.max_memory_bytes = out_of_core_memory_budget;
	spill_refusal_budget.allow_approximate = false;
	spill_refusal_budget.allow_chunking = false;
	const auto spill_refusal_policy = mtrc::space::storage::with_resource_budget(
		mtrc::space::storage::using_distance_table(), spill_refusal_budget);
	const auto spill_refusal_plan = mtrc::space::storage::estimate_cost(large_space, "neighbors", spill_refusal_policy);
	assert(spill_refusal_plan.refused);
	assert(spill_refusal_plan.budget_exceeded);
	assert(spill_refusal_plan.decision == mtrc::space::storage::budget_decision::refused);
	assert(spill_refusal_plan.memory_bytes_estimate == exact_dense_plan.memory_bytes_estimate);
	assert(spill_refusal_plan.memory_bytes_estimate > out_of_core_memory_budget);
	assert(*distance_evaluation_counter == 0);

	mtrc::benchmarks::OutOfCoreReadinessRow spill_readiness;
	spill_readiness.benchmark = "out-of-core spill readiness";
	spill_readiness.workload = "explicit dense materialization with spill disabled";
	spill_readiness.record_count = large_record_count;
	spill_readiness.dense_distance_slots = exact_dense_plan.dense_distance_slots;
	spill_readiness.dense_memory_bytes_estimate = spill_refusal_plan.memory_bytes_estimate;
	spill_readiness.memory_budget_bytes = out_of_core_memory_budget;
	spill_readiness.spill_block_bytes = out_of_core_spill_block_bytes;
	spill_readiness.planned_spill_blocks =
		(spill_refusal_plan.memory_bytes_estimate + out_of_core_spill_block_bytes - 1) /
		out_of_core_spill_block_bytes;
	spill_readiness.max_resident_blocks = 0;
	spill_readiness.spill_enabled = false;
	spill_readiness.explicit_policy_required = true;
	spill_readiness.decision = mtrc::space::storage::budget_decision_name(spill_refusal_plan.decision);
	spill_readiness.automated = true;
	spill_readiness.passed = spill_refusal_plan.refused && *distance_evaluation_counter == 0;
	spill_readiness.notes = "spill disabled: dense plan refuses before metric calls";
	report.add_out_of_core_readiness(spill_readiness);

	constexpr std::size_t landmark_candidate_limit = 64;
	counting_metric.reset();
	const auto landmark_policy = mtrc::space::storage::using_landmark_index(landmark_candidate_limit);
	const auto landmark_plan = mtrc::space::storage::estimate_cost(large_space, "neighbors", landmark_policy);
	assert(!landmark_plan.exact);
	assert(landmark_plan.representation == "landmark_index");
	assert(landmark_plan.estimated_distance_evaluations < exact_dense_plan.estimated_distance_evaluations);
	const auto landmark_neighbors =
		mtrc::find_neighbors(large_space, large_space.id(large_record_count / 2), 4, landmark_policy);
	const auto landmark_observed_evaluations = *distance_evaluation_counter;
	assert(!landmark_neighbors.exact);
	assert(landmark_neighbors.representation == "landmark_index");
	assert(landmark_neighbors.approximation_quality.candidate_count == landmark_candidate_limit);
	assert(landmark_observed_evaluations <= landmark_plan.estimated_distance_evaluations);
	assert(landmark_observed_evaluations < exact_dense_plan.estimated_distance_evaluations);

	mtrc::benchmarks::BenchmarkTargetRow landmark_target;
	landmark_target.benchmark = "landmark kNN target";
	landmark_target.workload = "large metric query via landmark lower-bound candidates";
	landmark_target.record_count = large_record_count;
	landmark_target.query_count = 1;
	landmark_target.target = "landmark build plus bounded exact refinement stays below dense all-pairs";
	landmark_target.max_distance_evaluations = landmark_plan.estimated_distance_evaluations;
	landmark_target.max_dense_slots = exact_dense_plan.dense_distance_slots;
	landmark_target.expected_representation = "landmark_index";
	landmark_target.exact = false;
	landmark_target.automated = true;
	landmark_target.notes = "CountingMetric covers O(n*p) build plus bounded candidate refinement";
	report.add_benchmark_target(landmark_target);

	mtrc::benchmarks::BenchmarkTrendRow landmark_trend;
	landmark_trend.benchmark = "landmark kNN trend";
	landmark_trend.workload = "large metric query via landmark lower-bound candidates";
	landmark_trend.record_count = large_record_count;
	landmark_trend.query_count = 1;
	landmark_trend.baseline_distance_evaluations = exact_dense_plan.estimated_distance_evaluations;
	landmark_trend.observed_distance_evaluations = landmark_observed_evaluations;
	landmark_trend.cached_distances = large_record_count * mtrc::space::storage::default_landmark_index_landmarks;
	landmark_trend.dense_distance_slots = exact_dense_plan.dense_distance_slots;
	landmark_trend.representation = "landmark_index";
	landmark_trend.exact = false;
	landmark_trend.passed = landmark_observed_evaluations <= landmark_plan.estimated_distance_evaluations &&
							 landmark_observed_evaluations < exact_dense_plan.estimated_distance_evaluations;
	landmark_trend.notes = "explicit landmark policy avoids dense all-pairs work for metric search";
	report.add_benchmark_trend(landmark_trend);

	counting_metric.reset();
	mtrc::space::storage::resource_budget guardrail_budget;
	guardrail_budget.max_dense_records = 1024;
	guardrail_budget.max_distance_evaluations = sampled_search_candidate_evaluations;
	guardrail_budget.allow_approximate = true;
	const auto guarded_policy = mtrc::space::storage::with_resource_budget(
		mtrc::space::storage::using_distance_table(), guardrail_budget);
	const auto guarded_plan = mtrc::space::storage::estimate_cost(large_space, "neighbors", guarded_policy);
	assert(guarded_plan.budget_exceeded);
	assert(guarded_plan.allowed);
	assert(guarded_plan.downgraded);
	assert(!guarded_plan.refused);
	assert(!guarded_plan.exact);
	assert(guarded_plan.representation == "sampled_metric_space");
	assert(*distance_evaluation_counter == 0);

	mtrc::benchmarks::BenchmarkTargetRow oneoff_target;
	oneoff_target.benchmark = "one-off kNN guardrail target";
	oneoff_target.workload = "large exact materialized request downgraded to sampled candidates";
	oneoff_target.record_count = large_record_count;
	oneoff_target.query_count = 1;
	oneoff_target.target = "bounded sampled candidate count";
	oneoff_target.max_distance_evaluations = sampled_search_candidate_evaluations;
	oneoff_target.max_dense_slots = exact_dense_plan.dense_distance_slots;
	oneoff_target.expected_representation = "sampled_metric_space";
	oneoff_target.exact = false;
	oneoff_target.automated = true;
	oneoff_target.notes = "CTest smoke executes the sampled route with CountingMetric";
	report.add_benchmark_target(oneoff_target);

	const auto guarded_batch_plan =
		mtrc::space::storage::estimate_cost(large_space, "neighbors", sampled_batch_query_count, guarded_policy);
	assert(guarded_batch_plan.budget_exceeded);
	assert(guarded_batch_plan.allowed);
	assert(guarded_batch_plan.downgraded);
	assert(!guarded_batch_plan.exact);
	assert(guarded_batch_plan.query_count == sampled_batch_query_count);
	assert(guarded_batch_plan.representation == "sampled_metric_space");

	mtrc::benchmarks::BenchmarkTargetRow batch_target;
	batch_target.benchmark = "batch kNN guardrail target";
	batch_target.workload = "large query batch downgraded to sampled candidates";
	batch_target.record_count = large_record_count;
	batch_target.query_count = sampled_batch_query_count;
	batch_target.target = "bounded sampled candidate count per query";
	batch_target.max_distance_evaluations = sampled_batch_candidate_evaluations;
	batch_target.max_dense_slots = exact_dense_plan.dense_distance_slots;
	batch_target.expected_representation = "sampled_metric_space";
	batch_target.exact = false;
	batch_target.automated = true;
	batch_target.notes = "CTest smoke runs the batch as repeated bounded query-id requests";
	report.add_benchmark_target(batch_target);

	const auto large_neighbors = mtrc::find_neighbors(large_space, large_space.id(0), 4, guarded_policy);
	assert(!large_neighbors.exact);
	assert(large_neighbors.representation == "sampled_metric_space");
	assert(large_neighbors.approximation_quality.candidate_count == sampled_search_candidate_evaluations);
	assert(large_neighbors.approximation_quality.candidate_universe == large_record_count - 1);
	assert(large_neighbors.approximation_quality.distance_evaluations == sampled_search_candidate_evaluations);
	assert(!large_neighbors.approximation_quality.recall_measured);
	assert(*distance_evaluation_counter == sampled_search_candidate_evaluations);
	assert(*distance_evaluation_counter < exact_dense_plan.estimated_distance_evaluations);
	assert(*distance_evaluation_counter <
		   exact_dense_plan.estimated_distance_evaluations * shared_workflow_operations);

	mtrc::benchmarks::ScaleGuardrailRow large_query_guardrail;
	large_query_guardrail.benchmark = "large sampled neighbor guardrail";
	large_query_guardrail.workload = "materialized exact request downgraded before dense build";
	large_query_guardrail.record_count = large_record_count;
	large_query_guardrail.shared_operations = shared_workflow_operations;
	large_query_guardrail.exact_plan_distance_evaluations = exact_dense_plan.estimated_distance_evaluations;
	large_query_guardrail.naive_workflow_distance_evaluations =
		exact_dense_plan.estimated_distance_evaluations * shared_workflow_operations;
	large_query_guardrail.dense_distance_slots = exact_dense_plan.dense_distance_slots;
	large_query_guardrail.max_distance_evaluations = guardrail_budget.max_distance_evaluations;
	large_query_guardrail.expected_distance_evaluations = sampled_search_candidate_evaluations;
	large_query_guardrail.observed_distance_evaluations = *distance_evaluation_counter;
	large_query_guardrail.decision = mtrc::space::storage::budget_decision_name(guarded_plan.decision);
	large_query_guardrail.representation = large_neighbors.representation;
	large_query_guardrail.exact = large_neighbors.exact;
	large_query_guardrail.passed =
		large_query_guardrail.observed_distance_evaluations == large_query_guardrail.expected_distance_evaluations;
	large_query_guardrail.notes = "counting metric proves the bounded sampled route, not dense all-pairs";
	report.add_scale_guardrail(large_query_guardrail);

	mtrc::benchmarks::BenchmarkTrendRow oneoff_sampled_trend;
	oneoff_sampled_trend.benchmark = "one-off kNN sampled trend";
	oneoff_sampled_trend.workload = "large exact materialized request downgraded to sampled candidates";
	oneoff_sampled_trend.record_count = large_record_count;
	oneoff_sampled_trend.query_count = 1;
	oneoff_sampled_trend.baseline_distance_evaluations = exact_dense_plan.estimated_distance_evaluations;
	oneoff_sampled_trend.observed_distance_evaluations = *distance_evaluation_counter;
	oneoff_sampled_trend.dense_distance_slots = exact_dense_plan.dense_distance_slots;
	oneoff_sampled_trend.representation = large_neighbors.representation;
	oneoff_sampled_trend.exact = large_neighbors.exact;
	oneoff_sampled_trend.passed =
		oneoff_sampled_trend.observed_distance_evaluations == sampled_search_candidate_evaluations;
	oneoff_sampled_trend.notes = "sampled query stays below dense exact plan";
	report.add_benchmark_trend(oneoff_sampled_trend);

	counting_metric.reset();
	assert(*distance_evaluation_counter == 0);
	for (std::size_t query_index = 0; query_index < sampled_batch_query_count; ++query_index) {
		const auto batch_neighbors = mtrc::find_neighbors(large_space, large_space.id(query_index), 4, guarded_policy);
		assert(!batch_neighbors.exact);
		assert(batch_neighbors.representation == "sampled_metric_space");
		assert(batch_neighbors.approximation_quality.candidate_count == sampled_search_candidate_evaluations);
		assert(batch_neighbors.approximation_quality.distance_evaluations == sampled_search_candidate_evaluations);
	}
	assert(*distance_evaluation_counter == sampled_batch_candidate_evaluations);
	assert(*distance_evaluation_counter <
		   exact_dense_plan.estimated_distance_evaluations * sampled_batch_query_count);

	mtrc::benchmarks::ScaleGuardrailRow batch_query_guardrail;
	batch_query_guardrail.benchmark = "large sampled batch neighbor guardrail";
	batch_query_guardrail.workload = "query batch downgraded before dense build";
	batch_query_guardrail.record_count = large_record_count;
	batch_query_guardrail.shared_operations = sampled_batch_query_count;
	batch_query_guardrail.exact_plan_distance_evaluations = exact_dense_plan.estimated_distance_evaluations;
	batch_query_guardrail.naive_workflow_distance_evaluations =
		exact_dense_plan.estimated_distance_evaluations * sampled_batch_query_count;
	batch_query_guardrail.dense_distance_slots = exact_dense_plan.dense_distance_slots;
	batch_query_guardrail.max_distance_evaluations = sampled_batch_candidate_evaluations;
	batch_query_guardrail.expected_distance_evaluations = sampled_batch_candidate_evaluations;
	batch_query_guardrail.observed_distance_evaluations = *distance_evaluation_counter;
	batch_query_guardrail.decision = mtrc::space::storage::budget_decision_name(guarded_batch_plan.decision);
	batch_query_guardrail.representation = guarded_batch_plan.representation;
	batch_query_guardrail.exact = guarded_batch_plan.exact;
	batch_query_guardrail.passed =
		batch_query_guardrail.observed_distance_evaluations == batch_query_guardrail.expected_distance_evaluations;
	batch_query_guardrail.notes = "CountingMetric proves per-query sampled budget scales linearly with query count";
	report.add_scale_guardrail(batch_query_guardrail);

	mtrc::benchmarks::BenchmarkTrendRow sampled_batch_trend;
	sampled_batch_trend.benchmark = "batch kNN sampled trend";
	sampled_batch_trend.workload = "query batch downgraded before dense build";
	sampled_batch_trend.record_count = large_record_count;
	sampled_batch_trend.query_count = sampled_batch_query_count;
	sampled_batch_trend.baseline_distance_evaluations =
		exact_dense_plan.estimated_distance_evaluations * sampled_batch_query_count;
	sampled_batch_trend.observed_distance_evaluations = *distance_evaluation_counter;
	sampled_batch_trend.dense_distance_slots = exact_dense_plan.dense_distance_slots;
	sampled_batch_trend.representation = guarded_batch_plan.representation;
	sampled_batch_trend.exact = guarded_batch_plan.exact;
	sampled_batch_trend.passed =
		sampled_batch_trend.observed_distance_evaluations == sampled_batch_candidate_evaluations;
	sampled_batch_trend.notes = "sampled batch row remains CTest-sized";
	report.add_benchmark_trend(sampled_batch_trend);

	counting_metric.reset();
	assert(*distance_evaluation_counter == 0);

	const auto large_distribution = mtrc::distance_distribution(large_space);
	assert(large_distribution.record_count == large_record_count);
	assert(large_distribution.pair_count == exact_distribution_pair_evaluations);
	assert(large_distribution.evaluated_pair_count == sampled_distribution_pair_evaluations);
	assert(large_distribution.sample_count == sampled_distribution_pair_evaluations);
	assert(!large_distribution.exact);
	assert(large_distribution.algorithm == "sampled_distance_distribution");
	assert(large_distribution.representation == "metric_space");
	assert(large_distribution.approximation_quality.diagnostic == "distance_distribution_approximation");
	assert(large_distribution.approximation_quality.candidate_count == sampled_distribution_pair_evaluations);
	assert(large_distribution.approximation_quality.candidate_universe == exact_distribution_pair_evaluations);
	assert(large_distribution.approximation_quality.distance_evaluations == sampled_distribution_pair_evaluations);
	assert(*distance_evaluation_counter == sampled_distribution_pair_evaluations);
	assert(*distance_evaluation_counter < exact_distribution_pair_evaluations);
	assert(*distance_evaluation_counter < exact_dense_plan.dense_distance_slots);

	mtrc::benchmarks::ScaleGuardrailRow large_distribution_guardrail;
	large_distribution_guardrail.benchmark = "large sampled distribution guardrail";
	large_distribution_guardrail.workload = "default distance distribution sampled before dense pair scan";
	large_distribution_guardrail.record_count = large_record_count;
	large_distribution_guardrail.shared_operations = 1;
	large_distribution_guardrail.exact_plan_distance_evaluations = exact_distribution_pair_evaluations;
	large_distribution_guardrail.naive_workflow_distance_evaluations = exact_distribution_pair_evaluations;
	large_distribution_guardrail.dense_distance_slots = exact_dense_plan.dense_distance_slots;
	large_distribution_guardrail.max_distance_evaluations = sampled_distribution_pair_evaluations;
	large_distribution_guardrail.expected_distance_evaluations = sampled_distribution_pair_evaluations;
	large_distribution_guardrail.observed_distance_evaluations = *distance_evaluation_counter;
	large_distribution_guardrail.decision = "default_sampled";
	large_distribution_guardrail.representation = large_distribution.representation;
	large_distribution_guardrail.exact = large_distribution.exact;
	large_distribution_guardrail.passed =
		large_distribution_guardrail.observed_distance_evaluations ==
		large_distribution_guardrail.expected_distance_evaluations;
	large_distribution_guardrail.notes = "counting metric proves pair sampling, not dense unordered-pair scan";
	report.add_scale_guardrail(large_distribution_guardrail);

	counting_metric.reset();
	assert(*distance_evaluation_counter == 0);
	mtrc::describe_options sampled_describe_options;
	sampled_describe_options.max_exact_pair_summary_records = 64;
	sampled_describe_options.pair_summary_sample_count = sampled_describe_pair_evaluations;
	sampled_describe_options.max_exact_intrinsic_records = 64;
	sampled_describe_options.intrinsic_sample_count = sampled_describe_intrinsic_centers;
	sampled_describe_options.intrinsic_radius_sample_count = 8;
	const auto large_describe = mtrc::describe_structure(large_space, sampled_describe_options);
	assert(!large_describe.exact);
	assert(large_describe.representation == "metric_space");
	assert(large_describe.strategy == "sampled_pairs_sampled_intrinsic_dimension");
	assert(*distance_evaluation_counter == sampled_describe_expected_evaluations);
	assert(*distance_evaluation_counter < exact_dense_plan.estimated_distance_evaluations);

	mtrc::benchmarks::BenchmarkTargetRow describe_target;
	describe_target.benchmark = "describe sampled guardrail target";
	describe_target.workload = "large describe uses sampled pair summary and intrinsic dimension";
	describe_target.record_count = large_record_count;
	describe_target.query_count = 0;
	describe_target.target = "sampled describe stays below dense exact n^2 plan";
	describe_target.max_distance_evaluations = sampled_describe_expected_evaluations;
	describe_target.max_dense_slots = exact_dense_plan.dense_distance_slots;
	describe_target.expected_representation = large_describe.representation;
	describe_target.exact = large_describe.exact;
	describe_target.automated = true;
	describe_target.notes = "CountingMetric covers pair sample plus sampled intrinsic centers";
	report.add_benchmark_target(describe_target);

	mtrc::benchmarks::BenchmarkTrendRow describe_trend;
	describe_trend.benchmark = "describe sampled vs exact trend";
	describe_trend.workload = "large describe uses sampled pair summary and intrinsic dimension";
	describe_trend.record_count = large_record_count;
	describe_trend.query_count = 0;
	describe_trend.baseline_distance_evaluations = exact_dense_plan.estimated_distance_evaluations;
	describe_trend.observed_distance_evaluations = *distance_evaluation_counter;
	describe_trend.dense_distance_slots = exact_dense_plan.dense_distance_slots;
	describe_trend.representation = large_describe.representation;
	describe_trend.exact = large_describe.exact;
	describe_trend.passed = describe_trend.observed_distance_evaluations == sampled_describe_expected_evaluations;
	describe_trend.notes = "sampled describe is visible next to exact dense baseline";
	report.add_benchmark_trend(describe_trend);

	mtrc::benchmarks::ScaleGuardrailRow describe_guardrail;
	describe_guardrail.benchmark = "large sampled describe guardrail";
	describe_guardrail.workload = "describe sampled before exact dense diagnostic scan";
	describe_guardrail.record_count = large_record_count;
	describe_guardrail.shared_operations = 1;
	describe_guardrail.exact_plan_distance_evaluations = exact_dense_plan.estimated_distance_evaluations;
	describe_guardrail.naive_workflow_distance_evaluations = exact_dense_plan.estimated_distance_evaluations;
	describe_guardrail.dense_distance_slots = exact_dense_plan.dense_distance_slots;
	describe_guardrail.max_distance_evaluations = sampled_describe_expected_evaluations;
	describe_guardrail.expected_distance_evaluations = sampled_describe_expected_evaluations;
	describe_guardrail.observed_distance_evaluations = *distance_evaluation_counter;
	describe_guardrail.decision = "default_sampled";
	describe_guardrail.representation = large_describe.representation;
	describe_guardrail.exact = large_describe.exact;
	describe_guardrail.passed =
		describe_guardrail.observed_distance_evaluations == describe_guardrail.expected_distance_evaluations;
	describe_guardrail.notes = "CountingMetric proves sampled describe work instead of dense exact diagnostic work";
	report.add_scale_guardrail(describe_guardrail);

	constexpr std::size_t chunked_record_count = 48;
	constexpr std::size_t chunked_chunk_size = 8;
	const auto chunked_records = make_integer_records(chunked_record_count);
	auto chunked_metric = mtrc::benchmarks::make_counting_metric(AbsoluteDistance{});
	const auto chunked_counter = chunked_metric.counter();
	auto chunked_space = mtrc::make_space(chunked_records, chunked_metric);
	const auto chunks = mtrc::space::chunked_view(chunked_space, chunked_chunk_size);
	const auto chunked_plan = chunks.plan_diagnostics();
	assert(chunked_plan.record_count == chunked_record_count);
	assert(chunked_plan.chunk_size == chunked_chunk_size);
	assert(chunked_plan.representation == "chunked_space_view");
	assert(chunked_plan.bounded_pair_distance_evaluations < chunked_plan.dense_pair_distance_evaluations);
	std::size_t chunked_local_visits = 0;
	std::size_t chunked_representative_visits = 0;
	const auto returned_local_pairs = mtrc::space::for_each_local_pair(
		chunks, [&](std::size_t, mtrc::RecordId, mtrc::RecordId, int distance) {
			++chunked_local_visits;
			(void)distance;
		});
	const auto returned_representative_pairs = mtrc::space::for_each_representative_pair(
		chunks, [&](std::size_t, std::size_t, mtrc::RecordId, mtrc::RecordId, int distance) {
			++chunked_representative_visits;
			(void)distance;
		});
	assert(returned_local_pairs == chunked_plan.local_pair_distance_evaluations);
	assert(returned_representative_pairs == chunked_plan.representative_pair_distance_evaluations);
	assert(chunked_local_visits == chunked_plan.local_pair_distance_evaluations);
	assert(chunked_representative_visits == chunked_plan.representative_pair_distance_evaluations);
	assert(*chunked_counter == chunked_plan.bounded_pair_distance_evaluations);

	mtrc::benchmarks::BenchmarkTargetRow chunked_target;
	chunked_target.benchmark = "chunked workflow target";
	chunked_target.workload = "local exact pairs plus representative merge pairs";
	chunked_target.record_count = chunked_record_count;
	chunked_target.query_count = 0;
	chunked_target.target = "bounded chunk work below dense all-pairs";
	chunked_target.max_distance_evaluations = chunked_plan.bounded_pair_distance_evaluations;
	chunked_target.max_dense_slots = chunked_record_count * chunked_record_count;
	chunked_target.expected_representation = chunked_plan.representation;
	chunked_target.exact = true;
	chunked_target.automated = true;
	chunked_target.notes = "CountingMetric covers local and representative pair callbacks";
	report.add_benchmark_target(chunked_target);

	mtrc::benchmarks::BenchmarkTrendRow chunked_trend;
	chunked_trend.benchmark = "chunked workflow trend";
	chunked_trend.workload = "local exact pairs plus representative merge pairs";
	chunked_trend.record_count = chunked_record_count;
	chunked_trend.query_count = 0;
	chunked_trend.baseline_distance_evaluations = chunked_plan.dense_pair_distance_evaluations;
	chunked_trend.observed_distance_evaluations = *chunked_counter;
	chunked_trend.cached_distances = 0;
	chunked_trend.dense_distance_slots = chunked_record_count * chunked_record_count;
	chunked_trend.representation = chunked_plan.representation;
	chunked_trend.exact = true;
	chunked_trend.passed = chunked_trend.observed_distance_evaluations == chunked_plan.bounded_pair_distance_evaluations;
	chunked_trend.notes = "chunk-local exact work stays below dense full-pair baseline";
	report.add_benchmark_trend(chunked_trend);

	mtrc::benchmarks::ScaleGuardrailRow chunked_guardrail;
	chunked_guardrail.benchmark = "chunked workflow guardrail";
	chunked_guardrail.workload = "local exact pairs plus representative merge pairs";
	chunked_guardrail.record_count = chunked_record_count;
	chunked_guardrail.shared_operations = chunks.chunk_count();
	chunked_guardrail.exact_plan_distance_evaluations = chunked_plan.dense_pair_distance_evaluations;
	chunked_guardrail.naive_workflow_distance_evaluations = chunked_plan.dense_pair_distance_evaluations;
	chunked_guardrail.dense_distance_slots = chunked_record_count * chunked_record_count;
	chunked_guardrail.max_distance_evaluations = chunked_plan.bounded_pair_distance_evaluations;
	chunked_guardrail.expected_distance_evaluations = chunked_plan.bounded_pair_distance_evaluations;
	chunked_guardrail.observed_distance_evaluations = *chunked_counter;
	chunked_guardrail.decision = "chunked_local_representatives";
	chunked_guardrail.representation = chunked_plan.representation;
	chunked_guardrail.exact = true;
	chunked_guardrail.passed =
		chunked_guardrail.observed_distance_evaluations == chunked_guardrail.expected_distance_evaluations;
	chunked_guardrail.notes = "CountingMetric proves m*O(b^2) local work plus representative merge";
	report.add_scale_guardrail(chunked_guardrail);

	auto chunked_compare_left_metric = mtrc::benchmarks::make_counting_metric(AbsoluteDistance{});
	auto chunked_compare_right_metric = mtrc::benchmarks::make_counting_metric(AbsoluteDistance{});
	const auto chunked_compare_left_counter = chunked_compare_left_metric.counter();
	const auto chunked_compare_right_counter = chunked_compare_right_metric.counter();
	auto chunked_compare_right_records = chunked_records;
	for (auto &value : chunked_compare_right_records) {
		value = value * value + 3;
	}
	auto chunked_compare_left_space = mtrc::make_space(chunked_records, chunked_compare_left_metric);
	auto chunked_compare_right_space = mtrc::make_space(chunked_compare_right_records, chunked_compare_right_metric);
	auto chunked_compare_policy = mtrc::space::storage::using_distance_table();
	chunked_compare_policy =
		mtrc::space::storage::with_distance_table_budget(chunked_compare_policy, chunked_chunk_size, 0);
	chunked_compare_policy = mtrc::space::storage::allow_approximate_fallback(chunked_compare_policy);
	chunked_compare_policy = mtrc::space::storage::allow_chunking_fallback(chunked_compare_policy);
	const auto chunked_compare_plan =
		mtrc::space::storage::estimate_cost(chunked_compare_left_space, "compare", chunked_compare_policy);
	assert(chunked_compare_plan.allowed);
	assert(chunked_compare_plan.downgraded);
	assert(!chunked_compare_plan.exact);
	assert(chunked_compare_plan.representation == "chunked_space_view");

	const auto chunked_compared =
		mtrc::compare(chunked_compare_left_space, chunked_compare_right_space, chunked_compare_policy);
	const auto chunked_compare_observed =
		*chunked_compare_left_counter + *chunked_compare_right_counter;
	const auto dense_compare_evaluations = chunked_record_count * (chunked_record_count - 1);
	assert(!chunked_compared.exact);
	assert(chunked_compared.algorithm == "chunked_mgc_estimate");
	assert(chunked_compared.left_representation == "chunked_space_view");
	assert(chunked_compared.right_representation == "chunked_space_view");
	assert(chunked_compared.sample_iterations == 1);
	assert(chunked_compared.value >= -1.0);
	assert(chunked_compared.value <= 1.0);
	assert(chunked_compare_observed <= chunked_compare_plan.estimated_distance_evaluations);
	assert(chunked_compare_observed < dense_compare_evaluations);

	mtrc::benchmarks::BenchmarkTargetRow chunked_compare_target;
	chunked_compare_target.benchmark = "chunked compare target";
	chunked_compare_target.workload = "paired MGC over chunked representative sample";
	chunked_compare_target.record_count = chunked_record_count;
	chunked_compare_target.query_count = 0;
	chunked_compare_target.target = "bounded chunked MGC estimate below dense compare work";
	chunked_compare_target.max_distance_evaluations = chunked_compare_observed;
	chunked_compare_target.max_dense_slots = chunked_record_count * chunked_record_count;
	chunked_compare_target.expected_representation = chunked_compared.left_representation;
	chunked_compare_target.exact = chunked_compared.exact;
	chunked_compare_target.automated = true;
	chunked_compare_target.notes = "CountingMetric covers deterministic chunked compare sample";
	report.add_benchmark_target(chunked_compare_target);

	mtrc::benchmarks::BenchmarkTrendRow chunked_compare_trend;
	chunked_compare_trend.benchmark = "chunked compare trend";
	chunked_compare_trend.workload = "paired MGC over chunked representative sample";
	chunked_compare_trend.record_count = chunked_record_count;
	chunked_compare_trend.query_count = 0;
	chunked_compare_trend.baseline_distance_evaluations = dense_compare_evaluations;
	chunked_compare_trend.observed_distance_evaluations = chunked_compare_observed;
	chunked_compare_trend.cached_distances = 0;
	chunked_compare_trend.dense_distance_slots = chunked_record_count * chunked_record_count;
	chunked_compare_trend.representation = chunked_compared.left_representation;
	chunked_compare_trend.exact = chunked_compared.exact;
	chunked_compare_trend.passed = chunked_compare_observed < dense_compare_evaluations;
	chunked_compare_trend.notes = "chunked compare avoids dense paired all-pairs MGC";
	report.add_benchmark_trend(chunked_compare_trend);

	mtrc::benchmarks::ScaleGuardrailRow chunked_compare_guardrail;
	chunked_compare_guardrail.benchmark = "chunked compare guardrail";
	chunked_compare_guardrail.workload = "paired MGC over chunked representative sample";
	chunked_compare_guardrail.record_count = chunked_record_count;
	chunked_compare_guardrail.shared_operations = chunks.chunk_count();
	chunked_compare_guardrail.exact_plan_distance_evaluations = dense_compare_evaluations;
	chunked_compare_guardrail.naive_workflow_distance_evaluations = dense_compare_evaluations;
	chunked_compare_guardrail.dense_distance_slots = chunked_record_count * chunked_record_count;
	chunked_compare_guardrail.max_distance_evaluations = chunked_compare_observed;
	chunked_compare_guardrail.expected_distance_evaluations = chunked_compare_observed;
	chunked_compare_guardrail.observed_distance_evaluations = chunked_compare_observed;
	chunked_compare_guardrail.decision = "chunked_mgc_estimate";
	chunked_compare_guardrail.representation = chunked_compared.left_representation;
	chunked_compare_guardrail.exact = chunked_compared.exact;
	chunked_compare_guardrail.passed = chunked_compare_observed < dense_compare_evaluations;
	chunked_compare_guardrail.notes = "CountingMetric proves bounded chunked compare work";
	report.add_scale_guardrail(chunked_compare_guardrail);

	chunked_metric.reset();
	const auto chunked_volume = mtrc::chunked_local_volume(chunks, 4);
	assert(*chunked_counter == chunked_plan.bounded_pair_distance_evaluations);
	assert(!chunked_volume.exact);
	assert(chunked_volume.algorithm == "chunked_local_volume");
	assert(chunked_volume.representation == "chunked_space_view");
	assert(chunked_volume.evaluated_distance_count == chunked_plan.bounded_pair_distance_evaluations);
	assert(chunked_volume.sample_universe == chunked_plan.dense_pair_distance_evaluations);
	assert(chunked_volume.approximation_quality.candidate_policy == "local_chunks_plus_representative_pairs");
	assert(chunked_volume.approximation_quality.distance_evaluations == chunked_plan.bounded_pair_distance_evaluations);

	const auto dense_directed_local_volume_evaluations = chunked_record_count * chunked_record_count;
	mtrc::benchmarks::BenchmarkTargetRow chunked_volume_target;
	chunked_volume_target.benchmark = "chunked local-volume target";
	chunked_volume_target.workload = "local-volume over chunked view";
	chunked_volume_target.record_count = chunked_record_count;
	chunked_volume_target.query_count = 0;
	chunked_volume_target.target = "local plus representative volume work below dense directed scan";
	chunked_volume_target.max_distance_evaluations = chunked_volume.evaluated_distance_count;
	chunked_volume_target.max_dense_slots = dense_directed_local_volume_evaluations;
	chunked_volume_target.expected_representation = chunked_volume.representation;
	chunked_volume_target.exact = chunked_volume.exact;
	chunked_volume_target.automated = true;
	chunked_volume_target.notes = "CountingMetric guards explicit chunked_local_volume work";
	report.add_benchmark_target(chunked_volume_target);

	mtrc::benchmarks::BenchmarkTrendRow chunked_volume_trend;
	chunked_volume_trend.benchmark = "chunked local-volume trend";
	chunked_volume_trend.workload = "local-volume over chunked view";
	chunked_volume_trend.record_count = chunked_record_count;
	chunked_volume_trend.query_count = 0;
	chunked_volume_trend.baseline_distance_evaluations = dense_directed_local_volume_evaluations;
	chunked_volume_trend.observed_distance_evaluations = *chunked_counter;
	chunked_volume_trend.cached_distances = 0;
	chunked_volume_trend.dense_distance_slots = dense_directed_local_volume_evaluations;
	chunked_volume_trend.representation = chunked_volume.representation;
	chunked_volume_trend.exact = chunked_volume.exact;
	chunked_volume_trend.passed =
		chunked_volume_trend.observed_distance_evaluations == chunked_volume.evaluated_distance_count;
	chunked_volume_trend.notes = "chunked local-volume avoids dense directed n*n scan";
	report.add_benchmark_trend(chunked_volume_trend);

	mtrc::benchmarks::ScaleGuardrailRow chunked_volume_guardrail;
	chunked_volume_guardrail.benchmark = "chunked local-volume guardrail";
	chunked_volume_guardrail.workload = "local-volume over chunked view";
	chunked_volume_guardrail.record_count = chunked_record_count;
	chunked_volume_guardrail.shared_operations = chunks.chunk_count();
	chunked_volume_guardrail.exact_plan_distance_evaluations = dense_directed_local_volume_evaluations;
	chunked_volume_guardrail.naive_workflow_distance_evaluations = dense_directed_local_volume_evaluations;
	chunked_volume_guardrail.dense_distance_slots = dense_directed_local_volume_evaluations;
	chunked_volume_guardrail.max_distance_evaluations = chunked_volume.evaluated_distance_count;
	chunked_volume_guardrail.expected_distance_evaluations = chunked_volume.evaluated_distance_count;
	chunked_volume_guardrail.observed_distance_evaluations = *chunked_counter;
	chunked_volume_guardrail.decision = "chunked_local_volume";
	chunked_volume_guardrail.representation = chunked_volume.representation;
	chunked_volume_guardrail.exact = chunked_volume.exact;
	chunked_volume_guardrail.passed =
		chunked_volume_guardrail.observed_distance_evaluations == chunked_volume_guardrail.expected_distance_evaluations;
	chunked_volume_guardrail.notes = "CountingMetric proves bounded local-volume work";
	report.add_scale_guardrail(chunked_volume_guardrail);

	assert(report.representation_costs().size() == 2);
	assert(report.workflow_evidence().size() == 2);
	assert(report.performance_rows().size() == 1);
	assert(report.benchmark_targets().size() == 9);
	assert(report.benchmark_trends().size() == 11);
	assert(report.wall_clock_trends().size() == 2);
	assert(report.wall_clock_trends().front().elapsed_ns > 0);
	assert(report.wall_clock_trends().back().benchmark == "measured wall-clock smoke");
	assert(report.wall_clock_trends().back().elapsed_ns > 0);
	assert(report.wall_clock_trends().back().distance_evaluations ==
		   measured_wall_clock_record_count * measured_wall_clock_record_count);
	assert(report.wall_clock_trends().back().memory_bytes_estimate > 0);
	assert(report.scale_guardrails().size() == 7);
	assert(report.out_of_core_readiness().size() == 1);
	assert(report.run_metadata().suite == "smoke suite");

	const auto markdown = report.to_markdown();
	assert(markdown.find("# METRIC Engine Benchmark Report") != std::string::npos);
	assert(markdown.find("## Run Metadata") != std::string::npos);
	assert(markdown.find("| Suite | smoke suite |") != std::string::npos);
	assert(markdown.find("| Platform | portable deterministic fixture |") != std::string::npos);
	assert(markdown.find("## Representation Cost") != std::string::npos);
	assert(markdown.find("integer distance table") != std::string::npos);
	assert(markdown.find("| integer distance table | distance_table | 4 | 16 | 16 | 16 |") != std::string::npos);
	assert(markdown.find("| integer kNN graph | knn_graph_index | 4 | 12 | 8 | 0 |") != std::string::npos);
	assert(markdown.find("## Application Workflow Evidence") != std::string::npos);
	assert(markdown.find("| string edit baseline | 4 | 4/4 | 4/4 | - | 2 |") != std::string::npos);
	assert(markdown.find("| process curve diffusion-coordinate gallery | 6 | 6/6 | 6/6 | 6/6 | 2.16667 |") != std::string::npos);
	assert(markdown.find("## Performance Evidence") != std::string::npos);
	assert(markdown.find("| integer amortization | shared gallery operators | 4 | 3 | 36 | 16 | 0 | 0 | ") !=
		   std::string::npos);
	assert(markdown.find("| 2.25 | yes | build once, reuse free |") != std::string::npos);
	assert(markdown.find("## Benchmark Targets") != std::string::npos);
	assert(markdown.find("| one-off kNN guardrail target | large exact materialized request downgraded to sampled "
						 "candidates | 5000 | 1 | bounded sampled candidate count | 4096 | 25000000 | "
						 "sampled_metric_space | no | yes |") != std::string::npos);
	assert(markdown.find("| batch kNN guardrail target | large query batch downgraded to sampled candidates | 5000 | "
						 "8 | bounded sampled candidate count per query | 32768 | 25000000 | sampled_metric_space | "
						 "no | yes |") != std::string::npos);
	assert(markdown.find("| repeated provider workflow target | shared kNN plus two distribution passes | 16 | 10 | "
						 "materialize once and keep observed metric calls below naive repeated workflow | 256 | 256 | "
						 "distance_table | yes | yes |") != std::string::npos);
	assert(markdown.find("| lazy sparse cache target | repeat one pair lookup | 12 | 2 | second lookup is a cache hit "
						 "| 1 | 144 | distance_table | yes | yes |") != std::string::npos);
	const auto landmark_target_row =
		std::string("| landmark kNN target | large metric query via landmark lower-bound candidates | ") +
		std::to_string(landmark_target.record_count) + " | " + std::to_string(landmark_target.query_count) +
		" | landmark build plus bounded exact refinement stays below dense all-pairs | " +
		std::to_string(landmark_target.max_distance_evaluations) + " | " +
		std::to_string(landmark_target.max_dense_slots) + " | landmark_index | no | yes |";
	assert(markdown.find(landmark_target_row) != std::string::npos);
	assert(markdown.find("| describe sampled guardrail target | large describe uses sampled pair summary and intrinsic "
						 "dimension | 5000 | 0 | sampled describe stays below dense exact n^2 plan | 20512 | "
						 "25000000 | metric_space | no | yes |") != std::string::npos);
	assert(markdown.find("| chunked workflow target | local exact pairs plus representative merge pairs | 48 | 0 | "
						 "bounded chunk work below dense all-pairs | 183 | 2304 | chunked_space_view | yes | yes |") !=
		   std::string::npos);
	const auto chunked_compare_target_row =
		std::string("| chunked compare target | paired MGC over chunked representative sample | 48 | 0 | "
					"bounded chunked MGC estimate below dense compare work | ") +
		std::to_string(chunked_compare_observed) + " | 2304 | chunked_space_view | no | yes |";
	assert(markdown.find(chunked_compare_target_row) != std::string::npos);
	assert(markdown.find("| chunked local-volume target | local-volume over chunked view | 48 | 0 | "
						 "local plus representative volume work below dense directed scan | 183 | 2304 | "
						 "chunked_space_view | no | yes |") != std::string::npos);
	assert(markdown.find("## Benchmark Trends") != std::string::npos);
	assert(markdown.find("| one-off kNN exact trend | single query over integer smoke space | 32 | 1 | 31 | 31 | "
						 "0 | 0 | metric_space | yes | yes |") != std::string::npos);
	assert(markdown.find("| batch kNN exact trend | five query-id kNN calls through batch API | 32 | 5 | 155 | "
						 "155 | 0 | 0 | metric_space | yes | yes |") != std::string::npos);
	assert(markdown.find("| repeated provider workflow trend | shared kNN plus two distribution passes | 16 | 10 | "
						 "390 | 256 | 256 | 256 | distance_table | yes | yes |") != std::string::npos);
	assert(markdown.find("| lazy sparse cache trend | repeat one pair lookup | 12 | 2 | 2 | 1 | 1 | 144 | "
						 "distance_table | yes | yes |") != std::string::npos);
	const auto landmark_trend_row =
		std::string("| landmark kNN trend | large metric query via landmark lower-bound candidates | ") +
		std::to_string(landmark_trend.record_count) + " | " + std::to_string(landmark_trend.query_count) +
		" | " + std::to_string(landmark_trend.baseline_distance_evaluations) + " | " +
		std::to_string(landmark_trend.observed_distance_evaluations) + " | " +
		std::to_string(landmark_trend.cached_distances) + " | " +
		std::to_string(landmark_trend.dense_distance_slots) + " | landmark_index | no | yes |";
	assert(markdown.find(landmark_trend_row) != std::string::npos);
	assert(markdown.find("| one-off kNN sampled trend | large exact materialized request downgraded to sampled "
						 "candidates | 5000 | 1 | 25000000 | 4096 | 0 | 25000000 | sampled_metric_space | no | "
						 "yes |") != std::string::npos);
	assert(markdown.find("| batch kNN sampled trend | query batch downgraded before dense build | 5000 | 8 | "
						 "200000000 | 32768 | 0 | 25000000 | sampled_metric_space | no | yes |") !=
		   std::string::npos);
	assert(markdown.find("| describe sampled vs exact trend | large describe uses sampled pair summary and intrinsic "
						 "dimension | 5000 | 0 | 25000000 | 20512 | 0 | 25000000 | metric_space | no | yes |") !=
		   std::string::npos);
	assert(markdown.find("| chunked workflow trend | local exact pairs plus representative merge pairs | 48 | 0 | "
						 "1128 | 183 | 0 | 2304 | chunked_space_view | yes | yes |") != std::string::npos);
	const auto chunked_compare_trend_row =
		std::string("| chunked compare trend | paired MGC over chunked representative sample | 48 | 0 | 2256 | ") +
		std::to_string(chunked_compare_observed) + " | 0 | 2304 | chunked_space_view | no | yes |";
	assert(markdown.find(chunked_compare_trend_row) != std::string::npos);
	assert(markdown.find("| chunked local-volume trend | local-volume over chunked view | 48 | 0 | "
						 "2304 | 183 | 0 | 2304 | chunked_space_view | no | yes |") != std::string::npos);
	assert(markdown.find("## Wall-clock Trends") != std::string::npos);
	const auto wall_clock_smoke_row =
		std::string("| synthetic wall-clock smoke | synthetic recorded metadata sample | 4 | distance_table | 1 | "
					"16 | ") +
		std::to_string(wall_clock_smoke.memory_bytes_estimate) +
		" | 1 | synthetic-smoke | yes | yes | synthetic smoke metadata; no wall-clock threshold or regression claim |";
	assert(markdown.find(wall_clock_smoke_row) != std::string::npos);
	const auto measured_wall_clock_row =
		std::string("| measured wall-clock smoke | small distance-table build | ") +
		std::to_string(measured_wall_clock.record_count) + " | distance_table | " +
		std::to_string(measured_wall_clock.elapsed_ns) + " | " +
		std::to_string(measured_wall_clock.distance_evaluations) + " | " +
		std::to_string(measured_wall_clock.memory_bytes_estimate) +
		" | 1 | steady_clock-smoke | yes | yes | measured with std::chrono; CTest asserts only nonzero elapsed and report fields |";
	assert(markdown.find(measured_wall_clock_row) != std::string::npos);
	assert(markdown.find("## Scale Guardrails") != std::string::npos);
	assert(markdown.find("| large sampled neighbor guardrail | materialized exact request downgraded before dense "
						 "build | 5000 | 3 | 25000000 | 75000000 | 25000000 | 4096 | 4096 | 4096 | "
						 "downgraded | sampled_metric_space | no | yes |") != std::string::npos);
	assert(markdown.find("| large sampled batch neighbor guardrail | query batch downgraded before dense build | "
						 "5000 | 8 | 25000000 | 200000000 | 25000000 | 32768 | 32768 | 32768 | downgraded | "
						 "sampled_metric_space | no | yes |") != std::string::npos);
	assert(markdown.find("| large sampled distribution guardrail | default distance distribution sampled before dense "
						 "pair scan | 5000 | 1 | 12497500 | 12497500 | 25000000 | 4096 | 4096 | 4096 | "
						 "default_sampled | metric_space | no | yes |") != std::string::npos);
	assert(markdown.find("| large sampled describe guardrail | describe sampled before exact dense diagnostic scan | "
						 "5000 | 1 | 25000000 | 25000000 | 25000000 | 20512 | 20512 | 20512 | default_sampled | "
						 "metric_space | no | yes |") != std::string::npos);
	assert(markdown.find("| chunked workflow guardrail | local exact pairs plus representative merge pairs | 48 | "
						 "6 | 1128 | 1128 | 2304 | 183 | 183 | 183 | chunked_local_representatives | "
						 "chunked_space_view | yes | yes |") != std::string::npos);
	const auto chunked_compare_guardrail_row =
		std::string("| chunked compare guardrail | paired MGC over chunked representative sample | 48 | 6 | "
					"2256 | 2256 | 2304 | ") +
		std::to_string(chunked_compare_observed) + " | " + std::to_string(chunked_compare_observed) + " | " +
		std::to_string(chunked_compare_observed) +
		" | chunked_mgc_estimate | chunked_space_view | no | yes |";
	assert(markdown.find(chunked_compare_guardrail_row) != std::string::npos);
	assert(markdown.find("| chunked local-volume guardrail | local-volume over chunked view | 48 | 6 | "
						 "2304 | 2304 | 2304 | 183 | 183 | 183 | chunked_local_volume | chunked_space_view | "
						 "no | yes |") != std::string::npos);
	assert(markdown.find("## Out-of-core Readiness") != std::string::npos);
	const auto spill_readiness_row =
		std::string("| out-of-core spill readiness | explicit dense materialization with spill disabled | 5000 | "
					"25000000 | ") +
		std::to_string(spill_refusal_plan.memory_bytes_estimate) + " | " +
		std::to_string(out_of_core_memory_budget) + " | " + std::to_string(out_of_core_spill_block_bytes) + " | " +
		std::to_string(spill_readiness.planned_spill_blocks) +
		" | 0 | no | yes | refused | yes | yes | spill disabled: dense plan refuses before metric calls |";
	assert(markdown.find(spill_readiness_row) != std::string::npos);

	return 0;
}
