#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>

namespace {

using Point = std::vector<double>;

auto record_kind_name(mtrc::record_kind kind) -> std::string
{
	switch (kind) {
	case mtrc::record_kind::custom:
		return "custom";
	case mtrc::record_kind::vector:
		return "vector";
	case mtrc::record_kind::aligned_vector:
		return "aligned_vector";
	case mtrc::record_kind::string:
		return "string";
	case mtrc::record_kind::sequence:
		return "sequence";
	case mtrc::record_kind::structured:
		return "structured";
	}
	return "custom";
}

auto close(double lhs, double rhs) -> bool { return std::fabs(lhs - rhs) < 1e-9; }

} // namespace

int main()
{
	static_assert(mtrc::metric_traits<mtrc::Euclidean<double>>::law == mtrc::metric_law::metric,
				  "Euclidean distance should satisfy the metric law");
	static_assert(mtrc::metric_traits<mtrc::Euclidean<double>>::records == mtrc::record_kind::aligned_vector,
				  "Euclidean vector records should be inspectable as aligned vectors");

	const std::vector<std::string> names = {"alpha-0", "alpha-1", "alpha-2", "beta-0", "beta-1",
											"beta-2",  "gamma-0", "gamma-1", "outlier"};
	const std::vector<Point> records = {
		{0.0, 0.0}, {0.3, 0.2}, {0.2, -0.1}, {5.0, 5.0}, {5.2, 4.9}, {4.8, 5.1}, {-4.0, 4.0}, {-4.2, 4.1}, {10.0, -5.0},
	};

	auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});
	assert(space.size() == records.size());

	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);
	assert(close(matrix.distance(space.id(0), space.id(1)), std::sqrt(0.13)));

	mtrc::space::storage::CoverTreeIndex<decltype(space)> tree(space);
	const Point query = {0.1, 0.1};
	const auto tree_neighbors = tree.knn(query, 3);
	assert(tree_neighbors.size() == 3);
	assert(tree_neighbors[0].id == space.id(0));

	const auto implicit_neighbors = mtrc::find_neighbors(space, query, mtrc::count{3});
	assert(implicit_neighbors.representation == "metric_space");
	assert(implicit_neighbors[0].id == tree_neighbors[0].id);

	const auto exact_diagnostics =
		mtrc::space::storage::diagnostics_for_space(space, mtrc::space::storage::exact(), {}, "neighbors");
	assert(exact_diagnostics.policy_name == "exact_lazy_serial");
	assert(exact_diagnostics.representation == "metric_space");
	assert(exact_diagnostics.supported);

	const auto indexed_neighbors =
		mtrc::find_neighbors(space, query, mtrc::count{3}, mtrc::stats::search::cover_tree{});
	assert(indexed_neighbors.representation == "cover_tree_index");
	assert(indexed_neighbors[0].id == tree_neighbors[0].id);
	const auto indexed_diagnostics = mtrc::space::storage::diagnostics_for_space(
		space, mtrc::space::storage::exact(), indexed_neighbors.representation, "neighbors");
	assert(indexed_diagnostics.representation == "cover_tree_index");

	mtrc::space::storage::KnnGraphIndex<decltype(space)> graph(space, 2);
	const auto graph_stats = graph.stats_against(matrix, 4);
	assert(graph_stats.nodes == records.size());
	assert(graph_stats.recall_validated);
	assert(close(graph_stats.sampled_recall, 1.0));

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto materialized_neighbors =
		mtrc::find_neighbors(space, space.id(0), mtrc::count{3}, materialized_policy);
	assert(materialized_neighbors.representation == "distance_table");

	const auto groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(3), materialized_policy);
	assert(groups.algorithm == "kmedoids");
	assert(groups.cluster_count == 3);
	assert(groups.record_count == records.size());
	assert(groups.representation == "distance_table");

	const auto outliers = mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(0.7, 2), materialized_policy);
	assert(outliers.strategy == "dbscan_noise");
	assert(outliers.noise_count == 1);
	assert(outliers.outliers.front().id == space.id(8));
	assert(outliers.representation == "distance_table");

	const auto mapped = mtrc::map(
		space, [](const Point &record) -> Point { return {record[0] / 10.0, record[1] / 10.0}; },
		mtrc::Euclidean<double>{});
	assert(mapped.mapping == "deterministic_transform");
	assert(mapped.strategy == "deterministic_transform");
	assert(mapped.space.size() == space.size());
	assert(mapped.space.record(mapped.space.id(0)).size() == 2);

	std::cout << "record_kind = " << record_kind_name(mtrc::metric_traits<mtrc::Euclidean<double>>::records)
			  << "\n";
	std::cout << "metric_law = " << mtrc::metric_law_name(mtrc::metric_traits<mtrc::Euclidean<double>>::law)
			  << "\n";
	std::cout << "vector dimension metadata = " << records.front().size() << "\n";
	std::cout << "available representations = metric_space,distance_table,cover_tree_index,knn_graph_index\n";
	std::cout << "nearest vector = " << names[implicit_neighbors[0].id.index()] << " at "
			  << implicit_neighbors[0].distance << "\n";
	std::cout << "indexed nearest = " << names[indexed_neighbors[0].id.index()] << " via "
			  << indexed_neighbors.representation << "\n";
	std::cout << "runtime policy = " << exact_diagnostics.policy_name << " via " << exact_diagnostics.representation
			  << "\n";
	std::cout << "distance table reuse = " << groups.representation << "," << outliers.representation << "\n";
	std::cout << "vector groups = " << groups.cluster_count << "\n";
	std::cout << "vector outlier = " << names[outliers.outliers.front().id.index()] << " score "
			  << outliers.outliers.front().score << "\n";
	std::cout << "mapped vector dimensions = " << mapped.space.record(mapped.space.id(0)).size() << "\n";

	return 0;
}
