#include <cassert>
#include <limits>
#include <map>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "metric/space/index/sparsify.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

int main()
{
	const std::vector<int> records = {0, 1, 2, 4};

	using Graph = mtrc::space::index::GraphConstructionResult<int>;
	using Edge = typename Graph::edge_type;

	const auto manual_knn = mtrc::space::index::make_exact_knn_graph_result<int>(
		std::vector<Edge>{{0, 1, 1}, {1, 0, 1}}, records.size(), 1);
	assert(manual_knn.metadata.strategy == "exact_knn");
	assert(manual_knn.metadata.record_count == records.size());
	assert(manual_knn.metadata.edge_count == 2);
	assert(manual_knn.metadata.k.has_value());
	assert(manual_knn.metadata.k.value() == 1);
	assert(manual_knn.metadata.edge_payload == "metric_distance");
	assert(manual_knn.metadata.sparsification == "none");
	assert(manual_knn.metadata.symmetrization == "none");
	assert(manual_knn.metadata.normalization == "none");
	assert(manual_knn.metadata.tie_break == "distance_then_target_index");

	const auto manual_radius = mtrc::space::index::make_exact_radius_graph_result<int, double>(
		std::vector<Edge>{{0, 1, 1}}, records.size(), 1.5);
	assert(manual_radius.metadata.strategy == "exact_radius");
	assert(manual_radius.metadata.edge_count == 1);
	assert(manual_radius.metadata.radius.has_value());
	assert(manual_radius.metadata.radius.value() == 1.5);
	assert(manual_radius.metadata.tie_break == "source_then_target_index");

	const auto manual_undirected = mtrc::space::index::make_symmetrized_graph_result(
		manual_knn, std::vector<Edge>{{0, 1, 1}}, "union", "minimum_distance");
	assert(!manual_undirected.metadata.directed);
	assert(manual_undirected.metadata.edge_count == 1);
	assert(manual_undirected.metadata.symmetrization == "union");
	assert(manual_undirected.metadata.weighting == "minimum_distance");
	assert(manual_undirected.metadata.tie_break == "source_index_then_target_index");

	mtrc::space::index::checked_graph_symmetrization_policy("union");
	mtrc::space::index::checked_graph_symmetrization_policy("mutual");
	bool rejected_symmetrization_policy = false;
	try {
		mtrc::space::index::checked_graph_symmetrization_policy("intersection");
	} catch (const std::invalid_argument &) {
		rejected_symmetrization_policy = true;
	}
	assert(rejected_symmetrization_policy);

	mtrc::space::index::checked_graph_weighting_policy("minimum_distance");
	mtrc::space::index::checked_graph_weighting_policy("maximum_distance");
	bool rejected_checked_weighting_policy = false;
	try {
		mtrc::space::index::checked_graph_weighting_policy("median_distance");
	} catch (const std::invalid_argument &) {
		rejected_checked_weighting_policy = true;
	}
	assert(rejected_checked_weighting_policy);

	assert(mtrc::space::index::merge_graph_edge_weight(4, 2, "minimum_distance") == 2);
	assert(mtrc::space::index::merge_graph_edge_weight(4, 2, "maximum_distance") == 4);
	bool rejected_weighting_policy = false;
	try {
		(void)mtrc::space::index::merge_graph_edge_weight(4, 2, "median_distance");
	} catch (const std::invalid_argument &) {
		rejected_weighting_policy = true;
	}
	assert(rejected_weighting_policy);

	std::optional<int> accumulated_weight;
	mtrc::space::index::accumulate_graph_edge_weight(accumulated_weight, 4, "minimum_distance");
	assert(accumulated_weight.has_value());
	assert(accumulated_weight.value() == 4);
	mtrc::space::index::accumulate_graph_edge_weight(accumulated_weight, 2, "minimum_distance");
	assert(accumulated_weight.value() == 2);
	mtrc::space::index::accumulate_graph_edge_weight(accumulated_weight, 5, "maximum_distance");
	assert(accumulated_weight.value() == 5);

	std::optional<int> rejected_accumulated_weight;
	bool rejected_accumulated_weighting_policy = false;
	try {
		mtrc::space::index::accumulate_graph_edge_weight(rejected_accumulated_weight, 4, "median_distance");
	} catch (const std::invalid_argument &) {
		rejected_accumulated_weighting_policy = true;
	}
	assert(rejected_accumulated_weighting_policy);
	assert(!rejected_accumulated_weight.has_value());

	std::map<std::pair<std::size_t, std::size_t>, mtrc::space::index::GraphSymmetricEdgeAccumulator<int>>
		symmetric_accumulators;
	mtrc::space::index::accumulate_graph_symmetric_edge(symmetric_accumulators, 2, 1, 4, "minimum_distance");
	mtrc::space::index::accumulate_graph_symmetric_edge(symmetric_accumulators, 1, 2, 3, "minimum_distance");
	mtrc::space::index::accumulate_graph_symmetric_edge(symmetric_accumulators, 1, 2, 1, "minimum_distance");
	mtrc::space::index::accumulate_graph_symmetric_edge(symmetric_accumulators, 0, 0, 9, "minimum_distance");
	assert(symmetric_accumulators.size() == 1);
	const auto symmetric_key = std::pair<std::size_t, std::size_t>{1, 2};
	assert(symmetric_accumulators[symmetric_key].forward.has_value());
	assert(symmetric_accumulators[symmetric_key].forward.value() == 1);
	assert(symmetric_accumulators[symmetric_key].reverse.has_value());
	assert(symmetric_accumulators[symmetric_key].reverse.value() == 4);

	std::map<std::pair<std::size_t, std::size_t>, mtrc::space::index::GraphSymmetricEdgeAccumulator<int>>
		maximum_accumulators;
	mtrc::space::index::accumulate_graph_symmetric_edge(maximum_accumulators, 0, 2, 4, "maximum_distance");
	mtrc::space::index::accumulate_graph_symmetric_edge(maximum_accumulators, 0, 2, 6, "maximum_distance");
	mtrc::space::index::accumulate_graph_symmetric_edge(maximum_accumulators, 2, 0, 1, "maximum_distance");
	assert(maximum_accumulators.size() == 1);
	const auto maximum_key = std::pair<std::size_t, std::size_t>{0, 2};
	assert(maximum_accumulators[maximum_key].forward.value() == 6);
	assert(maximum_accumulators[maximum_key].reverse.value() == 1);

	const auto accumulated_symmetric_edges = mtrc::space::index::accumulate_graph_symmetric_edges(
		std::vector<Edge>{{2, 1, 4}, {1, 2, 3}, {1, 2, 1}, {0, 0, 9}}, "minimum_distance");
	assert(accumulated_symmetric_edges.size() == 1);
	assert(accumulated_symmetric_edges.at(symmetric_key).forward.value() == 1);
	assert(accumulated_symmetric_edges.at(symmetric_key).reverse.value() == 4);
	bool rejected_accumulate_edges_weighting = false;
	try {
		(void)mtrc::space::index::accumulate_graph_symmetric_edges(std::vector<Edge>{{0, 0, 9}},
																  "median_distance");
	} catch (const std::invalid_argument &) {
		rejected_accumulate_edges_weighting = true;
	}
	assert(rejected_accumulate_edges_weighting);

	const mtrc::space::index::GraphSymmetricEdgeAccumulator<int> forward_only_edge{
		std::optional<int>{3}, std::nullopt};
	const auto union_forward_edge =
		mtrc::space::index::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 1},
															forward_only_edge, "union", "minimum_distance");
	assert(union_forward_edge.has_value());
	assert((union_forward_edge.value() == Edge{0, 1, 3}));
	const auto mutual_forward_edge =
		mtrc::space::index::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 1},
															forward_only_edge, "mutual", "minimum_distance");
	assert(!mutual_forward_edge.has_value());
	bool rejected_materialize_policy = false;
	try {
		(void)mtrc::space::index::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 1},
																  forward_only_edge, "intersection",
																  "minimum_distance");
	} catch (const std::invalid_argument &) {
		rejected_materialize_policy = true;
	}
	assert(rejected_materialize_policy);
	bool rejected_materialize_weighting = false;
	try {
		(void)mtrc::space::index::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 1},
																  forward_only_edge, "union", "median_distance");
	} catch (const std::invalid_argument &) {
		rejected_materialize_weighting = true;
	}
	assert(rejected_materialize_weighting);

	const mtrc::space::index::GraphSymmetricEdgeAccumulator<int> bidirectional_edge{
		std::optional<int>{2}, std::optional<int>{5}};
	const auto minimum_bidirectional_edge =
		mtrc::space::index::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 2},
															bidirectional_edge, "mutual", "minimum_distance");
	assert(minimum_bidirectional_edge.has_value());
	assert((minimum_bidirectional_edge.value() == Edge{0, 2, 2}));
	const auto maximum_bidirectional_edge =
		mtrc::space::index::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 2},
															bidirectional_edge, "mutual", "maximum_distance");
	assert(maximum_bidirectional_edge.has_value());
	assert((maximum_bidirectional_edge.value() == Edge{0, 2, 5}));

	std::map<std::pair<std::size_t, std::size_t>, mtrc::space::index::GraphSymmetricEdgeAccumulator<int>>
		materialization_accumulators;
	materialization_accumulators[{0, 1}] = forward_only_edge;
	materialization_accumulators[{0, 2}] = bidirectional_edge;
	const auto union_materialized_edges = mtrc::space::index::materialize_graph_symmetric_edges(
		materialization_accumulators, "union", "minimum_distance");
	assert((union_materialized_edges == std::vector<Edge>{{0, 1, 3}, {0, 2, 2}}));
	const auto mutual_materialized_edges = mtrc::space::index::materialize_graph_symmetric_edges(
		materialization_accumulators, "mutual", "maximum_distance");
	assert((mutual_materialized_edges == std::vector<Edge>{{0, 2, 5}}));
	bool rejected_materialize_edges_policy = false;
	try {
		(void)mtrc::space::index::materialize_graph_symmetric_edges(materialization_accumulators, "intersection",
																   "minimum_distance");
	} catch (const std::invalid_argument &) {
		rejected_materialize_edges_policy = true;
	}
	assert(rejected_materialize_edges_policy);
	bool rejected_materialize_edges_weighting = false;
	try {
		(void)mtrc::space::index::materialize_graph_symmetric_edges(materialization_accumulators, "union",
																   "median_distance");
	} catch (const std::invalid_argument &) {
		rejected_materialize_edges_weighting = true;
	}
	assert(rejected_materialize_edges_weighting);

	const auto manual_pruned =
		mtrc::space::index::make_out_degree_pruned_graph_result(manual_knn, std::vector<Edge>{{0, 1, 1}}, 1);
	assert(manual_pruned.metadata.edge_count == 1);
	assert(manual_pruned.metadata.max_out_degree.has_value());
	assert(manual_pruned.metadata.max_out_degree.value() == 1);
	assert(manual_pruned.metadata.sparsification == "out_degree");
	assert(manual_pruned.metadata.tie_break == "source_index_then_distance_then_target_index");

	const auto selected_neighbor_candidates = mtrc::space::index::select_graph_neighbor_candidates_by_distance(
		std::vector<std::pair<int, std::size_t>>{{3, 3}, {1, 2}, {1, 1}}, 2);
	assert((selected_neighbor_candidates == std::vector<std::pair<int, std::size_t>>{{1, 1}, {1, 2}}));
	const auto all_neighbor_candidates = mtrc::space::index::select_graph_neighbor_candidates_by_distance(
		std::vector<std::pair<int, std::size_t>>{{2, 3}, {1, 1}}, 5);
	assert((all_neighbor_candidates == std::vector<std::pair<int, std::size_t>>{{1, 1}, {2, 3}}));
	assert(mtrc::space::index::select_graph_neighbor_candidates_by_distance(
			   std::vector<std::pair<int, std::size_t>>{{1, 1}}, 0)
			   .empty());

	const auto selected_out_edges = mtrc::space::index::select_graph_out_edges_by_distance(
		std::vector<Edge>{{0, 3, 3}, {0, 2, 1}, {0, 1, 1}}, 2);
	assert((selected_out_edges == std::vector<Edge>{{0, 1, 1}, {0, 2, 1}}));
	const auto all_sorted_out_edges = mtrc::space::index::select_graph_out_edges_by_distance(
		std::vector<Edge>{{0, 3, 3}, {0, 1, 1}}, 5);
	assert((all_sorted_out_edges == std::vector<Edge>{{0, 1, 1}, {0, 3, 3}}));
	assert(mtrc::space::index::select_graph_out_edges_by_distance(std::vector<Edge>{{0, 1, 1}}, 0).empty());
	const auto grouped_out_edges = mtrc::space::index::group_graph_edges_by_source(
		std::vector<Edge>{{1, 3, 2}, {0, 2, 1}, {1, 2, 1}});
	assert(grouped_out_edges.size() == 2);
	assert((grouped_out_edges.at(0) == std::vector<Edge>{{0, 2, 1}}));
	assert((grouped_out_edges.at(1) == std::vector<Edge>{{1, 3, 2}, {1, 2, 1}}));
	const auto selected_out_edges_by_source = mtrc::space::index::select_graph_out_edges_by_source(
		std::vector<Edge>{{1, 3, 2}, {0, 2, 1}, {1, 2, 1}, {0, 1, 4}}, 1);
	assert((selected_out_edges_by_source == std::vector<Edge>{{0, 2, 1}, {1, 2, 1}}));
	assert(mtrc::space::index::select_graph_out_edges_by_source(std::vector<Edge>{{0, 1, 1}}, 0).empty());

	const auto directed_paths = mtrc::space::index::graph_shortest_path_distances(manual_knn);
	assert(directed_paths[0][1] == 1.0);
	assert(directed_paths[1][0] == 1.0);
	assert(directed_paths[0][2] == std::numeric_limits<double>::infinity());
	const auto initialized_paths = mtrc::space::index::make_graph_shortest_path_matrix(3);
	assert(initialized_paths[0][0] == 0.0);
	assert(initialized_paths[0][1] == std::numeric_limits<double>::infinity());
	assert(mtrc::space::index::checked_graph_shortest_path_matrix_record_count(initialized_paths) == 3);
	bool rejected_shortest_path_matrix_shape = false;
	try {
		(void)mtrc::space::index::checked_graph_shortest_path_matrix_record_count(
			std::vector<std::vector<double>>{{0.0}, {1.0, 0.0}});
	} catch (const std::invalid_argument &) {
		rejected_shortest_path_matrix_shape = true;
	}
	assert(rejected_shortest_path_matrix_shape);
	auto directed_observed_paths = mtrc::space::index::make_graph_shortest_path_matrix(3);
	mtrc::space::index::observe_graph_shortest_path_edge(directed_observed_paths, Edge{0, 1, 5}, true);
	mtrc::space::index::observe_graph_shortest_path_edge(directed_observed_paths, Edge{0, 1, 3}, true);
	assert(directed_observed_paths[0][1] == 3.0);
	assert(directed_observed_paths[1][0] == std::numeric_limits<double>::infinity());
	auto undirected_observed_paths = mtrc::space::index::make_graph_shortest_path_matrix(3);
	mtrc::space::index::observe_graph_shortest_path_edges(undirected_observed_paths,
														 std::vector<Edge>{{0, 1, 2}, {1, 2, 4}}, false);
	assert(undirected_observed_paths[0][1] == 2.0);
	assert(undirected_observed_paths[1][0] == 2.0);
	mtrc::space::index::close_graph_shortest_paths(undirected_observed_paths);
	assert(undirected_observed_paths[0][2] == 6.0);
	assert(undirected_observed_paths[2][0] == 6.0);
	bool rejected_shortest_path_edge_indices = false;
	try {
		mtrc::space::index::observe_graph_shortest_path_edge(directed_observed_paths, Edge{0, 3, 1}, true);
	} catch (const std::invalid_argument &) {
		rejected_shortest_path_edge_indices = true;
	}
	assert(rejected_shortest_path_edge_indices);

	const auto directed_pairs = mtrc::space::index::graph_metric_pair_indices(3, true);
	assert((directed_pairs == std::vector<std::pair<std::size_t, std::size_t>>{
								  {0, 1},
								  {0, 2},
								  {1, 0},
								  {1, 2},
								  {2, 0},
								  {2, 1},
							  }));
	const auto undirected_pairs = mtrc::space::index::graph_metric_pair_indices(4, false);
	assert((undirected_pairs == std::vector<std::pair<std::size_t, std::size_t>>{
									{0, 1},
									{0, 2},
									{0, 3},
									{1, 2},
									{1, 3},
									{2, 3},
								}));
	assert(mtrc::space::index::graph_metric_pair_indices(1, true).empty());
	assert(mtrc::space::index::graph_metric_pair_indices(0, false).empty());

	const auto manual_degrees = mtrc::space::index::make_graph_degree_diagnostics(4, 3, false);
	assert(manual_degrees.record_count == 4);
	assert(manual_degrees.edge_count == 3);
	assert(!manual_degrees.directed);
	assert((manual_degrees.degrees == std::vector<std::size_t>{0, 0, 0, 0}));
	assert((manual_degrees.out_degrees == std::vector<std::size_t>{0, 0, 0, 0}));
	assert((manual_degrees.in_degrees == std::vector<std::size_t>{0, 0, 0, 0}));
	assert(manual_degrees.degree_policy == "undirected_endpoint");

	auto directed_degree_updates = mtrc::space::index::make_graph_degree_diagnostics(3, 2, true);
	mtrc::space::index::observe_graph_degree_edge(directed_degree_updates, 0, 2);
	mtrc::space::index::observe_graph_degree_edge(directed_degree_updates, 2, 0);
	assert((directed_degree_updates.out_degrees == std::vector<std::size_t>{1, 0, 1}));
	assert((directed_degree_updates.in_degrees == std::vector<std::size_t>{1, 0, 1}));
	assert((directed_degree_updates.degrees == std::vector<std::size_t>{2, 0, 2}));

	auto undirected_degree_updates = mtrc::space::index::make_graph_degree_diagnostics(3, 2, false);
	mtrc::space::index::observe_graph_degree_edge(undirected_degree_updates, 0, 2);
	mtrc::space::index::observe_graph_degree_edge(undirected_degree_updates, 1, 2);
	assert((undirected_degree_updates.out_degrees == std::vector<std::size_t>{0, 0, 0}));
	assert((undirected_degree_updates.in_degrees == std::vector<std::size_t>{0, 0, 0}));
	assert((undirected_degree_updates.degrees == std::vector<std::size_t>{1, 1, 2}));
	auto bulk_degree_updates = mtrc::space::index::make_graph_degree_diagnostics(3, 3, true);
	mtrc::space::index::observe_graph_degree_edges(bulk_degree_updates,
												  std::vector<Edge>{{0, 2, 5}, {2, 0, 7}, {1, 2, 3}});
	assert((bulk_degree_updates.out_degrees == std::vector<std::size_t>{1, 1, 1}));
	assert((bulk_degree_updates.in_degrees == std::vector<std::size_t>{1, 0, 2}));
	assert((bulk_degree_updates.degrees == std::vector<std::size_t>{2, 1, 3}));
	bool rejected_degree_edge_indices = false;
	try {
		mtrc::space::index::observe_graph_degree_edges(bulk_degree_updates, std::vector<Edge>{{0, 3, 1}});
	} catch (const std::invalid_argument &) {
		rejected_degree_edge_indices = true;
	}
	assert(rejected_degree_edge_indices);

	const auto manual_connectivity = mtrc::space::index::make_graph_connectivity_diagnostics(4, 3, true);
	assert(manual_connectivity.record_count == 4);
	assert(manual_connectivity.edge_count == 3);
	assert(manual_connectivity.directed);
	assert((manual_connectivity.component_labels == std::vector<std::size_t>{0, 0, 0, 0}));
	assert(manual_connectivity.connectivity_policy == "weak_undirected_reachability");

	mtrc::space::index::GraphComponentTracker component_tracker(4);
	component_tracker.observe_edge(0, 1);
	component_tracker.observe_edge(2, 3);
	const auto component_labeling = component_tracker.labeling();
	assert((component_labeling.component_labels == std::vector<std::size_t>{0, 0, 1, 1}));
	assert((component_labeling.component_sizes == std::vector<std::size_t>{2, 2}));
	assert(component_labeling.isolated_count == 0);

	mtrc::space::index::GraphComponentTracker isolated_component_tracker(3);
	isolated_component_tracker.observe_edge(0, 1);
	const auto isolated_component_labeling = isolated_component_tracker.labeling();
	assert((isolated_component_labeling.component_labels == std::vector<std::size_t>{0, 0, 1}));
	assert((isolated_component_labeling.component_sizes == std::vector<std::size_t>{2, 1}));
	assert(isolated_component_labeling.isolated_count == 1);
	mtrc::space::index::GraphComponentTracker bulk_component_tracker(4);
	mtrc::space::index::observe_graph_component_edges(bulk_component_tracker,
													 std::vector<Edge>{{0, 1, 1}, {2, 3, 1}, {1, 2, 1}}, 4);
	const auto bulk_component_labeling = bulk_component_tracker.labeling();
	assert((bulk_component_labeling.component_labels == std::vector<std::size_t>{0, 0, 0, 0}));
	assert((bulk_component_labeling.component_sizes == std::vector<std::size_t>{4}));
	assert(bulk_component_labeling.isolated_count == 0);
	mtrc::space::index::GraphComponentTracker rejected_component_tracker(2);
	bool rejected_component_edge_indices = false;
	try {
		mtrc::space::index::observe_graph_component_edges(rejected_component_tracker, std::vector<Edge>{{0, 2, 1}}, 2);
	} catch (const std::invalid_argument &) {
		rejected_component_edge_indices = true;
	}
	assert(rejected_component_edge_indices);

	const auto manual_stretch = mtrc::space::index::make_graph_stretch_diagnostics(4, 3, false);
	assert(manual_stretch.record_count == 4);
	assert(manual_stretch.edge_count == 3);
	assert(!manual_stretch.directed);
	assert(manual_stretch.stretch_policy == "undirected_shortest_path");

	const auto checked_indices = mtrc::space::index::checked_graph_edge_indices(Edge{0, 1, 1}, records.size());
	assert(checked_indices.first == 0);
	assert(checked_indices.second == 1);

	const auto degree_summary = mtrc::space::index::summarize_graph_degrees(std::vector<std::size_t>{0, 2, 4});
	assert(degree_summary.isolated_count == 1);
	assert(degree_summary.max_degree == 4);
	assert(degree_summary.average_degree == 2.0);

	const auto empty_degree_summary = mtrc::space::index::summarize_graph_degrees(std::vector<std::size_t>{});
	assert(empty_degree_summary.isolated_count == 0);
	assert(empty_degree_summary.max_degree == 0);
	assert(empty_degree_summary.average_degree == 0.0);
	auto applied_degree_summary = mtrc::space::index::make_graph_degree_diagnostics(3, 2, false);
	mtrc::space::index::apply_graph_degree_summary(applied_degree_summary, degree_summary);
	assert(applied_degree_summary.isolated_count == 1);
	assert(applied_degree_summary.max_degree == 4);
	assert(applied_degree_summary.average_degree == 2.0);

	const auto component_summary = mtrc::space::index::summarize_graph_components(std::vector<std::size_t>{3, 1});
	assert(component_summary.component_count == 2);
	assert(component_summary.largest_component_size == 3);
	assert(!component_summary.connected);

	const auto empty_component_summary = mtrc::space::index::summarize_graph_components(std::vector<std::size_t>{});
	assert(empty_component_summary.component_count == 0);
	assert(empty_component_summary.largest_component_size == 0);
	assert(empty_component_summary.connected);
	auto applied_connectivity_summary = mtrc::space::index::make_graph_connectivity_diagnostics(4, 3, false);
	const mtrc::space::index::GraphConnectivityLabeling applied_labeling{{0, 0, 1, 1}, {2, 2}, 0};
	mtrc::space::index::apply_graph_connectivity_summary(applied_connectivity_summary, applied_labeling,
														component_summary);
	assert((applied_connectivity_summary.component_labels == std::vector<std::size_t>{0, 0, 1, 1}));
	assert(applied_connectivity_summary.isolated_count == 0);
	assert(applied_connectivity_summary.component_count == 2);
	assert(applied_connectivity_summary.largest_component_size == 3);
	assert(!applied_connectivity_summary.connected);

	mtrc::space::index::GraphStretchAccumulator stretch_accumulator;
	stretch_accumulator.observe_zero_metric_pair();
	stretch_accumulator.observe_unreachable_pair();
	stretch_accumulator.observe_reachable_pair(1.5);
	stretch_accumulator.observe_reachable_pair(2.5);
	const auto stretch_summary = stretch_accumulator.summary();
	assert(stretch_summary.pair_count == 3);
	assert(stretch_summary.reachable_pair_count == 2);
	assert(stretch_summary.unreachable_pair_count == 1);
	assert(stretch_summary.zero_metric_pair_count == 1);
	assert(stretch_summary.max_stretch == 2.5);
	assert(stretch_summary.average_stretch == 2.0);
	auto applied_stretch_summary = mtrc::space::index::make_graph_stretch_diagnostics(4, 3, false);
	mtrc::space::index::apply_graph_stretch_summary(applied_stretch_summary, stretch_summary);
	assert(applied_stretch_summary.pair_count == 3);
	assert(applied_stretch_summary.reachable_pair_count == 2);
	assert(applied_stretch_summary.unreachable_pair_count == 1);
	assert(applied_stretch_summary.zero_metric_pair_count == 1);
	assert(applied_stretch_summary.max_stretch == 2.5);
	assert(applied_stretch_summary.average_stretch == 2.0);

	const mtrc::space::index::GraphStretchAccumulator empty_stretch_accumulator;
	const auto empty_stretch_summary = empty_stretch_accumulator.summary();
	assert(empty_stretch_summary.pair_count == 0);
	assert(empty_stretch_summary.reachable_pair_count == 0);
	assert(empty_stretch_summary.unreachable_pair_count == 0);
	assert(empty_stretch_summary.zero_metric_pair_count == 0);
	assert(empty_stretch_summary.max_stretch == 0.0);
	assert(empty_stretch_summary.average_stretch == 0.0);

	auto stretch_paths = mtrc::space::index::make_graph_shortest_path_matrix(3);
	mtrc::space::index::observe_graph_shortest_path_edge(stretch_paths, Edge{0, 1, 2}, true);
	mtrc::space::index::GraphStretchAccumulator pair_stretch_accumulator;
	mtrc::space::index::observe_graph_stretch_pair(pair_stretch_accumulator, stretch_paths, 0, 1,
												  [](std::size_t, std::size_t) { return 1.0; });
	mtrc::space::index::observe_graph_stretch_pair(pair_stretch_accumulator, stretch_paths, 0, 2,
												  [](std::size_t, std::size_t) { return 1.0; });
	mtrc::space::index::observe_graph_stretch_pair(pair_stretch_accumulator, stretch_paths, 1, 1,
												  [](std::size_t, std::size_t) { return 0.0; });
	const auto pair_stretch_summary = pair_stretch_accumulator.summary();
	assert(pair_stretch_summary.pair_count == 2);
	assert(pair_stretch_summary.reachable_pair_count == 1);
	assert(pair_stretch_summary.unreachable_pair_count == 1);
	assert(pair_stretch_summary.zero_metric_pair_count == 1);
	assert(pair_stretch_summary.max_stretch == 2.0);
	assert(pair_stretch_summary.average_stretch == 2.0);
	mtrc::space::index::observe_graph_shortest_path_edge(stretch_paths, Edge{1, 2, 6}, true);
	mtrc::space::index::GraphStretchAccumulator bulk_stretch_accumulator;
	mtrc::space::index::observe_graph_stretch_pairs(bulk_stretch_accumulator, stretch_paths,
												   std::vector<std::pair<std::size_t, std::size_t>>{{0, 1}, {1, 2}},
												   [](std::size_t source_index, std::size_t target_index) {
													   return source_index == 0 && target_index == 1 ? 1.0 : 3.0;
												   });
	const auto bulk_stretch_summary = bulk_stretch_accumulator.summary();
	assert(bulk_stretch_summary.pair_count == 2);
	assert(bulk_stretch_summary.reachable_pair_count == 2);
	assert(bulk_stretch_summary.unreachable_pair_count == 0);
	assert(bulk_stretch_summary.zero_metric_pair_count == 0);
	assert(bulk_stretch_summary.max_stretch == 2.0);
	assert(bulk_stretch_summary.average_stretch == 2.0);
	bool rejected_stretch_pair_index = false;
	try {
		mtrc::space::index::observe_graph_stretch_pair(bulk_stretch_accumulator, stretch_paths, 0, 3,
													  [](std::size_t, std::size_t) { return 1.0; });
	} catch (const std::invalid_argument &) {
		rejected_stretch_pair_index = true;
	}
	assert(rejected_stretch_pair_index);

	bool rejected_checked_indices = false;
	try {
		(void)mtrc::space::index::checked_graph_edge_indices(Edge{0, records.size(), 1}, records.size());
	} catch (const std::invalid_argument &) {
		rejected_checked_indices = true;
	}
	assert(rejected_checked_indices);

	const auto graph = mtrc::space::index::exact_knn_graph(records, AbsoluteDistance{}, 2);
	assert(graph.metadata.strategy == "exact_knn");
	assert(graph.metadata.record_count == records.size());
	assert(graph.metadata.edge_count == 8);
	assert(graph.metadata.directed);

	const auto radius_graph = mtrc::space::index::exact_radius_graph(records, AbsoluteDistance{}, 1);
	assert(radius_graph.metadata.strategy == "exact_radius");
	assert(radius_graph.metadata.record_count == records.size());
	assert(radius_graph.metadata.edge_count == 4);
	assert(radius_graph.metadata.directed);
	assert((radius_graph.edges == std::vector<std::tuple<std::size_t, std::size_t, int>>{
									  {0, 1, 1},
									  {1, 0, 1},
									  {1, 2, 1},
									  {2, 1, 1},
								  }));

	const auto pruned = mtrc::space::index::prune_graph_out_degree(graph, 1);
	assert(pruned.metadata.sparsification == "out_degree");
	assert(pruned.metadata.max_out_degree.has_value());
	assert(pruned.metadata.max_out_degree.value() == 1);
	assert(pruned.metadata.edge_count == records.size());
	assert((pruned.edges == std::vector<std::tuple<std::size_t, std::size_t, int>>{
								{0, 1, 1},
								{1, 0, 1},
								{2, 1, 1},
								{3, 2, 2},
							}));

	const auto undirected = mtrc::space::index::symmetrize_graph(pruned, "union", "minimum_distance");
	assert(!undirected.metadata.directed);
	assert(undirected.metadata.symmetrization == "union");
	assert(undirected.metadata.edge_count == 3);
	const auto undirected_paths = mtrc::space::index::graph_shortest_path_distances(undirected);
	assert(undirected_paths[0][3] == 4.0);
	assert(undirected_paths[3][0] == 4.0);

	const auto degrees = mtrc::space::index::graph_degree_diagnostics(undirected);
	assert(degrees.degree_policy == "undirected_endpoint");
	assert((degrees.degrees == std::vector<std::size_t>{1, 2, 2, 1}));

	const auto connectivity = mtrc::space::index::graph_connectivity_diagnostics(undirected);
	assert(connectivity.connected);
	assert(connectivity.component_count == 1);

	const auto stretch = mtrc::space::index::graph_stretch_diagnostics(records, AbsoluteDistance{}, undirected);
	assert(stretch.record_count == records.size());
	assert(stretch.edge_count == undirected.edges.size());
	assert(stretch.reachable_pair_count == 6);
	assert(stretch.unreachable_pair_count == 0);

	return 0;
}
