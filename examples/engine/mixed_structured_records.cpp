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

struct MixedRecordDistance {
	auto operator()(const IndustrialRecord &lhs, const IndustrialRecord &rhs) const -> double
	{
		const auto fields = contributions(lhs, rhs);
		return std::accumulate(fields.begin(), fields.end(), 0.0,
							   [](double total, const auto &entry) { return total + entry.second; });
	}

	auto contributions(const IndustrialRecord &lhs, const IndustrialRecord &rhs) const
		-> std::map<std::string, double>
	{
		const auto message_scale = static_cast<double>(std::max({lhs.message.size(), rhs.message.size(), std::size_t{1}}));
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
				const auto substitute =
					previous[rhs_index - 1] + (lhs[lhs_index - 1] == rhs[rhs_index - 1] ? 0 : 1);
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
	return {{72.2, 73.1, 75.1},
			"warmup",
			"valve drift warning",
			{0.1, 0.3, 0.4, 0.2},
			{1.0, 1.2, 1.5, 1.4, 1.3}};
}

auto close_to(double lhs, double rhs) -> bool
{
	return std::abs(lhs - rhs) < 1.0e-9;
}

} // namespace

int main()
{
	const std::vector<std::string> names = {"stable-flow", "warmup-drift", "warmup-alert", "cooldown", "spike-stop"};
	const auto records = make_records();
	const auto query = make_query();
	const MixedRecordDistance metric;
	auto space = metric::make_space(records, metric);

	metric::representations::MatrixCache<decltype(space)> matrix(space);
	const auto warmup_distance = matrix.distance(space.id(1), space.id(2));
	assert(warmup_distance < matrix.distance(space.id(1), space.id(4)));

	const auto neighbors = metric::find_neighbors(space, query, metric::count{2});
	assert(neighbors.representation == "metric_space");
	assert(neighbors.size() == 2);
	assert(names[neighbors[0].id.index()] == "warmup-drift");
	assert(names[neighbors[1].id.index()] == "warmup-alert");

	const auto explanation = metric.contributions(query, space.record(neighbors[0].id));
	assert(close_to(explanation.at("status"), 0.0));
	assert(explanation.at("curve") < 0.01);

	const auto materialized_policy = metric::runtime::materialized(metric::runtime::exact());
	const auto groups = metric::find_groups(space, metric::strategies::k_medoids(2), materialized_policy);
	assert(groups.algorithm == "kmedoids");
	assert(groups.cluster_count == 2);
	assert(groups.representation == "matrix_cache");

	const auto outliers = metric::find_outliers(space, metric::strategies::dbscan(8.0, 2), materialized_policy);
	assert(outliers.strategy == "dbscan_noise");
	assert(outliers.representation == "matrix_cache");
	assert(outliers.size() == 1);
	assert(names[outliers[0].id.index()] == "spike-stop");

	const auto diagnostics =
		metric::runtime::diagnostics_for_space(space, materialized_policy, "matrix_cache", "mixed_record_demo");
	assert(diagnostics.policy_name == "exact_materialized_serial");
	assert(diagnostics.representation == "matrix_cache");
	assert(diagnostics.supported);

	std::cout << "nearest mixed records = " << names[neighbors[0].id.index()] << ", "
			  << names[neighbors[1].id.index()] << "\n";
	std::cout << "warmup distance = " << warmup_distance << "\n";
	std::cout << "message contribution = " << explanation.at("message") << "\n";
	std::cout << "mixed record groups = " << groups.cluster_count << " via " << groups.representation << "\n";
	std::cout << "mixed record outlier = " << names[outliers[0].id.index()] << "\n";
	std::cout << "runtime policy = " << diagnostics.policy_name << " via " << diagnostics.representation << "\n";

	return 0;
}
