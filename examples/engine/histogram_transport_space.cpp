#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/engine.hpp>

namespace {

using Histogram = std::vector<double>;

struct HistogramBenchmarkQuery {
	Histogram values;
	std::string expected_family;
};

struct CumulativeTransportDistance {
	auto operator()(const Histogram &lhs, const Histogram &rhs) const -> double
	{
		if (lhs.size() != rhs.size()) {
			throw std::invalid_argument("histograms must have the same number of bins");
		}

		double cumulative_delta = 0.0;
		double distance = 0.0;
		for (std::size_t index = 0; index < lhs.size(); ++index) {
			cumulative_delta += lhs[index] - rhs[index];
			distance += std::abs(cumulative_delta);
		}
		return distance;
	}
};

auto spike(std::size_t bin_count, std::size_t index) -> Histogram
{
	Histogram values(bin_count, 0.0);
	values[index] = 1.0;
	return values;
}

auto split_spike(std::size_t bin_count, std::size_t anchor, std::size_t far_bin) -> Histogram
{
	Histogram values(bin_count, 0.0);
	values[anchor] = 0.5;
	values[far_bin] = 0.5;
	return values;
}

auto close_to(double lhs, double rhs) -> bool { return std::abs(lhs - rhs) < 1e-12; }

} // namespace

int main()
{
	constexpr std::size_t bin_count = 12;
	const std::vector<Histogram> records = {
		spike(bin_count, 1),		  spike(bin_count, 4),			 spike(bin_count, 7),
		spike(bin_count, 10),		  split_spike(bin_count, 0, 11), split_spike(bin_count, 3, 11),
		split_spike(bin_count, 6, 0), split_spike(bin_count, 9, 0),
	};
	const std::vector<std::string> names = {
		"shifted_mass_0", "shifted_mass_3", "shifted_mass_6", "shifted_mass_9",
		"vector_decoy_0", "vector_decoy_3", "vector_decoy_6", "vector_decoy_9",
	};
	const std::vector<std::string> families = {
		"shift_0", "shift_3", "shift_6", "shift_9", "decoy", "decoy", "decoy", "decoy",
	};

	auto space = mtrc::make_space(records, CumulativeTransportDistance{});
	auto vector_baseline = mtrc::make_space(records, mtrc::Euclidean<double>{});

	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);
	assert(matrix.diagnostics().cached_distances == records.size() * records.size());
	assert(close_to(matrix.distance(space.id(0), space.id(0)), 0.0));
	assert(close_to(matrix.distance(space.id(0), space.id(1)), 3.0));
	assert(close_to(matrix.distance(space.id(1), space.id(2)), 3.0));

	const std::vector<HistogramBenchmarkQuery> benchmark_queries = {
		{spike(bin_count, 0), "shift_0"},
		{spike(bin_count, 3), "shift_3"},
		{spike(bin_count, 6), "shift_6"},
		{spike(bin_count, 9), "shift_9"},
	};

	const auto &query = benchmark_queries.front().values;
	const auto neighbors = mtrc::find_neighbors(space, query, 2);
	assert(neighbors.size() == 2);
	assert(neighbors[0].id == space.id(0));
	assert(close_to(neighbors[0].distance, 1.0));

	const auto first_vector_neighbors = mtrc::find_neighbors(vector_baseline, query, mtrc::count{1});
	assert(first_vector_neighbors.size() == 1);
	assert(first_vector_neighbors[0].id == space.id(4));
	assert(close_to(first_vector_neighbors[0].distance, std::sqrt(0.5)));

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto materialized_neighbors = mtrc::find_neighbors(space, space.id(0), 2, materialized_policy);
	assert(materialized_neighbors.representation == "distance_table");
	assert(materialized_neighbors.size() == 2);

	const auto groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(4));
	assert(groups.algorithm == "kmedoids");
	assert(groups.cluster_count == 4);

	std::size_t metric_correct = 0;
	std::size_t vector_mismatches = 0;
	double metric_margin_sum = 0.0;

	for (const auto &benchmark_query : benchmark_queries) {
		const auto metric_neighbors = mtrc::find_neighbors(space, benchmark_query.values, mtrc::count{1});
		const auto vector_neighbors = mtrc::find_neighbors(vector_baseline, benchmark_query.values, mtrc::count{1});

		assert(metric_neighbors.size() == 1);
		assert(vector_neighbors.size() == 1);

		const auto metric_family = families[metric_neighbors[0].id.index()];
		const auto vector_family = families[vector_neighbors[0].id.index()];
		assert(metric_family == benchmark_query.expected_family);
		assert(vector_family != benchmark_query.expected_family);

		const auto vector_winner_under_metric =
			space.metric()(benchmark_query.values, records[vector_neighbors[0].id.index()]);
		const auto metric_margin = vector_winner_under_metric - metric_neighbors[0].distance;
		assert(metric_margin > 0.0);

		++metric_correct;
		++vector_mismatches;
		metric_margin_sum += metric_margin;
	}

	const auto average_metric_margin = metric_margin_sum / static_cast<double>(benchmark_queries.size());
	assert(close_to(average_metric_margin, 3.25));

	std::cout << "nearest histograms = " << names[neighbors[0].id.index()] << ", " << names[neighbors[1].id.index()]
			  << "\n";
	std::cout << "histogram runtime representation = " << materialized_neighbors.representation << "\n";
	std::cout << "histogram benchmark metric winner = " << names[neighbors[0].id.index()] << " at "
			  << neighbors[0].distance << "\n";
	std::cout << "histogram benchmark vector baseline winner = " << names[first_vector_neighbors[0].id.index()]
			  << " at " << first_vector_neighbors[0].distance << "\n";
	std::cout << "histogram benchmark baseline mismatch = yes\n";
	std::cout << "histogram benchmark records = " << records.size() << "\n";
	std::cout << "histogram benchmark queries = " << benchmark_queries.size() << "\n";
	std::cout << "histogram benchmark metric correct = " << metric_correct << "/" << benchmark_queries.size() << "\n";
	std::cout << "histogram benchmark vector mismatches = " << vector_mismatches << "/" << benchmark_queries.size()
			  << "\n";
	std::cout << "histogram benchmark average metric margin = " << average_metric_margin << "\n";
	std::cout << "histogram benchmark dense evaluations = " << matrix.diagnostics().cached_distances << "\n";

	return 0;
}
