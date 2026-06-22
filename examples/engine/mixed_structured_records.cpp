#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/engine.hpp>

namespace {

struct IndustrialRecord {
	std::array<double, 3> temperature_summary{};
	std::string status;
	std::string message;
	std::array<double, 4> spectrum{};
	std::vector<double> curve;
};

struct MixedBenchmarkQuery {
	std::string expected_name;
	IndustrialRecord record;
};

struct MixedRecordDistance {
	auto operator()(const IndustrialRecord &lhs, const IndustrialRecord &rhs) const -> double
	{
		const auto fields = contributions(lhs, rhs);
		return std::accumulate(fields.begin(), fields.end(), 0.0,
							   [](double total, const auto &entry) { return total + entry.second; });
	}

	auto contributions(const IndustrialRecord &lhs, const IndustrialRecord &rhs) const -> std::map<std::string, double>
	{
		const auto message_scale =
			static_cast<double>(std::max({lhs.message.size(), rhs.message.size(), std::size_t{1}}));
		const auto curve_scale = static_cast<double>(std::max({lhs.curve.size(), rhs.curve.size(), std::size_t{1}}));

		return {
			{"temperature_summary", 0.20 * euclidean(lhs.temperature_summary, rhs.temperature_summary)},
			{"status", 0.15 * (lhs.status == rhs.status ? 0.0 : 2.0)},
			{"message", 0.15 * static_cast<double>(edit_distance(lhs.message, rhs.message)) / message_scale},
			{"spectrum", 0.25 * transport_distance(lhs.spectrum, rhs.spectrum)},
			{"curve", 0.25 * aligned_curve_distance(lhs.curve, rhs.curve) / curve_scale},
		};
	}

  private:
	static auto euclidean(const std::array<double, 3> &lhs, const std::array<double, 3> &rhs) -> double
	{
		double squared = 0.0;
		for (std::size_t index = 0; index < lhs.size(); ++index) {
			const auto delta = lhs[index] - rhs[index];
			squared += delta * delta;
		}
		return std::sqrt(squared);
	}

	static auto edit_distance(const std::string &lhs, const std::string &rhs) -> std::size_t
	{
		std::vector<std::size_t> previous(rhs.size() + 1);
		std::iota(previous.begin(), previous.end(), std::size_t{0});
		for (std::size_t lhs_index = 1; lhs_index <= lhs.size(); ++lhs_index) {
			std::vector<std::size_t> current(rhs.size() + 1);
			current[0] = lhs_index;
			for (std::size_t rhs_index = 1; rhs_index <= rhs.size(); ++rhs_index) {
				const auto substitute = previous[rhs_index - 1] + (lhs[lhs_index - 1] == rhs[rhs_index - 1] ? 0 : 1);
				const auto deletion = previous[rhs_index] + 1;
				const auto insertion = current[rhs_index - 1] + 1;
				current[rhs_index] = std::min({substitute, deletion, insertion});
			}
			previous = std::move(current);
		}
		return previous.back();
	}

	static auto transport_distance(const std::array<double, 4> &lhs, const std::array<double, 4> &rhs) -> double
	{
		double cumulative_delta = 0.0;
		double distance = 0.0;
		for (std::size_t index = 0; index < lhs.size(); ++index) {
			cumulative_delta += lhs[index] - rhs[index];
			distance += std::abs(cumulative_delta);
		}
		return distance;
	}

	static auto aligned_curve_distance(const std::vector<double> &lhs, const std::vector<double> &rhs) -> double
	{
		constexpr double gap_cost = 2.0;
		std::vector<double> previous(rhs.size() + 1);
		for (std::size_t index = 0; index < previous.size(); ++index) {
			previous[index] = static_cast<double>(index) * gap_cost;
		}

		for (std::size_t lhs_index = 1; lhs_index <= lhs.size(); ++lhs_index) {
			std::vector<double> current(rhs.size() + 1);
			current[0] = static_cast<double>(lhs_index) * gap_cost;
			for (std::size_t rhs_index = 1; rhs_index <= rhs.size(); ++rhs_index) {
				const auto substitute =
					previous[rhs_index - 1] + std::min(std::abs(lhs[lhs_index - 1] - rhs[rhs_index - 1]), 2 * gap_cost);
				const auto deletion = previous[rhs_index] + gap_cost;
				const auto insertion = current[rhs_index - 1] + gap_cost;
				current[rhs_index] = std::min({substitute, deletion, insertion});
			}
			previous = std::move(current);
		}
		return previous.back();
	}
};

struct FlatVectorDistance {
	auto operator()(const std::vector<double> &lhs, const std::vector<double> &rhs) const -> double
	{
		if (lhs.size() != rhs.size()) {
			throw std::invalid_argument("flat vectors must have equal size");
		}
		double squared = 0.0;
		for (std::size_t index = 0; index < lhs.size(); ++index) {
			const auto delta = lhs[index] - rhs[index];
			squared += delta * delta;
		}
		return std::sqrt(squared);
	}
};

auto make_records() -> std::vector<IndustrialRecord>
{
	return {
		{{68.0, 69.0, 70.0}, "stable", "nominal flow", {0.6, 0.3, 0.1, 0.0}, {1.0, 1.0, 1.1, 1.0}},
		{{72.0, 73.0, 75.0}, "warmup", "valve drift warning", {0.1, 0.3, 0.4, 0.2}, {1.0, 1.2, 1.5, 1.4, 1.2}},
		{{72.5, 73.2, 74.5}, "warmup", "valve drift alert", {0.1, 0.25, 0.45, 0.2}, {1.0, 1.2, 1.4, 1.4, 1.3}},
		{{64.0, 63.5, 63.0}, "cooldown", "pump normal", {0.0, 0.2, 0.5, 0.3}, {2.0, 1.7, 1.4, 1.1}},
		{{91.0, 96.0, 101.0}, "alert", "pressure spike stop", {0.0, 0.0, 0.2, 0.8}, {1.0, 6.5, 1.0, 6.0, 1.0}},
	};
}

auto make_query() -> IndustrialRecord
{
	return {{72.2, 73.1, 75.1}, "warmup", "valve drift warning", {0.1, 0.3, 0.4, 0.2}, {1.0, 1.2, 1.5, 1.4, 1.3}};
}

auto make_benchmark_names() -> std::vector<std::string>
{
	return {
		"warmup-valve-drift",	 "stable-valve-shadow",	  "cooldown-pump-cavitation", "warmup-cavitation-shadow",
		"alert-bearing-chatter", "stable-bearing-shadow", "filter-clog-drift",		  "stable-filter-shadow",
	};
}

auto make_benchmark_records() -> std::vector<IndustrialRecord>
{
	return {
		{{72.0, 73.0, 75.0}, "warmup", "valve drift warning", {0.1, 0.3, 0.4, 0.2}, {1.0, 1.2, 1.5, 1.4, 1.2}},
		{{72.2, 73.1, 75.1}, "stable", "nominal flow", {0.1, 0.3, 0.4, 0.2}, {1.0, 1.2, 1.5, 1.4, 1.3}},
		{{63.8, 63.1, 62.5}, "cooldown", "pump cavitation warning", {0.0, 0.2, 0.55, 0.25}, {2.0, 1.7, 1.3, 1.0}},
		{{64.0, 63.2, 62.7}, "warmup", "warmup normal", {0.0, 0.2, 0.55, 0.25}, {2.0, 1.6, 1.35, 1.05}},
		{{88.0, 92.0, 97.0}, "alert", "bearing chatter alert", {0.0, 0.05, 0.25, 0.7}, {1.0, 5.9, 1.2, 5.8, 1.1}},
		{{88.3, 92.1, 96.8}, "stable", "nominal flow", {0.0, 0.05, 0.25, 0.7}, {1.0, 6.0, 1.2, 5.8, 1.0}},
		{{70.0, 71.0, 72.0}, "degraded", "filter clog warning", {0.3, 0.4, 0.2, 0.1}, {1.5, 1.6, 1.8, 2.1, 2.0}},
		{{70.1, 71.1, 72.2}, "stable", "nominal flow", {0.3, 0.4, 0.2, 0.1}, {1.5, 1.7, 1.9, 2.1, 2.0}},
	};
}

auto make_benchmark_queries() -> std::vector<MixedBenchmarkQuery>
{
	return {
		{"warmup-valve-drift",
		 {{72.2, 73.1, 75.1}, "warmup", "valve drift warning", {0.1, 0.3, 0.4, 0.2}, {1.0, 1.2, 1.5, 1.4, 1.3}}},
		{"cooldown-pump-cavitation",
		 {{64.0, 63.2, 62.7}, "cooldown", "pump cavitation warning", {0.0, 0.2, 0.55, 0.25}, {2.0, 1.6, 1.35, 1.05}}},
		{"alert-bearing-chatter",
		 {{88.3, 92.1, 96.8}, "alert", "bearing chatter alert", {0.0, 0.05, 0.25, 0.7}, {1.0, 6.0, 1.2, 5.8, 1.0}}},
		{"filter-clog-drift",
		 {{70.1, 71.1, 72.2}, "degraded", "filter clog warning", {0.3, 0.4, 0.2, 0.1}, {1.5, 1.7, 1.9, 2.1, 2.0}}},
	};
}

auto flat_numeric_projection(const IndustrialRecord &record) -> std::vector<double>
{
	constexpr std::size_t curve_width = 5;
	std::vector<double> result;
	result.reserve(record.temperature_summary.size() + record.spectrum.size() + curve_width);
	result.insert(result.end(), record.temperature_summary.begin(), record.temperature_summary.end());
	result.insert(result.end(), record.spectrum.begin(), record.spectrum.end());
	for (std::size_t index = 0; index < curve_width; ++index) {
		result.push_back(index < record.curve.size() ? record.curve[index] : 0.0);
	}
	return result;
}

auto flat_numeric_projection(const std::vector<IndustrialRecord> &records) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> result;
	result.reserve(records.size());
	for (const auto &record : records) {
		result.push_back(flat_numeric_projection(record));
	}
	return result;
}

auto close_to(double lhs, double rhs) -> bool { return std::abs(lhs - rhs) < 1.0e-9; }

} // namespace

int main()
{
	const std::vector<std::string> names = {"stable-flow", "warmup-drift", "warmup-alert", "cooldown", "spike-stop"};
	const auto records = make_records();
	const auto query = make_query();
	const MixedRecordDistance metric;
	auto space = mtrc::make_space(records, metric);

	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);
	const auto warmup_distance = matrix.distance(space.id(1), space.id(2));
	assert(warmup_distance < matrix.distance(space.id(1), space.id(4)));

	const auto neighbors = mtrc::find_neighbors(space, query, mtrc::count{2});
	assert(neighbors.representation == "metric_space");
	assert(neighbors.size() == 2);
	assert(names[neighbors[0].id.index()] == "warmup-drift");
	assert(names[neighbors[1].id.index()] == "warmup-alert");

	const auto explanation = metric.contributions(query, space.record(neighbors[0].id));
	assert(close_to(explanation.at("status"), 0.0));
	assert(explanation.at("curve") < 0.01);

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2), materialized_policy);
	assert(groups.algorithm == "kmedoids");
	assert(groups.cluster_count == 2);
	assert(groups.representation == "distance_table");

	const auto outliers = mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(8.0, 2), materialized_policy);
	assert(outliers.strategy == "dbscan_noise");
	assert(outliers.representation == "distance_table");
	assert(outliers.size() == 1);
	assert(names[outliers[0].id.index()] == "spike-stop");

	const auto diagnostics =
		mtrc::space::storage::diagnostics_for_space(space, materialized_policy, "distance_table", "mixed_record_demo");
	assert(diagnostics.policy_name == "exact_materialized_serial");
	assert(diagnostics.representation == "distance_table");
	assert(diagnostics.supported);

	const auto benchmark_names = make_benchmark_names();
	const auto benchmark_records = make_benchmark_records();
	const auto benchmark_queries = make_benchmark_queries();
	assert(benchmark_names.size() == benchmark_records.size());

	auto benchmark_space = mtrc::make_space(benchmark_records, metric);
	auto flat_space = mtrc::make_space(flat_numeric_projection(benchmark_records), FlatVectorDistance{});
	std::size_t benchmark_metric_correct = 0;
	std::size_t benchmark_vector_mismatches = 0;
	double total_metric_margin = 0.0;

	for (const auto &benchmark_query : benchmark_queries) {
		const auto metric_neighbors = mtrc::find_neighbors(benchmark_space, benchmark_query.record, mtrc::count{1});
		assert(metric_neighbors.size() == 1);
		const auto metric_index = metric_neighbors[0].id.index();
		const auto metric_name = benchmark_names[metric_index];
		assert(metric_name == benchmark_query.expected_name);
		++benchmark_metric_correct;

		const auto flat_query = flat_numeric_projection(benchmark_query.record);
		const auto flat_neighbors = mtrc::find_neighbors(flat_space, flat_query, mtrc::count{1});
		assert(flat_neighbors.size() == 1);
		const auto flat_index = flat_neighbors[0].id.index();
		const auto flat_name = benchmark_names[flat_index];
		assert(flat_name != benchmark_query.expected_name);
		++benchmark_vector_mismatches;

		const auto flat_metric_distance = metric(benchmark_query.record, benchmark_records[flat_index]);
		const auto margin = flat_metric_distance - metric_neighbors[0].distance;
		assert(margin > 0.0);
		total_metric_margin += margin;
	}

	const auto average_metric_margin = total_metric_margin / static_cast<double>(benchmark_queries.size());
	assert(average_metric_margin > 0.0);

	std::cout << "nearest mixed records = " << names[neighbors[0].id.index()] << ", " << names[neighbors[1].id.index()]
			  << "\n";
	std::cout << "warmup distance = " << warmup_distance << "\n";
	std::cout << "message contribution = " << explanation.at("message") << "\n";
	std::cout << "mixed record groups = " << groups.cluster_count << " via " << groups.representation << "\n";
	std::cout << "mixed record outlier = " << names[outliers[0].id.index()] << "\n";
	std::cout << "runtime policy = " << diagnostics.policy_name << " via " << diagnostics.representation << "\n";
	std::cout << "mixed benchmark records = " << benchmark_records.size() << "\n";
	std::cout << "mixed benchmark queries = " << benchmark_queries.size() << "\n";
	std::cout << "mixed benchmark metric correct = " << benchmark_metric_correct << "/" << benchmark_queries.size()
			  << "\n";
	std::cout << "mixed benchmark vector mismatches = " << benchmark_vector_mismatches << "/"
			  << benchmark_queries.size() << "\n";
	std::cout << "mixed benchmark average metric margin = " << average_metric_margin << "\n";
	std::cout << "mixed benchmark query evaluations = " << benchmark_records.size() * benchmark_queries.size() << "\n";

	return 0;
}
