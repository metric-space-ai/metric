// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_BENCHMARKS_REPORT_HPP
#define _METRIC_BENCHMARKS_REPORT_HPP

#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <metric/space/storage/diagnostics.hpp>

namespace mtrc::benchmarks {

struct RepresentationCostRow {
	std::string benchmark;
	std::string representation;
	std::size_t record_count{};
	std::size_t distance_evaluations{};
	std::size_t cached_distances{};
	std::size_t dense_distance_slots{};
	std::size_t memory_bytes_estimate{};
	std::string exactness;
	std::string materialization;
	std::string notes;
};

struct WorkflowEvidenceRow {
	std::string benchmark;
	std::size_t query_count{};
	std::size_t metric_matches{};
	std::size_t vector_mismatches{};
	std::size_t latent_matches{};
	bool reports_latent_matches{};
	double average_metric_margin{};
	bool reports_average_metric_margin{};
	std::string diagnostics;
};

// Deterministic work-reduction evidence.
//
// METRIC has no metric-pruned nearest-neighbour search in the current core: a
// cover-tree or kNN-graph query over an out-of-sample point still evaluates the
// domain metric against every record. The reproducible performance argument is
// therefore *amortization*: a hero workflow runs several distance-consuming
// operators (neighbour search, grouping, outlier scoring, dependency tests) over
// one reference gallery. A naive pipeline re-evaluates the domain metric for
// every operator; a materialized representation (DistanceTable / KnnGraphIndex)
// evaluates the pairwise structure once and serves every later operator from the
// cache with zero additional metric evaluations.
//
// All counts below are measured with a shared-counter metric wrapper, so both
// the naive and the materialized columns are observed, not asserted by
// construction. They are integers and fully reproducible across platforms; this
// row carries no wall-clock timing (timing is reported out-of-band so the
// checked-in artifact stays byte-stable).
struct PerformanceRow {
	std::string benchmark;
	std::string workload;
	std::size_t record_count{};
	std::size_t shared_operations{};
	std::size_t naive_distance_evaluations{};
	std::size_t materialized_distance_evaluations{};
	std::size_t cache_miss_evaluations{};
	std::size_t naive_memory_bytes_estimate{};
	std::size_t materialized_memory_bytes_estimate{};
	double evaluation_reduction_factor{};
	bool exact_match{};
	std::string notes;
};

struct BenchmarkTargetRow {
	std::string benchmark;
	std::string workload;
	std::size_t record_count{};
	std::size_t query_count{};
	std::string target;
	std::size_t max_distance_evaluations{};
	std::size_t max_dense_slots{};
	std::string expected_representation;
	bool exact{};
	bool automated{};
	std::string notes;
};

struct BenchmarkTrendRow {
	std::string benchmark;
	std::string workload;
	std::size_t record_count{};
	std::size_t query_count{};
	std::size_t baseline_distance_evaluations{};
	std::size_t observed_distance_evaluations{};
	std::size_t cached_distances{};
	std::size_t dense_distance_slots{};
	std::string representation;
	bool exact{};
	bool passed{};
	std::string notes;
};

struct WallClockTrendRow {
	std::string benchmark;
	std::string workload;
	std::size_t record_count{};
	std::string representation;
	std::size_t elapsed_ns{};
	std::size_t distance_evaluations{};
	std::size_t memory_bytes_estimate{};
	std::size_t sample_count{};
	std::string platform_label;
	bool automated{};
	bool passed{};
	std::string notes;
};

struct ScaleGuardrailRow {
	std::string benchmark;
	std::string workload;
	std::size_t record_count{};
	std::size_t shared_operations{};
	std::size_t exact_plan_distance_evaluations{};
	std::size_t naive_workflow_distance_evaluations{};
	std::size_t dense_distance_slots{};
	std::size_t max_distance_evaluations{};
	std::size_t expected_distance_evaluations{};
	std::size_t observed_distance_evaluations{};
	std::string decision;
	std::string representation;
	bool exact{};
	bool passed{};
	std::string notes;
};

struct OutOfCoreReadinessRow {
	std::string benchmark;
	std::string workload;
	std::size_t record_count{};
	std::size_t dense_distance_slots{};
	std::size_t dense_memory_bytes_estimate{};
	std::size_t memory_budget_bytes{};
	std::size_t spill_block_bytes{};
	std::size_t planned_spill_blocks{};
	std::size_t max_resident_blocks{};
	bool spill_enabled{};
	bool explicit_policy_required{true};
	std::string decision;
	bool automated{};
	bool passed{};
	std::string notes;
};

struct BenchmarkRunMetadata {
	std::string suite;
	std::string source;
	std::string build_profile;
	std::string platform;
	std::string artifact;
	std::string notes;
};

namespace detail {

inline auto escape_cell(const std::string &value) -> std::string
{
	std::string escaped;
	escaped.reserve(value.size());
	for (const auto character : value) {
		if (character == '|') {
			escaped.push_back('/');
		} else if (character == '\n' || character == '\r') {
			escaped.push_back(' ');
		} else {
			escaped.push_back(character);
		}
	}
	return escaped;
}

inline auto format_count(std::size_t numerator, std::size_t denominator) -> std::string
{
	std::ostringstream output;
	output << numerator << "/" << denominator;
	return output.str();
}

inline auto format_double(double value) -> std::string
{
	std::ostringstream output;
	output << std::setprecision(6) << value;
	return output.str();
}

inline auto has_text(const std::string &value) -> bool { return !value.empty(); }

inline auto has_run_metadata(const BenchmarkRunMetadata &metadata) -> bool
{
	return has_text(metadata.suite) || has_text(metadata.source) || has_text(metadata.build_profile) ||
		   has_text(metadata.platform) || has_text(metadata.artifact) || has_text(metadata.notes);
}

inline auto representation_kind_name(space::storage::representation_kind kind) -> std::string
{
	switch (kind) {
	case space::storage::representation_kind::live_distances:
		return "metric_space";
	case space::storage::representation_kind::distance_table:
		return "distance_table";
	case space::storage::representation_kind::cover_tree_index:
		return "cover_tree_index";
	case space::storage::representation_kind::knn_graph_index:
		return "knn_graph_index";
	case space::storage::representation_kind::landmark_index:
		return "landmark_index";
	case space::storage::representation_kind::graph_topology:
		return "graph_topology";
	}
	return "unknown";
}

inline auto exactness_name(space::storage::exactness value) -> std::string
{
	switch (value) {
	case space::storage::exactness::exact:
		return "exact";
	case space::storage::exactness::approximate:
		return "approximate";
	}
	return "unknown";
}

inline auto materialization_name(space::storage::materialization value) -> std::string
{
	switch (value) {
	case space::storage::materialization::lazy:
		return "lazy";
	case space::storage::materialization::materialized:
		return "materialized";
	case space::storage::materialization::topology:
		return "topology";
	}
	return "unknown";
}

} // namespace detail

inline auto representation_cost_row(std::string benchmark,
									const space::storage::representation_diagnostics &diagnostics,
									std::string notes = {}) -> RepresentationCostRow
{
	RepresentationCostRow row;
	row.benchmark = std::move(benchmark);
	row.representation = detail::representation_kind_name(diagnostics.kind);
	row.record_count = diagnostics.records;
	row.distance_evaluations = diagnostics.distance_evaluations;
	row.cached_distances = diagnostics.cached_distances;
	row.dense_distance_slots = diagnostics.dense_distance_slots;
	row.memory_bytes_estimate = diagnostics.memory_bytes_estimate;
	row.exactness = detail::exactness_name(diagnostics.exact);
	row.materialization = detail::materialization_name(diagnostics.materialized);
	row.notes = std::move(notes);
	return row;
}

class BenchmarkReport {
  public:
	explicit BenchmarkReport(std::string title) : title_(std::move(title)) {}

	auto add_representation_cost(RepresentationCostRow row) -> BenchmarkReport &
	{
		representation_costs_.push_back(std::move(row));
		return *this;
	}

	auto add_workflow_evidence(WorkflowEvidenceRow row) -> BenchmarkReport &
	{
		workflow_evidence_.push_back(std::move(row));
		return *this;
	}

	auto add_performance_row(PerformanceRow row) -> BenchmarkReport &
	{
		performance_rows_.push_back(std::move(row));
		return *this;
	}

	auto add_benchmark_target(BenchmarkTargetRow row) -> BenchmarkReport &
	{
		benchmark_targets_.push_back(std::move(row));
		return *this;
	}

	auto add_benchmark_trend(BenchmarkTrendRow row) -> BenchmarkReport &
	{
		benchmark_trends_.push_back(std::move(row));
		return *this;
	}

	auto add_wall_clock_trend(WallClockTrendRow row) -> BenchmarkReport &
	{
		wall_clock_trends_.push_back(std::move(row));
		return *this;
	}

	auto add_scale_guardrail(ScaleGuardrailRow row) -> BenchmarkReport &
	{
		scale_guardrails_.push_back(std::move(row));
		return *this;
	}

	auto add_out_of_core_readiness(OutOfCoreReadinessRow row) -> BenchmarkReport &
	{
		out_of_core_readiness_.push_back(std::move(row));
		return *this;
	}

	auto set_run_metadata(BenchmarkRunMetadata metadata) -> BenchmarkReport &
	{
		run_metadata_ = std::move(metadata);
		return *this;
	}

	auto run_metadata() const -> const BenchmarkRunMetadata & { return run_metadata_; }
	auto representation_costs() const -> const std::vector<RepresentationCostRow> & { return representation_costs_; }
	auto workflow_evidence() const -> const std::vector<WorkflowEvidenceRow> & { return workflow_evidence_; }
	auto performance_rows() const -> const std::vector<PerformanceRow> & { return performance_rows_; }
	auto benchmark_targets() const -> const std::vector<BenchmarkTargetRow> & { return benchmark_targets_; }
	auto benchmark_trends() const -> const std::vector<BenchmarkTrendRow> & { return benchmark_trends_; }
	auto wall_clock_trends() const -> const std::vector<WallClockTrendRow> & { return wall_clock_trends_; }
	auto scale_guardrails() const -> const std::vector<ScaleGuardrailRow> & { return scale_guardrails_; }
	auto out_of_core_readiness() const -> const std::vector<OutOfCoreReadinessRow> &
	{
		return out_of_core_readiness_;
	}

	auto to_markdown() const -> std::string
	{
		std::ostringstream output;
		output << "# " << title_ << "\n\n";

		if (detail::has_run_metadata(run_metadata_)) {
			output << "## Run Metadata\n\n";
			output << "| Field | Value |\n";
			output << "|---|---|\n";
			if (detail::has_text(run_metadata_.suite)) {
				output << "| Suite | " << detail::escape_cell(run_metadata_.suite) << " |\n";
			}
			if (detail::has_text(run_metadata_.source)) {
				output << "| Source | " << detail::escape_cell(run_metadata_.source) << " |\n";
			}
			if (detail::has_text(run_metadata_.build_profile)) {
				output << "| Build profile | " << detail::escape_cell(run_metadata_.build_profile) << " |\n";
			}
			if (detail::has_text(run_metadata_.platform)) {
				output << "| Platform | " << detail::escape_cell(run_metadata_.platform) << " |\n";
			}
			if (detail::has_text(run_metadata_.artifact)) {
				output << "| Artifact | " << detail::escape_cell(run_metadata_.artifact) << " |\n";
			}
			if (detail::has_text(run_metadata_.notes)) {
				output << "| Notes | " << detail::escape_cell(run_metadata_.notes) << " |\n";
			}
			output << "\n";
		}

		output << "## Representation Cost\n\n";
		output << "| Benchmark | Representation | Records | Distance evals | Cached | Dense slots | Memory bytes | "
				  "Exactness | Materialization | Notes |\n";
		output << "|---|---|---:|---:|---:|---:|---:|---|---|---|\n";
		for (const auto &row : representation_costs_) {
			output << "| " << detail::escape_cell(row.benchmark) << " | " << detail::escape_cell(row.representation)
				   << " | " << row.record_count << " | " << row.distance_evaluations << " | " << row.cached_distances
				   << " | " << row.dense_distance_slots << " | " << row.memory_bytes_estimate << " | "
				   << detail::escape_cell(row.exactness) << " | " << detail::escape_cell(row.materialization) << " | "
				   << detail::escape_cell(row.notes) << " |\n";
		}

		output << "\n## Application Workflow Evidence\n\n";
		output << "| Benchmark | Queries | Metric-space matches | Vector-comparison mismatches | Latent matches | Avg metric margin | "
				  "Diagnostics |\n";
		output << "|---|---:|---:|---:|---:|---:|---|\n";
		for (const auto &row : workflow_evidence_) {
			output << "| " << detail::escape_cell(row.benchmark) << " | " << row.query_count << " | "
				   << detail::format_count(row.metric_matches, row.query_count) << " | "
				   << detail::format_count(row.vector_mismatches, row.query_count) << " | ";
			if (row.reports_latent_matches) {
				output << detail::format_count(row.latent_matches, row.query_count);
			} else {
				output << "-";
			}
			output << " | ";
			if (row.reports_average_metric_margin) {
				output << detail::format_double(row.average_metric_margin);
			} else {
				output << "-";
			}
			output << " | " << detail::escape_cell(row.diagnostics) << " |\n";
		}

		if (!performance_rows_.empty()) {
			output << "\n## Performance Evidence\n\n";
			output << "| Benchmark | Workload | Records | Shared ops | Naive evals | Materialized evals | "
						  "Cache-miss evals | Naive memory bytes | Materialized memory bytes | Reduction | Exact | Notes |\n";
			output << "|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---|---|\n";
			for (const auto &row : performance_rows_) {
				output << "| " << detail::escape_cell(row.benchmark) << " | " << detail::escape_cell(row.workload)
					   << " | " << row.record_count << " | " << row.shared_operations << " | "
					   << row.naive_distance_evaluations << " | " << row.materialized_distance_evaluations << " | "
					   << row.cache_miss_evaluations << " | " << row.naive_memory_bytes_estimate << " | "
					   << row.materialized_memory_bytes_estimate << " | "
					   << detail::format_double(row.evaluation_reduction_factor) << " | "
					   << (row.exact_match ? "yes" : "no") << " | " << detail::escape_cell(row.notes) << " |\n";
			}
		}

		if (!benchmark_targets_.empty()) {
			output << "\n## Benchmark Targets\n\n";
			output << "| Benchmark | Workload | Records | Queries | Target | Max evals | Max dense slots | "
						  "Expected representation | Exact | Automated | Notes |\n";
			output << "|---|---|---:|---:|---|---:|---:|---|---|---|---|\n";
			for (const auto &row : benchmark_targets_) {
				output << "| " << detail::escape_cell(row.benchmark) << " | " << detail::escape_cell(row.workload)
					   << " | " << row.record_count << " | " << row.query_count << " | "
					   << detail::escape_cell(row.target) << " | " << row.max_distance_evaluations << " | "
					   << row.max_dense_slots << " | " << detail::escape_cell(row.expected_representation)
					   << " | " << (row.exact ? "yes" : "no") << " | " << (row.automated ? "yes" : "no")
					   << " | " << detail::escape_cell(row.notes) << " |\n";
			}
		}

		if (!benchmark_trends_.empty()) {
			output << "\n## Benchmark Trends\n\n";
			output << "| Benchmark | Workload | Records | Queries | Baseline evals | Observed evals | Cached | "
						  "Dense slots | Representation | Exact | Passed | Notes |\n";
			output << "|---|---|---:|---:|---:|---:|---:|---:|---|---|---|---|\n";
			for (const auto &row : benchmark_trends_) {
				output << "| " << detail::escape_cell(row.benchmark) << " | " << detail::escape_cell(row.workload)
					   << " | " << row.record_count << " | " << row.query_count << " | "
					   << row.baseline_distance_evaluations << " | " << row.observed_distance_evaluations << " | "
					   << row.cached_distances << " | " << row.dense_distance_slots << " | "
					   << detail::escape_cell(row.representation) << " | " << (row.exact ? "yes" : "no")
					   << " | " << (row.passed ? "yes" : "no") << " | " << detail::escape_cell(row.notes)
					   << " |\n";
			}
		}

		if (!wall_clock_trends_.empty()) {
			output << "\n## Wall-clock Trends\n\n";
			output << "| Benchmark | Workload | Records | Representation | Elapsed ns | Distance evals | "
					  "Memory bytes | Samples | Platform | Automated | Passed | Notes |\n";
			output << "|---|---|---:|---|---:|---:|---:|---:|---|---|---|---|\n";
			for (const auto &row : wall_clock_trends_) {
				output << "| " << detail::escape_cell(row.benchmark) << " | " << detail::escape_cell(row.workload)
					   << " | " << row.record_count << " | " << detail::escape_cell(row.representation) << " | "
					   << row.elapsed_ns << " | " << row.distance_evaluations << " | "
					   << row.memory_bytes_estimate << " | " << row.sample_count << " | "
					   << detail::escape_cell(row.platform_label) << " | " << (row.automated ? "yes" : "no")
					   << " | " << (row.passed ? "yes" : "no") << " | " << detail::escape_cell(row.notes)
					   << " |\n";
			}
		}

		if (!scale_guardrails_.empty()) {
			output << "\n## Scale Guardrails\n\n";
			output << "| Benchmark | Workload | Records | Shared ops | Exact plan evals | Naive workflow evals | "
					  "Dense slots | Max evals | Expected evals | Observed evals | Decision | Representation | "
					  "Exact | Passed | Notes |\n";
			output << "|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---|---|---|---|---|\n";
			for (const auto &row : scale_guardrails_) {
				output << "| " << detail::escape_cell(row.benchmark) << " | " << detail::escape_cell(row.workload)
					   << " | " << row.record_count << " | " << row.shared_operations << " | "
					   << row.exact_plan_distance_evaluations << " | "
					   << row.naive_workflow_distance_evaluations << " | " << row.dense_distance_slots << " | "
					   << row.max_distance_evaluations << " | " << row.expected_distance_evaluations << " | "
					   << row.observed_distance_evaluations << " | " << detail::escape_cell(row.decision) << " | "
					   << detail::escape_cell(row.representation) << " | " << (row.exact ? "yes" : "no")
					   << " | " << (row.passed ? "yes" : "no") << " | " << detail::escape_cell(row.notes)
					   << " |\n";
			}
		}

		if (!out_of_core_readiness_.empty()) {
			output << "\n## Out-of-core Readiness\n\n";
			output << "| Benchmark | Workload | Records | Dense slots | Dense memory bytes | Memory budget bytes | "
					  "Spill block bytes | Planned spill blocks | Max resident blocks | Spill enabled | "
					  "Explicit policy required | Decision | Automated | Passed | Notes |\n";
			output << "|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|---|---|---|---|\n";
			for (const auto &row : out_of_core_readiness_) {
				output << "| " << detail::escape_cell(row.benchmark) << " | " << detail::escape_cell(row.workload)
					   << " | " << row.record_count << " | " << row.dense_distance_slots << " | "
					   << row.dense_memory_bytes_estimate << " | " << row.memory_budget_bytes << " | "
					   << row.spill_block_bytes << " | " << row.planned_spill_blocks << " | "
					   << row.max_resident_blocks << " | " << (row.spill_enabled ? "yes" : "no")
					   << " | " << (row.explicit_policy_required ? "yes" : "no") << " | "
					   << detail::escape_cell(row.decision) << " | " << (row.automated ? "yes" : "no")
					   << " | " << (row.passed ? "yes" : "no") << " | " << detail::escape_cell(row.notes)
					   << " |\n";
			}
		}

		return output.str();
	}

  private:
	std::string title_;
	BenchmarkRunMetadata run_metadata_;
	std::vector<RepresentationCostRow> representation_costs_;
	std::vector<WorkflowEvidenceRow> workflow_evidence_;
	std::vector<PerformanceRow> performance_rows_;
	std::vector<BenchmarkTargetRow> benchmark_targets_;
	std::vector<BenchmarkTrendRow> benchmark_trends_;
	std::vector<WallClockTrendRow> wall_clock_trends_;
	std::vector<ScaleGuardrailRow> scale_guardrails_;
	std::vector<OutOfCoreReadinessRow> out_of_core_readiness_;
};

} // namespace mtrc::benchmarks

#endif
