#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "metric/core/neighbor.hpp"
#include "metric/metric/catalog.hpp"
#include "metric/space/index/operators.hpp"
#include "metric/space.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct CountingAbsoluteDistance {
	int *calls{};

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

namespace {

auto id(std::size_t index) -> mtrc::RecordId { return mtrc::RecordId::from_index(index); }

void nearest_neighbor_selection_keeps_sort_semantics()
{
	using Neighbor = mtrc::core::Neighbor<double>;

	const auto empty = mtrc::core::take_nearest_neighbors(
		std::vector<Neighbor>{{id(2), 2.0}, {id(1), 1.0}, {id(0), 0.5}}, 0);
	assert(empty.empty());

	const auto one_from_ties = mtrc::core::take_nearest_neighbors(
		std::vector<Neighbor>{{id(4), 1.0}, {id(1), 1.0}, {id(3), 2.0}}, 1);
	assert(one_from_ties.size() == 1);
	assert(one_from_ties[0].id == id(1) && one_from_ties[0].distance == 1.0);

	const auto nearest = mtrc::core::take_nearest_neighbors(
		std::vector<Neighbor>{{id(8), 4.0}, {id(5), 2.0}, {id(2), 2.0}, {id(1), 1.0}, {id(7), 3.0}}, 3);
	assert(nearest.size() == 3);
	assert(nearest[0].id == id(1) && nearest[0].distance == 1.0);
	assert(nearest[1].id == id(2) && nearest[1].distance == 2.0);
	assert(nearest[2].id == id(5) && nearest[2].distance == 2.0);

	const auto nan = std::numeric_limits<double>::quiet_NaN();
	const auto with_nan = mtrc::core::take_nearest_neighbors(
		std::vector<Neighbor>{{id(4), nan}, {id(1), 2.0}, {id(3), nan}, {id(2), 1.0}}, 3);
	assert(with_nan.size() == 3);
	assert(with_nan[0].id == id(2) && with_nan[0].distance == 1.0);
	assert(with_nan[1].id == id(1) && with_nan[1].distance == 2.0);
	assert(with_nan[2].id == id(3) && std::isnan(with_nan[2].distance));
}

} // namespace

int main()
{
	nearest_neighbor_selection_keeps_sort_semantics();

	std::vector<std::string> records = {"cat", "cot", "coat", "dog"};

	auto space = mtrc::Space::from_records(records, mtrc::Edit<std::string>{});

	assert(space.size() == records.size());
	assert(space[0] == "cat");
	assert(space(0, 0) == 0);
	assert(space(0, 1) == 1);
	assert(space.distance(0, 2) == 1);
	assert(space(0, 1) == space(1, 0));

	const auto distances = space.pairwise_distances();
	assert(distances.size() == records.size());
	assert(distances[0][1] == 1);

	const auto operator_distances = mtrc::space::index::pairwise_distance_matrix(records, mtrc::Edit<std::string>{});
	assert(operator_distances.size() == records.size());
	assert(operator_distances[0][1] == 1);

	int guarded_metric_calls = 0;
	std::vector<int> large_records(1001);
	bool rejected_large_pairwise_matrix = false;
	try {
		(void)mtrc::space::index::pairwise_distance_matrix(large_records,
														   CountingAbsoluteDistance{&guarded_metric_calls});
	} catch (const mtrc::RepresentationError &) {
		rejected_large_pairwise_matrix = true;
	}
	assert(rejected_large_pairwise_matrix);
	assert(guarded_metric_calls == 0);

	bool rejected_large_legacy_space_build = false;
	try {
		(void)mtrc::Space::from_records(large_records, CountingAbsoluteDistance{&guarded_metric_calls});
	} catch (const mtrc::RepresentationError &) {
		rejected_large_legacy_space_build = true;
	}
	assert(rejected_large_legacy_space_build);
	assert(guarded_metric_calls == 0);

	auto small_int_space = mtrc::Space::from_records(std::vector<int>{0, 1, 2},
													 CountingAbsoluteDistance{&guarded_metric_calls});
	const auto calls_after_construction = guarded_metric_calls;
	bool rejected_small_custom_budget = false;
	try {
		(void)small_int_space.pairwise_distances(mtrc::pairwise_matrix_options{8});
	} catch (const mtrc::RepresentationError &) {
		rejected_small_custom_budget = true;
	}
	assert(rejected_small_custom_budget);
	assert(guarded_metric_calls == calls_after_construction);
	const auto unbounded_matrix = small_int_space.pairwise_distances(mtrc::pairwise_matrix_options{0});
	assert(unbounded_matrix.size() == 3 && unbounded_matrix[0][2] == 2);

	const auto nearest = space.neighbors(std::string("cut"), 2);
	assert(nearest.size() == 2);
	assert(nearest[0].second == 1);
	assert(space.nearest(std::string("cut")).second == 1);
	assert(space.within_radius(std::string("cut"), 1).size() == 2);

	const auto operator_nearest =
		mtrc::space::index::nearest_neighbors(records, mtrc::Edit<std::string>{}, std::string("cut"), 2);
	assert(operator_nearest.size() == 2);
	assert(operator_nearest[0].second == 1);

	const auto operator_range =
		mtrc::space::index::range_neighbors(records, mtrc::Edit<std::string>{}, std::string("cut"), 1);
	assert(operator_range.size() == 2);

	int direct_search_calls = 0;
	const auto direct_nearest = mtrc::space::index::nearest_neighbors(
		large_records, CountingAbsoluteDistance{&direct_search_calls}, 5, 1);
	assert(direct_nearest.size() == 1);
	assert(direct_search_calls == static_cast<int>(large_records.size()));
	const auto direct_range = mtrc::space::index::range_neighbors(
		large_records, CountingAbsoluteDistance{&direct_search_calls}, 5, 5);
	assert(direct_range.size() == large_records.size());
	assert(direct_search_calls == static_cast<int>(large_records.size() * 2));

	const auto representative_ids = mtrc::stats::structural_analysis::representative_indices(records, mtrc::Edit<std::string>{}, 3);
	assert((representative_ids == std::vector<std::size_t>{0, 3, 1}));

	const auto representative_records = mtrc::stats::structural_analysis::representatives(records, mtrc::Edit<std::string>{}, 3);
	assert((representative_records == std::vector<std::string>{"cat", "dog", "cot"}));

	assert(mtrc::stats::structural_analysis::medoid_index(records, mtrc::Edit<std::string>{}) == 1);
	assert(mtrc::stats::structural_analysis::medoid(records, mtrc::Edit<std::string>{}) == "cot");

	const auto separated_ids =
		mtrc::stats::structural_analysis::separated_representative_indices(records, mtrc::Edit<std::string>{}, 2);
	assert((separated_ids == std::vector<std::size_t>{0, 3}));

	const auto separated_records =
		mtrc::stats::structural_analysis::separated_representatives(records, mtrc::Edit<std::string>{}, 2);
	assert((separated_records == std::vector<std::string>{"cat", "dog"}));

	const auto coverage_ids =
		mtrc::stats::structural_analysis::coverage_representative_indices(records, mtrc::Edit<std::string>{}, 1);
	assert((coverage_ids == std::vector<std::size_t>{0, 3}));

	const auto coverage_records = mtrc::stats::structural_analysis::coverage_representatives(records, mtrc::Edit<std::string>{}, 1);
	assert((coverage_records == std::vector<std::string>{"cat", "dog"}));

	bool rejected_edit_negative_radius = false;
	try {
		mtrc::stats::structural_analysis::coverage_representative_indices(records, mtrc::Edit<std::string>{}, -1);
	} catch (const std::invalid_argument &) {
		rejected_edit_negative_radius = true;
	}
	assert(rejected_edit_negative_radius);

	bool rejected_empty_records = false;
	try {
		mtrc::stats::structural_analysis::representative_indices(std::vector<std::string>{}, mtrc::Edit<std::string>{}, 1);
	} catch (const std::invalid_argument &) {
		rejected_empty_records = true;
	}
	assert(rejected_empty_records);

	bool rejected_empty_medoid = false;
	try {
		mtrc::stats::structural_analysis::medoid_index(std::vector<std::string>{}, mtrc::Edit<std::string>{});
	} catch (const std::invalid_argument &) {
		rejected_empty_medoid = true;
	}
	assert(rejected_empty_medoid);

	bool rejected_large_k = false;
	try {
		mtrc::stats::structural_analysis::representative_indices(records, mtrc::Edit<std::string>{}, records.size() + 1);
	} catch (const std::invalid_argument &) {
		rejected_large_k = true;
	}
	assert(rejected_large_k);

	int legacy_structural_calls = 0;
	bool rejected_large_representatives = false;
	try {
		(void)mtrc::stats::structural_analysis::representative_indices(
			large_records, CountingAbsoluteDistance{&legacy_structural_calls}, large_records.size());
	} catch (const mtrc::RepresentationError &) {
		rejected_large_representatives = true;
	}
	assert(rejected_large_representatives);
	assert(legacy_structural_calls == 0);

	bool rejected_large_medoid = false;
	try {
		(void)mtrc::stats::structural_analysis::medoid_index(
			large_records, CountingAbsoluteDistance{&legacy_structural_calls});
	} catch (const mtrc::RepresentationError &) {
		rejected_large_medoid = true;
	}
	assert(rejected_large_medoid);
	assert(legacy_structural_calls == 0);

	bool rejected_large_separated = false;
	try {
		(void)mtrc::stats::structural_analysis::separated_representative_indices(
			large_records, CountingAbsoluteDistance{&legacy_structural_calls}, 1);
	} catch (const mtrc::RepresentationError &) {
		rejected_large_separated = true;
	}
	assert(rejected_large_separated);
	assert(legacy_structural_calls == 0);

	bool rejected_large_coverage = false;
	try {
		(void)mtrc::stats::structural_analysis::coverage_representative_indices(
			large_records, CountingAbsoluteDistance{&legacy_structural_calls}, 1);
	} catch (const mtrc::RepresentationError &) {
		rejected_large_coverage = true;
	}
	assert(rejected_large_coverage);
	assert(legacy_structural_calls == 0);

	bool rejected_seed = false;
	try {
		mtrc::stats::structural_analysis::representative_indices(records, mtrc::Edit<std::string>{}, 1, records.size());
	} catch (const std::out_of_range &) {
		rejected_seed = true;
	}
	assert(rejected_seed);

	const std::vector<int> line = {0, 1, 2, 3, 4};
	assert((mtrc::stats::structural_analysis::coverage_representative_indices(line, AbsoluteDistance{}, 2) ==
			std::vector<std::size_t>{0, 3}));
	assert((mtrc::stats::structural_analysis::coverage_representatives(line, AbsoluteDistance{}, 2) == std::vector<int>{0, 3}));
	assert(mtrc::stats::structural_analysis::medoid_index(line, AbsoluteDistance{}) == 2);
	assert(mtrc::stats::structural_analysis::medoid(line, AbsoluteDistance{}) == 2);
	assert((mtrc::stats::structural_analysis::separated_representative_indices(line, AbsoluteDistance{}, 2) ==
			std::vector<std::size_t>{0, 2, 4}));
	assert((mtrc::stats::structural_analysis::separated_representatives(line, AbsoluteDistance{}, 2) == std::vector<int>{0, 2, 4}));

	const auto exact_knn_edges = mtrc::space::index::exact_knn_graph_edges(line, AbsoluteDistance{}, 2);
	const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_knn_edges = {
		{0, 1, 1}, {0, 2, 2}, {1, 0, 1}, {1, 2, 1}, {2, 1, 1}, {2, 3, 1}, {3, 2, 1}, {3, 4, 1}, {4, 3, 1}, {4, 2, 2},
	};
	assert(exact_knn_edges == expected_knn_edges);

	int graph_guard_metric_calls = 0;
	std::vector<int> large_graph_records(1001);
	bool rejected_large_knn_graph = false;
	try {
		(void)mtrc::space::index::exact_knn_graph_edges(large_graph_records,
														 CountingAbsoluteDistance{&graph_guard_metric_calls}, 1);
	} catch (const mtrc::RepresentationError &) {
		rejected_large_knn_graph = true;
	}
	assert(rejected_large_knn_graph);
	assert(graph_guard_metric_calls == 0);

	bool rejected_small_knn_graph_budget = false;
	try {
		(void)mtrc::space::index::exact_knn_graph_edges(
			line, CountingAbsoluteDistance{&graph_guard_metric_calls}, 1, mtrc::space::index::exact_graph_options{4});
	} catch (const mtrc::RepresentationError &) {
		rejected_small_knn_graph_budget = true;
	}
	assert(rejected_small_knn_graph_budget);
	assert(graph_guard_metric_calls == 0);

	const auto calls_before_unbounded_knn = graph_guard_metric_calls;
	const auto unbounded_counting_knn = mtrc::space::index::exact_knn_graph_edges(
		line, CountingAbsoluteDistance{&graph_guard_metric_calls}, 1, mtrc::space::index::exact_graph_options{0});
	assert(unbounded_counting_knn.size() == line.size());
	assert(graph_guard_metric_calls > calls_before_unbounded_knn);
	const auto calls_after_unbounded_knn = graph_guard_metric_calls;

	const auto knn_graph = mtrc::space::index::exact_knn_graph(line, AbsoluteDistance{}, 2);
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

	const auto pruned_knn_graph = mtrc::space::index::prune_graph_out_degree(knn_graph, 1);
	const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_pruned_knn_edges = {
		{0, 1, 1}, {1, 0, 1}, {2, 1, 1}, {3, 2, 1}, {4, 3, 1},
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

	const auto empty_pruned_graph = mtrc::space::index::prune_graph_out_degree(knn_graph, 0);
	assert(empty_pruned_graph.edges.empty());
	assert(empty_pruned_graph.metadata.edge_count == 0);
	assert(empty_pruned_graph.metadata.max_out_degree.has_value());
	assert(empty_pruned_graph.metadata.max_out_degree.value() == 0);

	const auto one_neighbor_graph = mtrc::space::index::exact_knn_graph(line, AbsoluteDistance{}, 1);
	const auto directed_degrees = mtrc::space::index::graph_degree_diagnostics(one_neighbor_graph);
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

	const auto directed_connectivity = mtrc::space::index::graph_connectivity_diagnostics(one_neighbor_graph);
	assert(directed_connectivity.record_count == line.size());
	assert(directed_connectivity.edge_count == one_neighbor_graph.edges.size());
	assert(directed_connectivity.directed);
	assert((directed_connectivity.component_labels == std::vector<std::size_t>{0, 0, 0, 0, 0}));
	assert(directed_connectivity.component_count == 1);
	assert(directed_connectivity.isolated_count == 0);
	assert(directed_connectivity.largest_component_size == 5);
	assert(directed_connectivity.connected);
	assert(directed_connectivity.connectivity_policy == "weak_undirected_reachability");

	const auto union_graph = mtrc::space::index::symmetrize_graph(one_neighbor_graph, "union", "minimum_distance");
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

	const auto undirected_degrees = mtrc::space::index::graph_degree_diagnostics(union_graph);
	assert(!undirected_degrees.directed);
	assert((undirected_degrees.degrees == std::vector<std::size_t>{1, 2, 2, 2, 1}));
	assert((undirected_degrees.out_degrees == std::vector<std::size_t>{0, 0, 0, 0, 0}));
	assert((undirected_degrees.in_degrees == std::vector<std::size_t>{0, 0, 0, 0, 0}));
	assert(undirected_degrees.edge_count == expected_union_edges.size());
	assert(undirected_degrees.isolated_count == 0);
	assert(undirected_degrees.max_degree == 2);
	assert(std::abs(undirected_degrees.average_degree - 1.6) < 1e-12);
	assert(undirected_degrees.degree_policy == "undirected_endpoint");

	const auto undirected_connectivity = mtrc::space::index::graph_connectivity_diagnostics(union_graph);
	assert(!undirected_connectivity.directed);
	assert((undirected_connectivity.component_labels == std::vector<std::size_t>{0, 0, 0, 0, 0}));
	assert(undirected_connectivity.component_count == 1);
	assert(undirected_connectivity.isolated_count == 0);
	assert(undirected_connectivity.largest_component_size == 5);
	assert(undirected_connectivity.connected);
	assert(undirected_connectivity.connectivity_policy == "undirected_reachability");

	const auto undirected_stretch = mtrc::space::index::graph_stretch_diagnostics(line, AbsoluteDistance{}, union_graph);
	assert(!undirected_stretch.directed);
	assert(undirected_stretch.record_count == line.size());
	assert(undirected_stretch.edge_count == union_graph.edges.size());
	assert(undirected_stretch.pair_count == 10);
	assert(undirected_stretch.reachable_pair_count == 10);
	assert(undirected_stretch.unreachable_pair_count == 0);
	assert(undirected_stretch.zero_metric_pair_count == 0);
	assert(std::abs(undirected_stretch.max_stretch - 1.0) < 1e-12);
	assert(std::abs(undirected_stretch.average_stretch - 1.0) < 1e-12);
	assert(undirected_stretch.stretch_policy == "undirected_shortest_path");

	bool rejected_stretch_metric_budget = false;
	try {
		(void)mtrc::space::index::graph_stretch_diagnostics(
			line, CountingAbsoluteDistance{&graph_guard_metric_calls}, union_graph,
			mtrc::space::index::graph_stretch_options{4, 0, 0});
	} catch (const mtrc::RepresentationError &) {
		rejected_stretch_metric_budget = true;
	}
	assert(rejected_stretch_metric_budget);
	assert(graph_guard_metric_calls == calls_after_unbounded_knn);

	bool rejected_stretch_matrix_budget = false;
	try {
		(void)mtrc::space::index::graph_stretch_diagnostics(
			line, CountingAbsoluteDistance{&graph_guard_metric_calls}, union_graph,
			mtrc::space::index::graph_stretch_options{0, 4, 0});
	} catch (const mtrc::RepresentationError &) {
		rejected_stretch_matrix_budget = true;
	}
	assert(rejected_stretch_matrix_budget);
	assert(graph_guard_metric_calls == calls_after_unbounded_knn);

	const auto calls_before_unbounded_stretch = graph_guard_metric_calls;
	const auto unbounded_counting_stretch = mtrc::space::index::graph_stretch_diagnostics(
		line, CountingAbsoluteDistance{&graph_guard_metric_calls}, union_graph,
		mtrc::space::index::graph_stretch_options{0, 0, 0});
	assert(unbounded_counting_stretch.pair_count == 10);
	assert(graph_guard_metric_calls > calls_before_unbounded_stretch);
	const auto calls_after_unbounded_stretch = graph_guard_metric_calls;

	bool rejected_large_graph_pairs = false;
	try {
		(void)mtrc::space::index::graph_metric_pair_indices(1001, true);
	} catch (const mtrc::RepresentationError &) {
		rejected_large_graph_pairs = true;
	}
	assert(rejected_large_graph_pairs);

	bool rejected_large_shortest_path_matrix = false;
	try {
		(void)mtrc::space::index::make_graph_shortest_path_matrix(1001);
	} catch (const mtrc::RepresentationError &) {
		rejected_large_shortest_path_matrix = true;
	}
	assert(rejected_large_shortest_path_matrix);

	bool rejected_undirected_pruning = false;
	try {
		mtrc::space::index::prune_graph_out_degree(union_graph, 1);
	} catch (const std::invalid_argument &) {
		rejected_undirected_pruning = true;
	}
	assert(rejected_undirected_pruning);

	const auto mutual_graph = mtrc::space::index::symmetrize_graph(one_neighbor_graph, "mutual", "minimum_distance");
	const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_mutual_edges = {
		{0, 1, 1},
	};
	assert(mutual_graph.edges == expected_mutual_edges);
	assert(mutual_graph.metadata.symmetrization == "mutual");

	mtrc::space::index::GraphConstructionResult<int> asymmetric_graph;
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

	mtrc::space::index::GraphConstructionResult<int> invalid_graph;
	invalid_graph.edges = {
		{0, 3, 1},
	};
	invalid_graph.metadata.record_count = 2;
	invalid_graph.metadata.edge_count = invalid_graph.edges.size();
	invalid_graph.metadata.directed = true;

	bool rejected_invalid_degree_graph = false;
	try {
		mtrc::space::index::graph_degree_diagnostics(invalid_graph);
	} catch (const std::invalid_argument &) {
		rejected_invalid_degree_graph = true;
	}
	assert(rejected_invalid_degree_graph);

	bool rejected_invalid_connectivity_graph = false;
	try {
		mtrc::space::index::graph_connectivity_diagnostics(invalid_graph);
	} catch (const std::invalid_argument &) {
		rejected_invalid_connectivity_graph = true;
	}
	assert(rejected_invalid_connectivity_graph);

	const std::vector<int> two_record_line = {0, 1};
	bool rejected_invalid_stretch_graph = false;
	try {
		mtrc::space::index::graph_stretch_diagnostics(two_record_line, AbsoluteDistance{}, invalid_graph);
	} catch (const std::invalid_argument &) {
		rejected_invalid_stretch_graph = true;
	}
	assert(rejected_invalid_stretch_graph);

	bool rejected_stretch_record_mismatch = false;
	try {
		mtrc::space::index::graph_stretch_diagnostics(line, AbsoluteDistance{}, invalid_graph);
	} catch (const std::invalid_argument &) {
		rejected_stretch_record_mismatch = true;
	}
	assert(rejected_stretch_record_mismatch);

	const auto minimum_weighted_graph =
		mtrc::space::index::symmetrize_graph(asymmetric_graph, "union", "minimum_distance");
	const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_minimum_edges = {
		{0, 1, 3},
		{1, 2, 4},
	};
	assert(minimum_weighted_graph.edges == expected_minimum_edges);

	const auto maximum_weighted_graph =
		mtrc::space::index::symmetrize_graph(asymmetric_graph, "union", "maximum_distance");
	const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_maximum_edges = {
		{0, 1, 5},
		{1, 2, 4},
	};
	assert(maximum_weighted_graph.edges == expected_maximum_edges);

	bool rejected_symmetrization_policy = false;
	try {
		mtrc::space::index::symmetrize_graph(asymmetric_graph, "invalid", "minimum_distance");
	} catch (const std::invalid_argument &) {
		rejected_symmetrization_policy = true;
	}
	assert(rejected_symmetrization_policy);

	bool rejected_weighting_policy = false;
	try {
		mtrc::space::index::symmetrize_graph(asymmetric_graph, "union", "average_distance");
	} catch (const std::invalid_argument &) {
		rejected_weighting_policy = true;
	}
	assert(rejected_weighting_policy);

	const auto exact_radius_edges = mtrc::space::index::exact_radius_graph_edges(line, AbsoluteDistance{}, 1);
	const std::vector<std::tuple<std::size_t, std::size_t, int>> expected_radius_edges = {
		{0, 1, 1}, {1, 0, 1}, {1, 2, 1}, {2, 1, 1}, {2, 3, 1}, {3, 2, 1}, {3, 4, 1}, {4, 3, 1},
	};
	assert(exact_radius_edges == expected_radius_edges);

	bool rejected_large_radius_graph = false;
	try {
		(void)mtrc::space::index::exact_radius_graph_edges(large_graph_records,
														   CountingAbsoluteDistance{&graph_guard_metric_calls}, 1);
	} catch (const mtrc::RepresentationError &) {
		rejected_large_radius_graph = true;
	}
	assert(rejected_large_radius_graph);
	assert(graph_guard_metric_calls == calls_after_unbounded_stretch);

	bool rejected_small_radius_graph_budget = false;
	try {
		(void)mtrc::space::index::exact_radius_graph_edges(
			line, CountingAbsoluteDistance{&graph_guard_metric_calls}, 1,
			mtrc::space::index::exact_graph_options{4});
	} catch (const mtrc::RepresentationError &) {
		rejected_small_radius_graph_budget = true;
	}
	assert(rejected_small_radius_graph_budget);
	assert(graph_guard_metric_calls == calls_after_unbounded_stretch);

	const auto calls_before_unbounded_radius = graph_guard_metric_calls;
	const auto unbounded_counting_radius = mtrc::space::index::exact_radius_graph_edges(
		line, CountingAbsoluteDistance{&graph_guard_metric_calls}, 1, mtrc::space::index::exact_graph_options{0});
	assert(unbounded_counting_radius == expected_radius_edges);
	assert(graph_guard_metric_calls > calls_before_unbounded_radius);
	const auto calls_after_unbounded_radius = graph_guard_metric_calls;

	const auto radius_graph = mtrc::space::index::exact_radius_graph(line, AbsoluteDistance{}, 1);
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

	const auto directed_stretch = mtrc::space::index::graph_stretch_diagnostics(line, AbsoluteDistance{}, radius_graph);
	assert(directed_stretch.directed);
	assert(directed_stretch.record_count == line.size());
	assert(directed_stretch.edge_count == radius_graph.edges.size());
	assert(directed_stretch.pair_count == 20);
	assert(directed_stretch.reachable_pair_count == 20);
	assert(directed_stretch.unreachable_pair_count == 0);
	assert(directed_stretch.zero_metric_pair_count == 0);
	assert(std::abs(directed_stretch.max_stretch - 1.0) < 1e-12);
	assert(std::abs(directed_stretch.average_stretch - 1.0) < 1e-12);
	assert(directed_stretch.stretch_policy == "directed_shortest_path");

	mtrc::space::index::GraphConstructionResult<int> large_stretch_graph;
	large_stretch_graph.metadata.record_count = large_graph_records.size();
	large_stretch_graph.metadata.edge_count = 0;
	large_stretch_graph.metadata.directed = true;
	bool rejected_large_stretch_graph = false;
	try {
		(void)mtrc::space::index::graph_stretch_diagnostics(
			large_graph_records, CountingAbsoluteDistance{&graph_guard_metric_calls}, large_stretch_graph);
	} catch (const mtrc::RepresentationError &) {
		rejected_large_stretch_graph = true;
	}
	assert(rejected_large_stretch_graph);
	assert(graph_guard_metric_calls == calls_after_unbounded_radius);

	const std::vector<int> separated_line = {0, 1, 10};
	const auto disconnected_graph = mtrc::space::index::exact_radius_graph(separated_line, AbsoluteDistance{}, 1);
	const auto disconnected_connectivity = mtrc::space::index::graph_connectivity_diagnostics(disconnected_graph);
	assert((disconnected_connectivity.component_labels == std::vector<std::size_t>{0, 0, 1}));
	assert(disconnected_connectivity.component_count == 2);
	assert(disconnected_connectivity.isolated_count == 1);
	assert(disconnected_connectivity.largest_component_size == 2);
	assert(!disconnected_connectivity.connected);

	const auto disconnected_stretch =
		mtrc::space::index::graph_stretch_diagnostics(separated_line, AbsoluteDistance{}, disconnected_graph);
	assert(disconnected_stretch.pair_count == 6);
	assert(disconnected_stretch.reachable_pair_count == 2);
	assert(disconnected_stretch.unreachable_pair_count == 4);
	assert(disconnected_stretch.zero_metric_pair_count == 0);
	assert(std::abs(disconnected_stretch.max_stretch - 1.0) < 1e-12);
	assert(std::abs(disconnected_stretch.average_stretch - 1.0) < 1e-12);

	bool rejected_large_graph_k = false;
	try {
		mtrc::space::index::exact_knn_graph_edges(line, AbsoluteDistance{}, line.size());
	} catch (const std::invalid_argument &) {
		rejected_large_graph_k = true;
	}
	assert(rejected_large_graph_k);

	bool rejected_negative_radius = false;
	try {
		mtrc::stats::structural_analysis::coverage_representative_indices(line, AbsoluteDistance{}, -1);
	} catch (const std::invalid_argument &) {
		rejected_negative_radius = true;
	}
	assert(rejected_negative_radius);

	bool rejected_negative_graph_radius = false;
	try {
		mtrc::space::index::exact_radius_graph_edges(line, AbsoluteDistance{}, -1);
	} catch (const std::invalid_argument &) {
		rejected_negative_graph_radius = true;
	}
	assert(rejected_negative_graph_radius);

	bool rejected_negative_minimum_distance = false;
	try {
		mtrc::stats::structural_analysis::separated_representative_indices(line, AbsoluteDistance{}, -1);
	} catch (const std::invalid_argument &) {
		rejected_negative_minimum_distance = true;
	}
	assert(rejected_negative_minimum_distance);

	const double dimension = mtrc::stats::properties::intrinsic_dimension(line, AbsoluteDistance{});
	assert(std::abs(dimension - (std::log(5.0 / 3.0) / std::log(2.0))) < 1e-12);

	return 0;
}
