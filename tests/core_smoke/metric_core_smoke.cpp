#include <cassert>
#include <cmath>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "metric/distance.hpp"
#include "metric/operators.hpp"
#include "metric/space.hpp"

struct AbsoluteDistance {
    auto operator()(int lhs, int rhs) const -> int
    {
        return lhs > rhs ? lhs - rhs : rhs - lhs;
    }
};

int main()
{
    std::vector<std::string> records = {"cat", "cot", "coat", "dog"};

    auto space = metric::Space::from_records(records, metric::Edit<std::string>{});

    assert(space.size() == records.size());
    assert(space[0] == "cat");
    assert(space(0, 0) == 0);
    assert(space(0, 1) == 1);
    assert(space.distance(0, 2) == 1);
    assert(space(0, 1) == space(1, 0));

    const auto distances = space.pairwise_distances();
    assert(distances.size() == records.size());
    assert(distances[0][1] == 1);

    const auto operator_distances = metric::operators::pairwise_distance_matrix(records, metric::Edit<std::string>{});
    assert(operator_distances.size() == records.size());
    assert(operator_distances[0][1] == 1);

    const auto nearest = space.neighbors(std::string("cut"), 2);
    assert(nearest.size() == 2);
    assert(nearest[0].second == 1);
    assert(space.nearest(std::string("cut")).second == 1);
    assert(space.within_radius(std::string("cut"), 1).size() == 2);

    const auto operator_nearest =
        metric::operators::nearest_neighbors(records, metric::Edit<std::string>{}, std::string("cut"), 2);
    assert(operator_nearest.size() == 2);
    assert(operator_nearest[0].second == 1);

    const auto operator_range =
        metric::operators::range_neighbors(records, metric::Edit<std::string>{}, std::string("cut"), 1);
    assert(operator_range.size() == 2);

    const auto representative_ids = metric::operators::representative_indices(records, metric::Edit<std::string>{}, 3);
    assert((representative_ids == std::vector<std::size_t>{0, 3, 1}));

    const auto representative_records = metric::operators::representatives(records, metric::Edit<std::string>{}, 3);
    assert((representative_records == std::vector<std::string>{"cat", "dog", "cot"}));

    assert(metric::operators::medoid_index(records, metric::Edit<std::string>{}) == 1);
    assert(metric::operators::medoid(records, metric::Edit<std::string>{}) == "cot");

    const auto separated_ids =
        metric::operators::separated_representative_indices(records, metric::Edit<std::string>{}, 2);
    assert((separated_ids == std::vector<std::size_t>{0, 3}));

    const auto separated_records =
        metric::operators::separated_representatives(records, metric::Edit<std::string>{}, 2);
    assert((separated_records == std::vector<std::string>{"cat", "dog"}));

    const auto coverage_ids = metric::operators::coverage_representative_indices(records, metric::Edit<std::string>{}, 1);
    assert((coverage_ids == std::vector<std::size_t>{0, 3}));

    const auto coverage_records = metric::operators::coverage_representatives(records, metric::Edit<std::string>{}, 1);
    assert((coverage_records == std::vector<std::string>{"cat", "dog"}));

    bool rejected_edit_negative_radius = false;
    try {
        metric::operators::coverage_representative_indices(records, metric::Edit<std::string>{}, -1);
    } catch (const std::invalid_argument &) {
        rejected_edit_negative_radius = true;
    }
    assert(rejected_edit_negative_radius);

    bool rejected_empty_records = false;
    try {
        metric::operators::representative_indices(std::vector<std::string>{}, metric::Edit<std::string>{}, 1);
    } catch (const std::invalid_argument &) {
        rejected_empty_records = true;
    }
    assert(rejected_empty_records);

    bool rejected_empty_medoid = false;
    try {
        metric::operators::medoid_index(std::vector<std::string>{}, metric::Edit<std::string>{});
    } catch (const std::invalid_argument &) {
        rejected_empty_medoid = true;
    }
    assert(rejected_empty_medoid);

    bool rejected_large_k = false;
    try {
        metric::operators::representative_indices(records, metric::Edit<std::string>{}, records.size() + 1);
    } catch (const std::invalid_argument &) {
        rejected_large_k = true;
    }
    assert(rejected_large_k);

    bool rejected_seed = false;
    try {
        metric::operators::representative_indices(records, metric::Edit<std::string>{}, 1, records.size());
    } catch (const std::out_of_range &) {
        rejected_seed = true;
    }
    assert(rejected_seed);

    const std::vector<int> line = {0, 1, 2, 3, 4};
    assert((metric::operators::coverage_representative_indices(line, AbsoluteDistance{}, 2) ==
            std::vector<std::size_t>{0, 3}));
    assert((metric::operators::coverage_representatives(line, AbsoluteDistance{}, 2) == std::vector<int>{0, 3}));
    assert(metric::operators::medoid_index(line, AbsoluteDistance{}) == 2);
    assert(metric::operators::medoid(line, AbsoluteDistance{}) == 2);
    assert((metric::operators::separated_representative_indices(line, AbsoluteDistance{}, 2) ==
            std::vector<std::size_t>{0, 2, 4}));
    assert((metric::operators::separated_representatives(line, AbsoluteDistance{}, 2) == std::vector<int>{0, 2, 4}));

    const auto exact_knn_edges = metric::operators::exact_knn_graph_edges(line, AbsoluteDistance{}, 2);
    const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_knn_edges = {
        {0, 1, 1},
        {0, 2, 2},
        {1, 0, 1},
        {1, 2, 1},
        {2, 1, 1},
        {2, 3, 1},
        {3, 2, 1},
        {3, 4, 1},
        {4, 3, 1},
        {4, 2, 2},
    };
    assert(exact_knn_edges == expected_knn_edges);

    const auto knn_graph = metric::operators::exact_knn_graph(line, AbsoluteDistance{}, 2);
    assert(knn_graph.edges == expected_knn_edges);
    assert(knn_graph.metadata.strategy == "exact_knn");
    assert(knn_graph.metadata.record_count == line.size());
    assert(knn_graph.metadata.edge_count == expected_knn_edges.size());
    assert(knn_graph.metadata.directed);
    assert(!knn_graph.metadata.self_loops);
    assert(knn_graph.metadata.exact);
    assert(knn_graph.metadata.k.has_value());
    assert(knn_graph.metadata.k.value() == 2);
    assert(!knn_graph.metadata.radius.has_value());
    assert(knn_graph.metadata.edge_payload == "metric_distance");
    assert(knn_graph.metadata.weighting == "none");
    assert(!knn_graph.metadata.max_out_degree.has_value());
    assert(knn_graph.metadata.sparsification == "none");
    assert(knn_graph.metadata.symmetrization == "none");
    assert(knn_graph.metadata.normalization == "none");
    assert(knn_graph.metadata.tie_break == "distance_then_target_index");

    const auto pruned_knn_graph = metric::operators::prune_graph_out_degree(knn_graph, 1);
    const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_pruned_knn_edges = {
        {0, 1, 1},
        {1, 0, 1},
        {2, 1, 1},
        {3, 2, 1},
        {4, 3, 1},
    };
    assert(pruned_knn_graph.edges == expected_pruned_knn_edges);
    assert(pruned_knn_graph.metadata.directed);
    assert(pruned_knn_graph.metadata.edge_count == expected_pruned_knn_edges.size());
    assert(pruned_knn_graph.metadata.k.has_value());
    assert(pruned_knn_graph.metadata.k.value() == 2);
    assert(pruned_knn_graph.metadata.max_out_degree.has_value());
    assert(pruned_knn_graph.metadata.max_out_degree.value() == 1);
    assert(pruned_knn_graph.metadata.sparsification == "out_degree");
    assert(pruned_knn_graph.metadata.tie_break == "source_index_then_distance_then_target_index");

    const auto empty_pruned_graph = metric::operators::prune_graph_out_degree(knn_graph, 0);
    assert(empty_pruned_graph.edges.empty());
    assert(empty_pruned_graph.metadata.edge_count == 0);
    assert(empty_pruned_graph.metadata.max_out_degree.has_value());
    assert(empty_pruned_graph.metadata.max_out_degree.value() == 0);

    const auto one_neighbor_graph = metric::operators::exact_knn_graph(line, AbsoluteDistance{}, 1);
    const auto directed_degrees = metric::operators::graph_degree_diagnostics(one_neighbor_graph);
    assert(directed_degrees.record_count == line.size());
    assert(directed_degrees.edge_count == one_neighbor_graph.edges.size());
    assert(directed_degrees.directed);
    assert((directed_degrees.out_degrees == std::vector<std::size_t>{1, 1, 1, 1, 1}));
    assert((directed_degrees.in_degrees == std::vector<std::size_t>{1, 2, 1, 1, 0}));
    assert((directed_degrees.degrees == std::vector<std::size_t>{2, 3, 2, 2, 1}));
    assert(directed_degrees.isolated_count == 0);
    assert(directed_degrees.max_degree == 3);
    assert(std::abs(directed_degrees.average_degree - 2.0) < 1e-12);
    assert(directed_degrees.degree_policy == "directed_in_out");

    const auto directed_connectivity = metric::operators::graph_connectivity_diagnostics(one_neighbor_graph);
    assert(directed_connectivity.record_count == line.size());
    assert(directed_connectivity.edge_count == one_neighbor_graph.edges.size());
    assert(directed_connectivity.directed);
    assert((directed_connectivity.component_labels == std::vector<std::size_t>{0, 0, 0, 0, 0}));
    assert(directed_connectivity.component_count == 1);
    assert(directed_connectivity.isolated_count == 0);
    assert(directed_connectivity.largest_component_size == 5);
    assert(directed_connectivity.connected);
    assert(directed_connectivity.connectivity_policy == "weak_undirected_reachability");

    const auto union_graph = metric::operators::symmetrize_graph(one_neighbor_graph, "union", "minimum_distance");
    const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_union_edges = {
        {0, 1, 1},
        {1, 2, 1},
        {2, 3, 1},
        {3, 4, 1},
    };
    assert(union_graph.edges == expected_union_edges);
    assert(!union_graph.metadata.directed);
    assert(union_graph.metadata.edge_count == expected_union_edges.size());
    assert(union_graph.metadata.weighting == "minimum_distance");
    assert(union_graph.metadata.sparsification == "none");
    assert(union_graph.metadata.symmetrization == "union");
    assert(union_graph.metadata.tie_break == "source_index_then_target_index");

    const auto undirected_degrees = metric::operators::graph_degree_diagnostics(union_graph);
    assert(!undirected_degrees.directed);
    assert((undirected_degrees.degrees == std::vector<std::size_t>{1, 2, 2, 2, 1}));
    assert((undirected_degrees.out_degrees == std::vector<std::size_t>{0, 0, 0, 0, 0}));
    assert((undirected_degrees.in_degrees == std::vector<std::size_t>{0, 0, 0, 0, 0}));
    assert(undirected_degrees.edge_count == expected_union_edges.size());
    assert(undirected_degrees.isolated_count == 0);
    assert(undirected_degrees.max_degree == 2);
    assert(std::abs(undirected_degrees.average_degree - 1.6) < 1e-12);
    assert(undirected_degrees.degree_policy == "undirected_endpoint");

    const auto undirected_connectivity = metric::operators::graph_connectivity_diagnostics(union_graph);
    assert(!undirected_connectivity.directed);
    assert((undirected_connectivity.component_labels == std::vector<std::size_t>{0, 0, 0, 0, 0}));
    assert(undirected_connectivity.component_count == 1);
    assert(undirected_connectivity.isolated_count == 0);
    assert(undirected_connectivity.largest_component_size == 5);
    assert(undirected_connectivity.connected);
    assert(undirected_connectivity.connectivity_policy == "undirected_reachability");

    bool rejected_undirected_pruning = false;
    try {
        metric::operators::prune_graph_out_degree(union_graph, 1);
    } catch (const std::invalid_argument &) {
        rejected_undirected_pruning = true;
    }
    assert(rejected_undirected_pruning);

    const auto mutual_graph = metric::operators::symmetrize_graph(one_neighbor_graph, "mutual", "minimum_distance");
    const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_mutual_edges = {
        {0, 1, 1},
    };
    assert(mutual_graph.edges == expected_mutual_edges);
    assert(mutual_graph.metadata.symmetrization == "mutual");

    metric::operators::GraphConstructionResult<int> asymmetric_graph;
    asymmetric_graph.edges = {
        {0, 1, 5},
        {1, 0, 3},
        {1, 2, 4},
    };
    asymmetric_graph.metadata.strategy = "synthetic";
    asymmetric_graph.metadata.record_count = 3;
    asymmetric_graph.metadata.edge_count = asymmetric_graph.edges.size();
    asymmetric_graph.metadata.directed = true;
    asymmetric_graph.metadata.self_loops = false;
    asymmetric_graph.metadata.exact = true;
    asymmetric_graph.metadata.edge_payload = "metric_distance";
    asymmetric_graph.metadata.weighting = "none";
    asymmetric_graph.metadata.sparsification = "none";
    asymmetric_graph.metadata.symmetrization = "none";
    asymmetric_graph.metadata.normalization = "none";

    metric::operators::GraphConstructionResult<int> invalid_graph;
    invalid_graph.edges = {
        {0, 3, 1},
    };
    invalid_graph.metadata.record_count = 2;
    invalid_graph.metadata.edge_count = invalid_graph.edges.size();
    invalid_graph.metadata.directed = true;

    bool rejected_invalid_degree_graph = false;
    try {
        metric::operators::graph_degree_diagnostics(invalid_graph);
    } catch (const std::invalid_argument &) {
        rejected_invalid_degree_graph = true;
    }
    assert(rejected_invalid_degree_graph);

    bool rejected_invalid_connectivity_graph = false;
    try {
        metric::operators::graph_connectivity_diagnostics(invalid_graph);
    } catch (const std::invalid_argument &) {
        rejected_invalid_connectivity_graph = true;
    }
    assert(rejected_invalid_connectivity_graph);

    const auto minimum_weighted_graph =
        metric::operators::symmetrize_graph(asymmetric_graph, "union", "minimum_distance");
    const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_minimum_edges = {
        {0, 1, 3},
        {1, 2, 4},
    };
    assert(minimum_weighted_graph.edges == expected_minimum_edges);

    const auto maximum_weighted_graph =
        metric::operators::symmetrize_graph(asymmetric_graph, "union", "maximum_distance");
    const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_maximum_edges = {
        {0, 1, 5},
        {1, 2, 4},
    };
    assert(maximum_weighted_graph.edges == expected_maximum_edges);

    bool rejected_symmetrization_policy = false;
    try {
        metric::operators::symmetrize_graph(asymmetric_graph, "invalid", "minimum_distance");
    } catch (const std::invalid_argument &) {
        rejected_symmetrization_policy = true;
    }
    assert(rejected_symmetrization_policy);

    bool rejected_weighting_policy = false;
    try {
        metric::operators::symmetrize_graph(asymmetric_graph, "union", "average_distance");
    } catch (const std::invalid_argument &) {
        rejected_weighting_policy = true;
    }
    assert(rejected_weighting_policy);

    const auto exact_radius_edges = metric::operators::exact_radius_graph_edges(line, AbsoluteDistance{}, 1);
    const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_radius_edges = {
        {0, 1, 1},
        {1, 0, 1},
        {1, 2, 1},
        {2, 1, 1},
        {2, 3, 1},
        {3, 2, 1},
        {3, 4, 1},
        {4, 3, 1},
    };
    assert(exact_radius_edges == expected_radius_edges);

    const auto radius_graph = metric::operators::exact_radius_graph(line, AbsoluteDistance{}, 1);
    assert(radius_graph.edges == expected_radius_edges);
    assert(radius_graph.metadata.strategy == "exact_radius");
    assert(radius_graph.metadata.record_count == line.size());
    assert(radius_graph.metadata.edge_count == expected_radius_edges.size());
    assert(radius_graph.metadata.directed);
    assert(!radius_graph.metadata.self_loops);
    assert(radius_graph.metadata.exact);
    assert(!radius_graph.metadata.k.has_value());
    assert(radius_graph.metadata.radius.has_value());
    assert(radius_graph.metadata.radius.value() == 1);
    assert(radius_graph.metadata.edge_payload == "metric_distance");
    assert(radius_graph.metadata.weighting == "none");
    assert(!radius_graph.metadata.max_out_degree.has_value());
    assert(radius_graph.metadata.sparsification == "none");
    assert(radius_graph.metadata.symmetrization == "none");
    assert(radius_graph.metadata.normalization == "none");
    assert(radius_graph.metadata.tie_break == "source_then_target_index");

    const std::vector<int> separated_line = {0, 1, 10};
    const auto disconnected_graph = metric::operators::exact_radius_graph(separated_line, AbsoluteDistance{}, 1);
    const auto disconnected_connectivity = metric::operators::graph_connectivity_diagnostics(disconnected_graph);
    assert((disconnected_connectivity.component_labels == std::vector<std::size_t>{0, 0, 1}));
    assert(disconnected_connectivity.component_count == 2);
    assert(disconnected_connectivity.isolated_count == 1);
    assert(disconnected_connectivity.largest_component_size == 2);
    assert(!disconnected_connectivity.connected);

    bool rejected_large_graph_k = false;
    try {
        metric::operators::exact_knn_graph_edges(line, AbsoluteDistance{}, line.size());
    } catch (const std::invalid_argument &) {
        rejected_large_graph_k = true;
    }
    assert(rejected_large_graph_k);

    bool rejected_negative_radius = false;
    try {
        metric::operators::coverage_representative_indices(line, AbsoluteDistance{}, -1);
    } catch (const std::invalid_argument &) {
        rejected_negative_radius = true;
    }
    assert(rejected_negative_radius);

    bool rejected_negative_graph_radius = false;
    try {
        metric::operators::exact_radius_graph_edges(line, AbsoluteDistance{}, -1);
    } catch (const std::invalid_argument &) {
        rejected_negative_graph_radius = true;
    }
    assert(rejected_negative_graph_radius);

    bool rejected_negative_minimum_distance = false;
    try {
        metric::operators::separated_representative_indices(line, AbsoluteDistance{}, -1);
    } catch (const std::invalid_argument &) {
        rejected_negative_minimum_distance = true;
    }
    assert(rejected_negative_minimum_distance);

    const double dimension = metric::operators::intrinsic_dimension(line, AbsoluteDistance{});
    assert(std::abs(dimension - (std::log(5.0 / 3.0) / std::log(2.0))) < 1e-12);

    return 0;
}
