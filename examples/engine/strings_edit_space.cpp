#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>

namespace {

using FeatureVector = std::vector<double>;

struct StringBenchmarkQuery {
	std::string text;
	std::string expected_family;
};

auto string_features(const std::string &value) -> FeatureVector
{
	FeatureVector features(27, 0.0);
	for (const auto ch : value) {
		if (ch >= 'a' && ch <= 'z') {
			features[static_cast<std::size_t>(ch - 'a')] += 1.0;
		}
	}
	features.back() = static_cast<double>(value.size());
	return features;
}

auto string_features(const std::vector<std::string> &values) -> std::vector<FeatureVector>
{
	std::vector<FeatureVector> features;
	features.reserve(values.size());
	for (const auto &value : values) {
		features.push_back(string_features(value));
	}
	return features;
}

auto close_to(double lhs, double rhs) -> bool { return std::fabs(lhs - rhs) < 1e-12; }

} // namespace

int main()
{
	const std::vector<std::string> records = {
		"metrics",	"matrix", "alerts",	 "alter",  "searches", "chaser",
		"listener", "silent", "rescued", "secure", "tree",	   "metric",
	};
	const std::vector<std::string> families = {
		"metric", "matrix",		   "alert",	 "anagram_decoy", "search", "anagram_decoy",
		"listen", "anagram_decoy", "rescue", "anagram_decoy", "tree",	"metric",
	};

	auto space = mtrc::make_space(records, mtrc::Edit<char>{});
	auto vector_baseline = mtrc::make_space(string_features(records), mtrc::Euclidean<double>{});

	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);
	assert(matrix.diagnostics().cached_distances == records.size() * records.size());
	assert(matrix.distance(space.id(0), space.id(11)) == 1);

	const auto neighbors =
		mtrc::find_neighbors(space, std::string("metricks"), mtrc::count{2}, mtrc::stats::search::cover_tree{});
	assert(neighbors.operator_name == "knn");
	assert(neighbors.representation == "cover_tree_index");
	assert(neighbors.size() == 2);
	assert(neighbors[0].id == space.id(0));
	assert(neighbors[0].distance == 1);

	const auto groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(4));
	assert(groups.algorithm == "kmedoids");
	assert(groups.cluster_count == 4);
	assert(groups.record_count == records.size());

	const std::vector<StringBenchmarkQuery> benchmark_queries = {
		{"alert", "alert"},
		{"search", "search"},
		{"listen", "listen"},
		{"rescue", "rescue"},
	};

	std::size_t metric_correct = 0;
	std::size_t vector_mismatches = 0;
	double metric_margin_sum = 0.0;
	std::string first_metric_winner;
	std::string first_vector_winner;
	double first_metric_distance = 0.0;
	double first_vector_distance = 0.0;

	for (std::size_t index = 0; index < benchmark_queries.size(); ++index) {
		const auto &query = benchmark_queries[index];
		const auto metric_neighbors =
			mtrc::find_neighbors(space, query.text, mtrc::count{1}, mtrc::stats::search::cover_tree{});
		const auto vector_neighbors =
			mtrc::find_neighbors(vector_baseline, string_features(query.text), mtrc::count{1});

		assert(metric_neighbors.size() == 1);
		assert(vector_neighbors.size() == 1);

		const auto metric_family = families[metric_neighbors[0].id.index()];
		const auto vector_family = families[vector_neighbors[0].id.index()];
		assert(metric_family == query.expected_family);
		assert(vector_family != query.expected_family);

		const auto vector_winner_under_metric = space.metric()(query.text, records[vector_neighbors[0].id.index()]);
		const auto metric_margin =
			static_cast<double>(vector_winner_under_metric) - static_cast<double>(metric_neighbors[0].distance);
		assert(metric_margin > 0.0);

		++metric_correct;
		++vector_mismatches;
		metric_margin_sum += metric_margin;

		if (index == 0) {
			first_metric_winner = records[metric_neighbors[0].id.index()];
			first_vector_winner = records[vector_neighbors[0].id.index()];
			first_metric_distance = static_cast<double>(metric_neighbors[0].distance);
			first_vector_distance = vector_neighbors[0].distance;
		}
	}

	const auto average_metric_margin = metric_margin_sum / static_cast<double>(benchmark_queries.size());
	assert(close_to(average_metric_margin, 2.0));

	std::cout << "nearest string = " << records[neighbors[0].id.index()] << " at " << neighbors[0].distance << "\n";
	std::cout << "string groups = " << groups.cluster_count << "\n";
	std::cout << "string benchmark metric winner = " << first_metric_winner << " at " << first_metric_distance << "\n";
	std::cout << "string benchmark vector baseline winner = " << first_vector_winner << " at " << first_vector_distance
			  << "\n";
	std::cout << "string benchmark baseline mismatch = yes\n";
	std::cout << "string benchmark records = " << records.size() << "\n";
	std::cout << "string benchmark queries = " << benchmark_queries.size() << "\n";
	std::cout << "string benchmark metric correct = " << metric_correct << "/" << benchmark_queries.size() << "\n";
	std::cout << "string benchmark vector mismatches = " << vector_mismatches << "/" << benchmark_queries.size()
			  << "\n";
	std::cout << "string benchmark average metric margin = " << average_metric_margin << "\n";
	std::cout << "string benchmark dense evaluations = " << matrix.diagnostics().cached_distances << "\n";

	return 0;
}
