#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <metric/distance.hpp>
#include <metric/engine.hpp>

namespace {

using Point = std::vector<double>;

auto metric_law_name(metric::metric_law law) -> std::string
{
    switch (law) {
    case metric::metric_law::distance:
        return "distance";
    case metric::metric_law::metric:
        return "metric";
    case metric::metric_law::pseudo_metric:
        return "pseudo_metric";
    case metric::metric_law::unknown:
        return "unknown";
    }
    return "unknown";
}

auto record_kind_name(metric::record_kind kind) -> std::string
{
    switch (kind) {
    case metric::record_kind::custom:
        return "custom";
    case metric::record_kind::vector:
        return "vector";
    case metric::record_kind::aligned_vector:
        return "aligned_vector";
    case metric::record_kind::string:
        return "string";
    case metric::record_kind::sequence:
        return "sequence";
    case metric::record_kind::structured:
        return "structured";
    }
    return "custom";
}

auto close(double lhs, double rhs) -> bool
{
    return std::fabs(lhs - rhs) < 1e-9;
}

} // namespace

int main()
{
    static_assert(metric::metric_traits<metric::Euclidean<double>>::law == metric::metric_law::metric,
                  "Euclidean distance should satisfy the metric law");
    static_assert(metric::metric_traits<metric::Euclidean<double>>::records == metric::record_kind::aligned_vector,
                  "Euclidean vector records should be inspectable as aligned vectors");

    const std::vector<std::string> names = {
        "alpha-0", "alpha-1", "alpha-2", "beta-0", "beta-1", "beta-2", "gamma-0", "gamma-1", "outlier"};
    const std::vector<Point> records = {
        {0.0, 0.0},  {0.3, 0.2},  {0.2, -0.1}, {5.0, 5.0},  {5.2, 4.9},
        {4.8, 5.1},  {-4.0, 4.0}, {-4.2, 4.1}, {10.0, -5.0},
    };

    auto space = metric::make_space(records, metric::Euclidean<double>{});
    assert(space.size() == records.size());

    metric::representations::MatrixCache<decltype(space)> matrix(space);
    assert(close(matrix.distance(space.id(0), space.id(1)), std::sqrt(0.13)));

    metric::representations::CoverTreeIndex<decltype(space)> tree(space);
    const Point query = {0.1, 0.1};
    const auto tree_neighbors = tree.knn(query, 3);
    assert(tree_neighbors.size() == 3);
    assert(tree_neighbors[0].id == space.id(0));

    const auto implicit_neighbors = metric::find_neighbors(space, query, metric::count{3});
    assert(implicit_neighbors.representation == "metric_space");
    assert(implicit_neighbors[0].id == tree_neighbors[0].id);

    const auto exact_diagnostics = metric::runtime::diagnostics_for_space(space, metric::runtime::exact(), {}, "neighbors");
    assert(exact_diagnostics.policy_name == "exact_lazy_serial");
    assert(exact_diagnostics.representation == "metric_space");
    assert(exact_diagnostics.supported);

    const auto indexed_neighbors =
        metric::find_neighbors(space, query, metric::count{3}, metric::strategies::cover_tree{});
    assert(indexed_neighbors.representation == "cover_tree_index");
    assert(indexed_neighbors[0].id == tree_neighbors[0].id);
    const auto indexed_diagnostics =
        metric::runtime::diagnostics_for_space(space, metric::runtime::exact(), indexed_neighbors.representation,
                                               "neighbors");
    assert(indexed_diagnostics.representation == "cover_tree_index");

    metric::representations::KnnGraphIndex<decltype(space)> graph(space, 2);
    const auto graph_stats = graph.stats_against(matrix, 4);
    assert(graph_stats.nodes == records.size());
    assert(graph_stats.recall_validated);
    assert(close(graph_stats.sampled_recall, 1.0));

    const auto materialized_policy = metric::runtime::materialized(metric::runtime::exact());
    const auto materialized_neighbors = metric::find_neighbors(space, space.id(0), metric::count{3}, materialized_policy);
    assert(materialized_neighbors.representation == "matrix_cache");

    const auto groups = metric::find_groups(space, metric::strategies::k_medoids(3), materialized_policy);
    assert(groups.algorithm == "kmedoids");
    assert(groups.cluster_count == 3);
    assert(groups.record_count == records.size());
    assert(groups.representation == "matrix_cache");

    const auto outliers = metric::find_outliers(space, metric::strategies::dbscan(0.7, 2), materialized_policy);
    assert(outliers.strategy == "dbscan_noise");
    assert(outliers.noise_count == 1);
    assert(outliers.outliers.front().id == space.id(8));
    assert(outliers.representation == "matrix_cache");

    const auto mapped = metric::map(
        space,
        [](const Point &record) -> Point {
            return {record[0] / 10.0, record[1] / 10.0};
        },
        metric::Euclidean<double>{});
    assert(mapped.mapping == "deterministic_transform");
    assert(mapped.strategy == "deterministic_transform");
    assert(mapped.space.size() == space.size());
    assert(mapped.space.record(mapped.space.id(0)).size() == 2);

    std::cout << "record_kind = " << record_kind_name(metric::metric_traits<metric::Euclidean<double>>::records) << "\n";
    std::cout << "metric_law = " << metric_law_name(metric::metric_traits<metric::Euclidean<double>>::law) << "\n";
    std::cout << "vector dimension metadata = " << records.front().size() << "\n";
    std::cout << "available representations = metric_space,matrix_cache,cover_tree_index,knn_graph_index\n";
    std::cout << "nearest vector = " << names[implicit_neighbors[0].id.index()] << " at "
              << implicit_neighbors[0].distance << "\n";
    std::cout << "indexed nearest = " << names[indexed_neighbors[0].id.index()] << " via "
              << indexed_neighbors.representation << "\n";
    std::cout << "runtime policy = " << exact_diagnostics.policy_name << " via "
              << exact_diagnostics.representation << "\n";
    std::cout << "matrix cache reuse = " << groups.representation << "," << outliers.representation << "\n";
    std::cout << "vector groups = " << groups.cluster_count << "\n";
    std::cout << "vector outlier = " << names[outliers.outliers.front().id.index()] << " score "
              << outliers.outliers.front().score << "\n";
    std::cout << "mapped vector dimensions = " << mapped.space.record(mapped.space.id(0)).size() << "\n";

    return 0;
}
