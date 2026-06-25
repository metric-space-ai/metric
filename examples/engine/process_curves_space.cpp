#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <metric/engine.hpp>

namespace {

struct AlignedCurveDistance {
	double gap_cost{2.0};

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

struct ProcessCurveQuery {
	std::string name;
	std::string expected_family;
	std::vector<double> curve;
};

} // namespace

int main()
{
	const std::vector<std::string> names = {"normal_reference", "flat_hold", "late_ramp", "spike", "early_ramp"};
	const std::vector<std::vector<double>> records = {
		{0, 0, 1, 1, 1, 2, 3, 3}, {0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 1, 1, 1, 2, 3, 3},
		{0, 0, 1, 7, 1, 2, 3, 3}, {0, 1, 1, 1, 2, 3, 3, 3},
	};
	const std::vector<double> query = {0, 0, 0, 0, 1, 1, 1, 2, 3, 3};

	auto space = mtrc::make_space(records, AlignedCurveDistance{});
	auto vector_baseline = mtrc::make_space(records, PointwisePaddedEuclideanDistance{});

	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);
	const auto matrix_diagnostics = matrix.diagnostics();
	assert(matrix_diagnostics.cached_distances == records.size() * records.size());

	const auto neighbors = mtrc::find_neighbors(space, query, 2);
	assert(neighbors.representation == "metric_space");
	assert(neighbors.size() == 2);
	assert(neighbors[0].id == space.id(2));
	assert(neighbors[0].distance == 2.0);
	assert(neighbors[1].id == space.id(0));

	const auto baseline_neighbors = mtrc::find_neighbors(vector_baseline, query, 2);
	assert(baseline_neighbors.size() == 2);
	assert(baseline_neighbors[0].id == vector_baseline.id(1));
	assert(baseline_neighbors[0].id != neighbors[0].id);

	const auto groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2));
	assert(groups.cluster_count == 2);
	assert(groups.record_count == records.size());
	assert(groups.algorithm == "kmedoids");

	const auto outliers = mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(3.0, 2));
	assert(outliers.strategy == "dbscan_density_outlier");
	assert(outliers.unassigned_count == 2);

	const auto representatives = mtrc::find_representatives(space, 2, mtrc::space::select::farthest_first{});
	assert(representatives.size() == 2);
	assert(representatives.strategy == "farthest_first");

	mtrc::space::storage::GraphTopology<decltype(space)> graph(space);
	for (std::size_t source = 0; source < space.size(); ++source) {
		for (std::size_t target = 0; target < space.size(); ++target) {
			if (source == target) {
				continue;
			}
			const auto distance = space.distance(space.id(source), space.id(target));
			if (distance <= 3.0) {
				graph.add_edge(space.id(source), space.id(target), distance);
			}
		}
	}
	assert(graph.record_count() == space.size());
	assert(graph.edge_count() > 0);

	const auto metric_margin = space.metric()(query, records[1]) - neighbors[0].distance;
	assert(metric_margin == 3.0);

	std::cout << "process workflow metric winner = " << names[neighbors[0].id.index()] << " at " << neighbors[0].distance
			  << "\n";
	std::cout << "process workflow vector comparison winner = " << names[baseline_neighbors[0].id.index()] << " at "
			  << baseline_neighbors[0].distance << "\n";
	std::cout << "process workflow baseline mismatch = yes\n";
	std::cout << "process workflow metric margin = " << metric_margin << "\n";
	std::cout << "process workflow groups = " << groups.cluster_count << "\n";
	std::cout << "process workflow outliers = " << outliers.unassigned_count << "\n";
	std::cout << "process workflow dense evaluations = " << matrix_diagnostics.cached_distances << "\n";
	std::cout << "process curve graph edges = " << graph.edge_count() << "\n";

	const std::vector<std::string> benchmark_families = {
		"normal_reference", "flat_hold", "late_ramp", "spike", "early_ramp",
		"normal_reference", "flat_hold", "late_ramp", "spike", "early_ramp"};
	const std::vector<std::vector<double>> benchmark_records = {
		{0, 0, 1, 1, 1, 2, 3, 3},		{0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 1, 1, 1, 2, 3, 3},
		{0, 0, 1, 7, 1, 2, 3, 3},		{0, 1, 1, 1, 2, 3, 3, 3},		{0, 0, 1, 1, 2, 2, 3, 3},
		{0, 0, 0, 0, 0, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 1, 1, 2, 3, 3},	{0, 0, 0, 7, 1, 2, 3, 3},
		{0, 1, 1, 2, 2, 3, 3},
	};
	const std::vector<ProcessCurveQuery> benchmark_queries = {
		{"delayed_ramp_A", "late_ramp", {0, 0, 0, 0, 1, 1, 1, 2, 3, 3}},
		{"delayed_ramp_B", "late_ramp", {0, 0, 0, 0, 0, 1, 1, 2, 3, 3}},
		{"delayed_ramp_C", "late_ramp", {0, 0, 0, 0, 1, 1, 2, 3}},
		{"delayed_ramp_D", "late_ramp", {0, 0, 0, 1, 1, 1, 1, 2, 3, 3}},
	};

	auto benchmark_space = mtrc::make_space(benchmark_records, AlignedCurveDistance{});
	auto benchmark_vector_baseline = mtrc::make_space(benchmark_records, PointwisePaddedEuclideanDistance{});
	mtrc::space::storage::DistanceTable<decltype(benchmark_space)> benchmark_matrix(benchmark_space);
	const auto benchmark_matrix_diagnostics = benchmark_matrix.diagnostics();
	assert(benchmark_matrix_diagnostics.cached_distances == benchmark_records.size() * benchmark_records.size());

	std::size_t benchmark_metric_correct = 0;
	std::size_t benchmark_baseline_mismatches = 0;
	double benchmark_metric_margin_sum = 0.0;
	for (const auto &benchmark_query : benchmark_queries) {
		const auto metric_neighbors = mtrc::find_neighbors(benchmark_space, benchmark_query.curve, 1);
		const auto baseline_neighbors = mtrc::find_neighbors(benchmark_vector_baseline, benchmark_query.curve, 1);
		assert(metric_neighbors.size() == 1);
		assert(baseline_neighbors.size() == 1);

		const auto metric_family = benchmark_families[metric_neighbors[0].id.index()];
		const auto baseline_family = benchmark_families[baseline_neighbors[0].id.index()];
		if (metric_family == benchmark_query.expected_family) {
			++benchmark_metric_correct;
		}
		if (baseline_family != benchmark_query.expected_family) {
			++benchmark_baseline_mismatches;
		}
		const auto baseline_distance_under_metric =
			benchmark_space.metric()(benchmark_query.curve, benchmark_records[baseline_neighbors[0].id.index()]);
		const auto metric_margin = baseline_distance_under_metric - metric_neighbors[0].distance;
		assert(metric_margin > 0.0);
		benchmark_metric_margin_sum += metric_margin;
	}
	assert(benchmark_metric_correct == benchmark_queries.size());
	assert(benchmark_baseline_mismatches == benchmark_queries.size());
	assert(benchmark_metric_margin_sum == 12.0);

	mtrc::space::storage::GraphTopology<decltype(benchmark_space)> benchmark_graph(benchmark_space);
	for (std::size_t source = 0; source < benchmark_space.size(); ++source) {
		for (std::size_t target = 0; target < benchmark_space.size(); ++target) {
			if (source == target) {
				continue;
			}
			const auto distance = benchmark_space.distance(benchmark_space.id(source), benchmark_space.id(target));
			if (distance <= 3.0) {
				benchmark_graph.add_edge(benchmark_space.id(source), benchmark_space.id(target), distance);
			}
		}
	}
	assert(benchmark_graph.edge_count() == 24);

	std::cout << "process benchmark records = " << benchmark_records.size() << "\n";
	std::cout << "process benchmark queries = " << benchmark_queries.size() << "\n";
	std::cout << "process benchmark metric correct = " << benchmark_metric_correct << "/" << benchmark_queries.size()
			  << "\n";
	std::cout << "process benchmark vector mismatches = " << benchmark_baseline_mismatches << "/"
			  << benchmark_queries.size() << "\n";
	std::cout << "process benchmark average metric margin = "
			  << benchmark_metric_margin_sum / static_cast<double>(benchmark_queries.size()) << "\n";
	std::cout << "process benchmark dense evaluations = " << benchmark_matrix_diagnostics.cached_distances << "\n";
	std::cout << "process benchmark graph edges = " << benchmark_graph.edge_count() << "\n";

	const std::vector<std::string> gallery_families = {
		"normal_reference", "flat_hold", "late_ramp",		 "spike", "early_ramp",
		"normal_reference", "flat_hold", "late_ramp",		 "spike", "early_ramp",
		"late_ramp",		"flat_hold", "normal_reference", "spike", "early_ramp"};
	const std::vector<std::vector<double>> gallery_records = {
		{0, 0, 1, 1, 1, 2, 3, 3},		{0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 1, 1, 1, 2, 3, 3},
		{0, 0, 1, 7, 1, 2, 3, 3},		{0, 1, 1, 1, 2, 3, 3, 3},		{0, 0, 1, 1, 2, 2, 3, 3},
		{0, 0, 0, 0, 0, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 1, 1, 2, 3, 3},	{0, 0, 0, 7, 1, 2, 3, 3},
		{0, 1, 1, 2, 2, 3, 3},			{0, 0, 0, 0, 0, 1, 2, 3, 3},	{0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
		{0, 0, 1, 1, 1, 1, 2, 3, 3},	{0, 0, 0, 1, 8, 1, 2, 3, 3},	{0, 1, 1, 1, 1, 2, 3, 3},
	};
	const std::vector<ProcessCurveQuery> gallery_queries = {
		{"gallery_delay_A", "late_ramp", {0, 0, 0, 0, 1, 1, 1, 2, 3, 3}},
		{"gallery_delay_B", "late_ramp", {0, 0, 0, 0, 0, 1, 1, 2, 3, 3}},
		{"gallery_delay_C", "late_ramp", {0, 0, 0, 0, 1, 1, 2, 3}},
		{"gallery_delay_D", "late_ramp", {0, 0, 0, 1, 1, 1, 1, 2, 3, 3}},
		{"gallery_delay_E", "late_ramp", {0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3}},
		{"gallery_delay_F", "late_ramp", {0, 0, 0, 0, 1, 1, 1, 1, 2, 3, 3}},
	};

	auto gallery_space = mtrc::make_space(gallery_records, AlignedCurveDistance{});
	auto gallery_vector_baseline = mtrc::make_space(gallery_records, PointwisePaddedEuclideanDistance{});
	mtrc::space::storage::DistanceTable<decltype(gallery_space)> gallery_matrix(gallery_space);
	const auto gallery_matrix_diagnostics = gallery_matrix.diagnostics();
	assert(gallery_matrix_diagnostics.cached_distances == gallery_records.size() * gallery_records.size());

	std::size_t gallery_metric_correct = 0;
	std::size_t gallery_baseline_mismatches = 0;
	double gallery_metric_margin_sum = 0.0;
	for (const auto &gallery_query : gallery_queries) {
		const auto metric_neighbors = mtrc::find_neighbors(gallery_space, gallery_query.curve, 1);
		const auto baseline_neighbors = mtrc::find_neighbors(gallery_vector_baseline, gallery_query.curve, 1);
		assert(metric_neighbors.size() == 1);
		assert(baseline_neighbors.size() == 1);

		const auto metric_family = gallery_families[metric_neighbors[0].id.index()];
		const auto baseline_family = gallery_families[baseline_neighbors[0].id.index()];
		if (metric_family == gallery_query.expected_family) {
			++gallery_metric_correct;
		}
		if (baseline_family != gallery_query.expected_family) {
			++gallery_baseline_mismatches;
		}
		const auto baseline_distance_under_metric =
			gallery_space.metric()(gallery_query.curve, gallery_records[baseline_neighbors[0].id.index()]);
		const auto metric_margin = baseline_distance_under_metric - metric_neighbors[0].distance;
		assert(metric_margin > 0.0);
		gallery_metric_margin_sum += metric_margin;
	}
	assert(gallery_metric_correct == gallery_queries.size());
	assert(gallery_baseline_mismatches == gallery_queries.size());

	mtrc::space::storage::GraphTopology<decltype(gallery_space)> gallery_graph(gallery_space);
	for (std::size_t source = 0; source < gallery_space.size(); ++source) {
		for (std::size_t target = 0; target < gallery_space.size(); ++target) {
			if (source == target) {
				continue;
			}
			const auto distance = gallery_space.distance(gallery_space.id(source), gallery_space.id(target));
			if (distance <= 3.0) {
				gallery_graph.add_edge(gallery_space.id(source), gallery_space.id(target), distance);
			}
		}
	}
	assert(gallery_graph.edge_count() > benchmark_graph.edge_count());

	std::cout << "process gallery records = " << gallery_records.size() << "\n";
	std::cout << "process gallery queries = " << gallery_queries.size() << "\n";
	std::cout << "process gallery metric correct = " << gallery_metric_correct << "/" << gallery_queries.size() << "\n";
	std::cout << "process gallery vector mismatches = " << gallery_baseline_mismatches << "/" << gallery_queries.size()
			  << "\n";
	std::cout << "process gallery average metric margin = "
			  << gallery_metric_margin_sum / static_cast<double>(gallery_queries.size()) << "\n";
	std::cout << "process gallery dense evaluations = " << gallery_matrix_diagnostics.cached_distances << "\n";
	std::cout << "process gallery graph edges = " << gallery_graph.edge_count() << "\n";

	return 0;
}
