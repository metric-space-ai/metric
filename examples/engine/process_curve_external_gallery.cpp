#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/engine.hpp>

#ifndef METRIC_PROCESS_CURVE_POWER_DEMAND_CSV
#define METRIC_PROCESS_CURVE_POWER_DEMAND_CSV "assets/process_curve_power_demand_gallery.csv"
#endif

#ifndef METRIC_PROCESS_CURVE_INTERNAL_BLEEDING_CSV
#define METRIC_PROCESS_CURVE_INTERNAL_BLEEDING_CSV "assets/process_curve_internal_bleeding_gallery.csv"
#endif

namespace {

struct ExternalProcessWindow {
	std::string id;
	std::string source_label;
	std::vector<double> values;
};

struct ProcessCurveQuery {
	std::string id;
	std::string expected_label;
	std::vector<double> values;
};

struct DomainBenchmarkInput {
	std::string name;
	std::string csv_path;
	std::size_t expected_records;
	std::size_t expected_queries;
	double minimum_average_margin;
};

struct DomainBenchmarkSummary {
	std::string name;
	std::size_t records;
	std::size_t queries;
	std::size_t metric_correct;
	std::size_t vector_mismatches;
	double average_metric_margin;
	std::string first_query;
	std::string first_metric_winner;
	std::string first_vector_winner;
	double first_metric_distance;
	double first_vector_distance;
	std::size_t dense_evaluations;
};

struct AlignedCurveDistance {
	double gap_cost{18.0};

	auto operator()(const std::vector<double> &lhs, const std::vector<double> &rhs) const -> double
	{
		std::vector<double> previous(rhs.size() + 1, 0.0);
		for (std::size_t index = 0; index < previous.size(); ++index) {
			previous[index] = static_cast<double>(index) * gap_cost;
		}

		for (std::size_t lhs_index = 1; lhs_index <= lhs.size(); ++lhs_index) {
			std::vector<double> current(rhs.size() + 1, 0.0);
			current[0] = static_cast<double>(lhs_index) * gap_cost;
			for (std::size_t rhs_index = 1; rhs_index <= rhs.size(); ++rhs_index) {
				const auto substitution =
					previous[rhs_index - 1] + std::min(std::abs(lhs[lhs_index - 1] - rhs[rhs_index - 1]), 2 * gap_cost);
				const auto deletion = previous[rhs_index] + gap_cost;
				const auto insertion = current[rhs_index - 1] + gap_cost;
				current[rhs_index] = std::min({substitution, deletion, insertion});
			}
			previous = std::move(current);
		}

		return previous.back();
	}
};

struct PointwisePaddedEuclideanDistance {
	auto operator()(const std::vector<double> &lhs, const std::vector<double> &rhs) const -> double
	{
		const auto size = std::max(lhs.size(), rhs.size());
		double squared_sum = 0.0;
		for (std::size_t index = 0; index < size; ++index) {
			const auto left = index < lhs.size() ? lhs[index] : 0.0;
			const auto right = index < rhs.size() ? rhs[index] : 0.0;
			const auto delta = left - right;
			squared_sum += delta * delta;
		}
		return std::sqrt(squared_sum);
	}
};

auto parse_values(std::string value_text) -> std::vector<double>
{
	if (value_text.size() >= 2 && value_text.front() == '"' && value_text.back() == '"') {
		value_text = value_text.substr(1, value_text.size() - 2);
	}

	std::istringstream stream(value_text);
	std::vector<double> values;
	double value = 0.0;
	while (stream >> value) {
		values.push_back(value);
	}
	return values;
}

auto split_csv_line(const std::string &line) -> std::vector<std::string>
{
	std::vector<std::string> fields;
	std::string field;
	bool in_quotes = false;
	for (const char character : line) {
		if (character == '"') {
			in_quotes = !in_quotes;
			field.push_back(character);
		} else if (character == ',' && !in_quotes) {
			fields.push_back(field);
			field.clear();
		} else {
			field.push_back(character);
		}
	}
	fields.push_back(field);
	return fields;
}

auto load_external_windows(const std::string &path) -> std::vector<ExternalProcessWindow>
{
	std::ifstream input(path);
	if (!input) {
		throw std::runtime_error("unable to open process-curve gallery CSV: " + path);
	}

	std::string line;
	std::getline(input, line);
	std::vector<ExternalProcessWindow> records;
	while (std::getline(input, line)) {
		if (line.empty()) {
			continue;
		}
		auto fields = split_csv_line(line);
		if (fields.size() != 6) {
			throw std::runtime_error("unexpected process-curve gallery CSV row: " + line);
		}
		records.push_back(ExternalProcessWindow{fields[0], fields[4], parse_values(fields[5])});
	}
	return records;
}

auto downsample_even(const std::vector<double> &values) -> std::vector<double>
{
	std::vector<double> query;
	for (std::size_t index = 0; index < values.size(); index += 2) {
		query.push_back(values[index]);
	}
	return query;
}

auto contains(const std::string &text, const std::string &needle) -> bool
{
	return text.find(needle) != std::string::npos;
}

auto run_domain_benchmark(const DomainBenchmarkInput &input) -> DomainBenchmarkSummary
{
	const auto windows = load_external_windows(input.csv_path);
	assert(windows.size() == input.expected_records);

	std::vector<std::string> ids;
	std::vector<std::string> labels;
	std::vector<std::vector<double>> records;
	std::vector<ProcessCurveQuery> queries;
	for (const auto &window : windows) {
		ids.push_back(window.id);
		labels.push_back(window.source_label);
		records.push_back(window.values);
		assert(window.values.size() == 36);
		if (contains(window.id, "anomaly_start") || contains(window.id, "recovery")) {
			queries.push_back(
				ProcessCurveQuery{"downsampled_" + window.id, window.source_label, downsample_even(window.values)});
		}
	}
	assert(queries.size() == input.expected_queries);

	auto alignment_space = mtrc::make_space(records, AlignedCurveDistance{});
	auto vector_baseline = mtrc::make_space(records, PointwisePaddedEuclideanDistance{});

	mtrc::space::storage::DistanceTable<decltype(alignment_space)> matrix(alignment_space);
	const auto matrix_diagnostics = matrix.diagnostics();
	assert(matrix_diagnostics.cached_distances == records.size() * records.size());

	std::size_t metric_correct = 0;
	std::size_t vector_mismatches = 0;
	double metric_margin_sum = 0.0;
	std::string first_metric_winner;
	std::string first_vector_winner;
	double first_metric_distance = 0.0;
	double first_vector_distance = 0.0;

	for (std::size_t query_index = 0; query_index < queries.size(); ++query_index) {
		const auto &query = queries[query_index];
		assert(query.values.size() == 18);
		const auto metric_neighbors = mtrc::find_neighbors(alignment_space, query.values, 1);
		const auto baseline_neighbors = mtrc::find_neighbors(vector_baseline, query.values, 1);
		assert(metric_neighbors.size() == 1);
		assert(baseline_neighbors.size() == 1);

		const auto metric_label = labels[metric_neighbors[0].id.index()];
		const auto baseline_label = labels[baseline_neighbors[0].id.index()];
		if (metric_label == query.expected_label) {
			++metric_correct;
		}
		if (baseline_label != query.expected_label) {
			++vector_mismatches;
		}

		const auto baseline_distance_under_metric =
			alignment_space.metric()(query.values, records[baseline_neighbors[0].id.index()]);
		const auto metric_margin = baseline_distance_under_metric - metric_neighbors[0].distance;
		assert(metric_margin > 0.0);
		metric_margin_sum += metric_margin;

		if (query_index == 0) {
			first_metric_winner = ids[metric_neighbors[0].id.index()];
			first_vector_winner = ids[baseline_neighbors[0].id.index()];
			first_metric_distance = metric_neighbors[0].distance;
			first_vector_distance = baseline_neighbors[0].distance;
		}
	}
	assert(metric_correct == queries.size());
	assert(vector_mismatches == queries.size());
	assert(metric_margin_sum / static_cast<double>(queries.size()) > input.minimum_average_margin);

	return DomainBenchmarkSummary{input.name,
								  records.size(),
								  queries.size(),
								  metric_correct,
								  vector_mismatches,
								  metric_margin_sum / static_cast<double>(queries.size()),
								  queries.front().id,
								  first_metric_winner,
								  first_vector_winner,
								  first_metric_distance,
								  first_vector_distance,
								  matrix_diagnostics.cached_distances};
}

} // namespace

int main()
{
	const std::vector<DomainBenchmarkInput> domains{
		{"power_demand", METRIC_PROCESS_CURVE_POWER_DEMAND_CSV, 24, 8, 300.0},
		{"internal_bleeding", METRIC_PROCESS_CURVE_INTERNAL_BLEEDING_CSV, 24, 8, 150.0},
	};

	std::vector<DomainBenchmarkSummary> summaries;
	std::size_t total_records = 0;
	std::size_t total_queries = 0;
	std::size_t total_dense_evaluations = 0;
	for (const auto &domain : domains) {
		auto summary = run_domain_benchmark(domain);
		total_records += summary.records;
		total_queries += summary.queries;
		total_dense_evaluations += summary.dense_evaluations;
		summaries.push_back(std::move(summary));
	}

	std::cout << "process external source = UCR_Time_Series_Anomaly_Detection_2021\n";
	std::cout << "process external license = CC BY 4.0\n";
	std::cout << "process external domains = " << summaries.size() << "\n";
	std::cout << "process external records = " << total_records << "\n";
	std::cout << "process external queries = " << total_queries << "\n";
	for (const auto &summary : summaries) {
		std::cout << "process external " << summary.name << " records = " << summary.records << "\n";
		std::cout << "process external " << summary.name << " queries = " << summary.queries << "\n";
		std::cout << "process external " << summary.name << " metric correct = " << summary.metric_correct << "/"
				  << summary.queries << "\n";
		std::cout << "process external " << summary.name << " vector mismatches = " << summary.vector_mismatches << "/"
				  << summary.queries << "\n";
		std::cout << "process external " << summary.name << " average metric margin = " << summary.average_metric_margin
				  << "\n";
		std::cout << "process external " << summary.name << " first query = " << summary.first_query << "\n";
		std::cout << "process external " << summary.name << " first metric winner = " << summary.first_metric_winner
				  << " at " << summary.first_metric_distance << "\n";
		std::cout << "process external " << summary.name << " first vector winner = " << summary.first_vector_winner
				  << " at " << summary.first_vector_distance << "\n";
		std::cout << "process external " << summary.name << " dense evaluations = " << summary.dense_evaluations
				  << "\n";
	}
	std::cout << "process external dense evaluations = " << total_dense_evaluations << "\n";

	return 0;
}
