#include <cassert>
#include <string>
#include <vector>

#include "metric/engine.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

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

	mtrc::benchmarks::WorkflowEvidenceRow phate_hero;
	phate_hero.benchmark = "process curve PHATE gallery";
	phate_hero.query_count = 6;
	phate_hero.metric_matches = 6;
	phate_hero.vector_mismatches = 6;
	phate_hero.latent_matches = 6;
	phate_hero.reports_latent_matches = true;
	phate_hero.average_metric_margin = 2.16667;
	phate_hero.reports_average_metric_margin = true;
	phate_hero.diagnostics = "OOS rank and distance penalty";
	report.add_workflow_evidence(phate_hero);

	mtrc::benchmarks::PerformanceRow amortization;
	amortization.benchmark = "integer amortization";
	amortization.workload = "shared gallery operators";
	amortization.record_count = 4;
	amortization.shared_operations = 3;
	amortization.naive_distance_evaluations = 36;
	amortization.materialized_distance_evaluations = 16;
	amortization.cache_miss_evaluations = 0;
	amortization.evaluation_reduction_factor = 2.25;
	amortization.exact_match = true;
	amortization.notes = "build once, reuse free";
	report.add_performance_row(amortization);

	assert(report.representation_costs().size() == 2);
	assert(report.workflow_evidence().size() == 2);
	assert(report.performance_rows().size() == 1);
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
	assert(markdown.find("| process curve PHATE gallery | 6 | 6/6 | 6/6 | 6/6 | 2.16667 |") != std::string::npos);
	assert(markdown.find("## Performance Evidence") != std::string::npos);
	assert(markdown.find("| integer amortization | shared gallery operators | 4 | 3 | 36 | 16 | 0 | 2.25 | yes |") !=
		   std::string::npos);

	return 0;
}
