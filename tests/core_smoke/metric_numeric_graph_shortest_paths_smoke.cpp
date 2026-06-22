#include <cassert>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include <metric/numeric/math/GraphPrimitives.h>

namespace {

template <typename Function> auto rejects_invalid_argument(Function function) -> bool
{
	try {
		function();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

struct NumericDegreeDiagnostics {
	std::size_t record_count{};
	bool directed{true};
	std::vector<std::size_t> degrees;
	std::vector<std::size_t> out_degrees;
	std::vector<std::size_t> in_degrees;
	std::size_t isolated_count{};
	std::size_t max_degree{};
	double average_degree{};
};

auto make_numeric_degree_diagnostics(std::size_t record_count, bool directed) -> NumericDegreeDiagnostics
{
	NumericDegreeDiagnostics diagnostics;
	diagnostics.record_count = record_count;
	diagnostics.directed = directed;
	diagnostics.degrees.assign(record_count, 0);
	diagnostics.out_degrees.assign(record_count, 0);
	diagnostics.in_degrees.assign(record_count, 0);
	return diagnostics;
}

struct NumericConnectivityDiagnostics {
	std::vector<std::size_t> component_labels;
	std::size_t component_count{};
	std::size_t isolated_count{};
	std::size_t largest_component_size{};
	bool connected{true};
};

struct NumericStretchDiagnostics {
	std::size_t pair_count{};
	std::size_t reachable_pair_count{};
	std::size_t unreachable_pair_count{};
	std::size_t zero_metric_pair_count{};
	double max_stretch{};
	double average_stretch{};
};

} // namespace

int main()
{
	auto paths = mtrc::numeric::make_graph_shortest_path_matrix(3);
	assert(paths.size() == 3);
	assert(paths[0][0] == 0.0);
	assert(paths[0][1] == std::numeric_limits<double>::infinity());
	assert(mtrc::numeric::checked_graph_shortest_path_matrix_record_count(paths) == 3);
	assert(rejects_invalid_argument([] {
		(void)mtrc::numeric::checked_graph_shortest_path_matrix_record_count(
			std::vector<std::vector<double>>{{0.0}, {1.0, 0.0}});
	}));

	mtrc::numeric::observe_graph_shortest_path_edge(paths, 0, 1, 5, true);
	mtrc::numeric::observe_graph_shortest_path_edge(paths, 0, 1, 3, true);
	assert(paths[0][1] == 3.0);
	assert(paths[1][0] == std::numeric_limits<double>::infinity());
	assert(rejects_invalid_argument([&paths] {
		mtrc::numeric::observe_graph_shortest_path_edge(paths, 0, 3, 1, true);
	}));

	auto undirected_paths = mtrc::numeric::make_graph_shortest_path_matrix(4);
	mtrc::numeric::observe_graph_shortest_path_edge(undirected_paths, 0, 1, 2.0, false);
	mtrc::numeric::observe_graph_shortest_path_edge(undirected_paths, 1, 2, 4.0, false);
	mtrc::numeric::observe_graph_shortest_path_edge(undirected_paths, 2, 3, 1.5, false);
	assert(undirected_paths[1][0] == 2.0);
	mtrc::numeric::close_graph_shortest_paths(undirected_paths);
	assert(undirected_paths[0][2] == 6.0);
	assert(undirected_paths[0][3] == 7.5);
	assert(undirected_paths[3][0] == 7.5);

	assert(rejects_invalid_argument([] {
		auto ragged_paths = std::vector<std::vector<double>>{{0.0, 1.0}, {1.0}};
		mtrc::numeric::close_graph_shortest_paths(ragged_paths);
	}));

	const auto checked_pair = mtrc::numeric::checked_graph_index_pair(1, 2, 3);
	assert(checked_pair.first == 1);
	assert(checked_pair.second == 2);
	assert(rejects_invalid_argument([] {
		(void)mtrc::numeric::checked_graph_index_pair(1, 3, 3);
	}));

	const auto directed_pairs = mtrc::numeric::graph_index_pairs(3, true);
	assert((directed_pairs == std::vector<std::pair<std::size_t, std::size_t>>{
								  {0, 1},
								  {0, 2},
								  {1, 0},
								  {1, 2},
								  {2, 0},
								  {2, 1},
							  }));
	const auto undirected_pairs = mtrc::numeric::graph_index_pairs(4, false);
	assert((undirected_pairs == std::vector<std::pair<std::size_t, std::size_t>>{
									{0, 1},
									{0, 2},
									{0, 3},
									{1, 2},
									{1, 3},
									{2, 3},
								}));
	assert(mtrc::numeric::graph_index_pairs(1, true).empty());
	assert(mtrc::numeric::graph_index_pairs(0, false).empty());

	const auto selected_distance_index_pairs = mtrc::numeric::select_graph_distance_index_pairs_by_distance(
		std::vector<std::pair<int, std::size_t>>{{3, 3}, {1, 2}, {1, 1}}, 2);
	assert((selected_distance_index_pairs == std::vector<std::pair<int, std::size_t>>{{1, 1}, {1, 2}}));
	const auto full_distance_index_pairs = mtrc::numeric::select_graph_distance_index_pairs_by_distance(
		std::vector<std::pair<int, std::size_t>>{{2, 3}, {1, 1}}, 5);
	assert((full_distance_index_pairs == std::vector<std::pair<int, std::size_t>>{{1, 1}, {2, 3}}));
	assert(mtrc::numeric::select_graph_distance_index_pairs_by_distance(
			   std::vector<std::pair<int, std::size_t>>{{1, 1}}, 0)
			   .empty());

	using Edge = std::tuple<std::size_t, std::size_t, int>;
	const auto checked_edge_indices = mtrc::numeric::checked_graph_edge_indices(Edge{0, 2, 1}, 3);
	assert(checked_edge_indices.first == 0);
	assert(checked_edge_indices.second == 2);
	assert(rejects_invalid_argument([] {
		(void)mtrc::numeric::checked_graph_edge_indices(Edge{0, 3, 1}, 3);
	}));

	auto tuple_edge_paths = mtrc::numeric::make_graph_shortest_path_matrix(3);
	mtrc::numeric::observe_graph_shortest_path_edge(tuple_edge_paths, Edge{0, 1, 5}, true);
	mtrc::numeric::observe_graph_shortest_path_edge(tuple_edge_paths, Edge{0, 1, 3}, true);
	assert(tuple_edge_paths[0][1] == 3.0);
	assert(tuple_edge_paths[1][0] == std::numeric_limits<double>::infinity());
	assert(rejects_invalid_argument([&tuple_edge_paths] {
		mtrc::numeric::observe_graph_shortest_path_edge(tuple_edge_paths, Edge{0, 3, 1}, true);
	}));

	auto bulk_edge_paths = mtrc::numeric::make_graph_shortest_path_matrix(3);
	mtrc::numeric::observe_graph_shortest_path_edges(bulk_edge_paths,
													   std::vector<Edge>{{0, 1, 2}, {1, 2, 4}}, false);
	assert(bulk_edge_paths[0][1] == 2.0);
	assert(bulk_edge_paths[1][0] == 2.0);
	assert(bulk_edge_paths[1][2] == 4.0);
	assert(bulk_edge_paths[2][1] == 4.0);
	assert(rejects_invalid_argument([&bulk_edge_paths] {
		mtrc::numeric::observe_graph_shortest_path_edges(bulk_edge_paths, std::vector<Edge>{{0, 3, 1}}, true);
	}));

	const auto directed_shortest_paths =
		mtrc::numeric::graph_shortest_path_distances(3, std::vector<Edge>{{0, 1, 2}, {1, 2, 4}}, true);
	assert(directed_shortest_paths[0][2] == 6.0);
	assert(directed_shortest_paths[2][0] == std::numeric_limits<double>::infinity());
	const auto undirected_shortest_paths =
		mtrc::numeric::graph_shortest_path_distances(3, std::vector<Edge>{{0, 1, 2}, {1, 2, 4}}, false);
	assert(undirected_shortest_paths[0][2] == 6.0);
	assert(undirected_shortest_paths[2][0] == 6.0);
	assert(rejects_invalid_argument([] {
		(void)mtrc::numeric::graph_shortest_path_distances(3, std::vector<Edge>{{0, 3, 1}}, true);
	}));

	const auto selected_edges = mtrc::numeric::select_graph_edges_by_distance_then_target(
		std::vector<Edge>{{0, 3, 3}, {0, 2, 1}, {0, 1, 1}}, 2);
	assert((selected_edges == std::vector<Edge>{{0, 1, 1}, {0, 2, 1}}));
	const auto full_sorted_edges =
		mtrc::numeric::select_graph_edges_by_distance_then_target(std::vector<Edge>{{0, 3, 3}, {0, 1, 1}}, 5);
	assert((full_sorted_edges == std::vector<Edge>{{0, 1, 1}, {0, 3, 3}}));
	assert(mtrc::numeric::select_graph_edges_by_distance_then_target(std::vector<Edge>{{0, 1, 1}}, 0).empty());

	const auto grouped_edges = mtrc::numeric::group_graph_edges_by_source(
		std::vector<Edge>{{1, 3, 2}, {0, 2, 1}, {1, 2, 1}});
	assert(grouped_edges.size() == 2);
	assert((grouped_edges.at(0) == std::vector<Edge>{{0, 2, 1}}));
	assert((grouped_edges.at(1) == std::vector<Edge>{{1, 3, 2}, {1, 2, 1}}));
	const auto selected_edges_by_source = mtrc::numeric::select_graph_edges_by_source_then_distance(
		std::vector<Edge>{{1, 3, 2}, {0, 2, 1}, {1, 2, 1}, {0, 1, 4}}, 1);
	assert((selected_edges_by_source == std::vector<Edge>{{0, 2, 1}, {1, 2, 1}}));
	assert(mtrc::numeric::select_graph_edges_by_source_then_distance(std::vector<Edge>{{0, 1, 1}}, 0).empty());

	auto directed_degree_updates = make_numeric_degree_diagnostics(3, true);
	mtrc::numeric::observe_graph_degree_edge(directed_degree_updates, 0, 2);
	mtrc::numeric::observe_graph_degree_edge(directed_degree_updates, 2, 0);
	assert((directed_degree_updates.out_degrees == std::vector<std::size_t>{1, 0, 1}));
	assert((directed_degree_updates.in_degrees == std::vector<std::size_t>{1, 0, 1}));
	assert((directed_degree_updates.degrees == std::vector<std::size_t>{2, 0, 2}));

	auto undirected_degree_updates = make_numeric_degree_diagnostics(3, false);
	mtrc::numeric::observe_graph_degree_edge(undirected_degree_updates, 0, 2);
	mtrc::numeric::observe_graph_degree_edge(undirected_degree_updates, 1, 2);
	assert((undirected_degree_updates.out_degrees == std::vector<std::size_t>{0, 0, 0}));
	assert((undirected_degree_updates.in_degrees == std::vector<std::size_t>{0, 0, 0}));
	assert((undirected_degree_updates.degrees == std::vector<std::size_t>{1, 1, 2}));

	auto bulk_degree_updates = make_numeric_degree_diagnostics(3, true);
	mtrc::numeric::observe_graph_degree_edges(bulk_degree_updates,
												std::vector<Edge>{{0, 2, 5}, {2, 0, 7}, {1, 2, 3}});
	assert((bulk_degree_updates.out_degrees == std::vector<std::size_t>{1, 1, 1}));
	assert((bulk_degree_updates.in_degrees == std::vector<std::size_t>{1, 0, 2}));
	assert((bulk_degree_updates.degrees == std::vector<std::size_t>{2, 1, 3}));
	assert(rejects_invalid_argument([&bulk_degree_updates] {
		mtrc::numeric::observe_graph_degree_edges(bulk_degree_updates, std::vector<Edge>{{0, 3, 1}});
	}));

	const auto degree_summary = mtrc::numeric::summarize_graph_degrees(std::vector<std::size_t>{0, 2, 4});
	assert(degree_summary.isolated_count == 1);
	assert(degree_summary.max_degree == 4);
	assert(degree_summary.average_degree == 2.0);
	const auto empty_degree_summary = mtrc::numeric::summarize_graph_degrees(std::vector<std::size_t>{});
	assert(empty_degree_summary.isolated_count == 0);
	assert(empty_degree_summary.max_degree == 0);
	assert(empty_degree_summary.average_degree == 0.0);
	auto applied_degree_summary = make_numeric_degree_diagnostics(3, false);
	mtrc::numeric::apply_graph_degree_summary(applied_degree_summary, degree_summary);
	assert(applied_degree_summary.isolated_count == 1);
	assert(applied_degree_summary.max_degree == 4);
	assert(applied_degree_summary.average_degree == 2.0);

	mtrc::numeric::GraphComponentTracker component_tracker(4);
	component_tracker.observe_edge(0, 1);
	component_tracker.observe_edge(2, 3);
	const auto component_labeling = component_tracker.labeling();
	assert((component_labeling.component_labels == std::vector<std::size_t>{0, 0, 1, 1}));
	assert((component_labeling.component_sizes == std::vector<std::size_t>{2, 2}));
	assert(component_labeling.isolated_count == 0);
	mtrc::numeric::GraphComponentTracker isolated_component_tracker(3);
	isolated_component_tracker.observe_edge(0, 1);
	const auto isolated_component_labeling = isolated_component_tracker.labeling();
	assert((isolated_component_labeling.component_labels == std::vector<std::size_t>{0, 0, 1}));
	assert((isolated_component_labeling.component_sizes == std::vector<std::size_t>{2, 1}));
	assert(isolated_component_labeling.isolated_count == 1);
	mtrc::numeric::GraphComponentTracker bulk_component_tracker(4);
	mtrc::numeric::observe_graph_component_edges(bulk_component_tracker,
												   std::vector<Edge>{{0, 1, 1}, {2, 3, 1}, {1, 2, 1}}, 4);
	const auto bulk_component_labeling = bulk_component_tracker.labeling();
	assert((bulk_component_labeling.component_labels == std::vector<std::size_t>{0, 0, 0, 0}));
	assert((bulk_component_labeling.component_sizes == std::vector<std::size_t>{4}));
	assert(bulk_component_labeling.isolated_count == 0);
	mtrc::numeric::GraphComponentTracker rejected_component_tracker(2);
	assert(rejects_invalid_argument([&rejected_component_tracker] {
		mtrc::numeric::observe_graph_component_edges(rejected_component_tracker, std::vector<Edge>{{0, 2, 1}}, 2);
	}));
	const auto component_summary = mtrc::numeric::summarize_graph_components(std::vector<std::size_t>{3, 1});
	assert(component_summary.component_count == 2);
	assert(component_summary.largest_component_size == 3);
	assert(!component_summary.connected);
	const auto empty_component_summary = mtrc::numeric::summarize_graph_components(std::vector<std::size_t>{});
	assert(empty_component_summary.component_count == 0);
	assert(empty_component_summary.largest_component_size == 0);
	assert(empty_component_summary.connected);
	NumericConnectivityDiagnostics applied_connectivity_summary;
	const mtrc::numeric::GraphConnectivityLabeling applied_labeling{{0, 0, 1, 1}, {2, 2}, 0};
	mtrc::numeric::apply_graph_connectivity_summary(applied_connectivity_summary, applied_labeling,
													  component_summary);
	assert((applied_connectivity_summary.component_labels == std::vector<std::size_t>{0, 0, 1, 1}));
	assert(applied_connectivity_summary.isolated_count == 0);
	assert(applied_connectivity_summary.component_count == 2);
	assert(applied_connectivity_summary.largest_component_size == 3);
	assert(!applied_connectivity_summary.connected);

	mtrc::numeric::GraphStretchAccumulator stretch_accumulator;
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
	const mtrc::numeric::GraphStretchAccumulator empty_stretch_accumulator;
	const auto empty_stretch_summary = empty_stretch_accumulator.summary();
	assert(empty_stretch_summary.pair_count == 0);
	assert(empty_stretch_summary.reachable_pair_count == 0);
	assert(empty_stretch_summary.unreachable_pair_count == 0);
	assert(empty_stretch_summary.zero_metric_pair_count == 0);
	assert(empty_stretch_summary.max_stretch == 0.0);
	assert(empty_stretch_summary.average_stretch == 0.0);
	auto stretch_paths = mtrc::numeric::make_graph_shortest_path_matrix(3);
	mtrc::numeric::observe_graph_shortest_path_edge(stretch_paths, 0, 1, 2, true);
	mtrc::numeric::GraphStretchAccumulator pair_stretch_accumulator;
	mtrc::numeric::observe_graph_stretch_pair(pair_stretch_accumulator, stretch_paths, 0, 1,
												[](std::size_t, std::size_t) { return 1.0; });
	mtrc::numeric::observe_graph_stretch_pair(pair_stretch_accumulator, stretch_paths, 0, 2,
												[](std::size_t, std::size_t) { return 1.0; });
	mtrc::numeric::observe_graph_stretch_pair(pair_stretch_accumulator, stretch_paths, 1, 1,
												[](std::size_t, std::size_t) { return 0.0; });
	const auto pair_stretch_summary = pair_stretch_accumulator.summary();
	assert(pair_stretch_summary.pair_count == 2);
	assert(pair_stretch_summary.reachable_pair_count == 1);
	assert(pair_stretch_summary.unreachable_pair_count == 1);
	assert(pair_stretch_summary.zero_metric_pair_count == 1);
	assert(pair_stretch_summary.max_stretch == 2.0);
	assert(pair_stretch_summary.average_stretch == 2.0);
	assert(rejects_invalid_argument([&stretch_paths] {
		mtrc::numeric::GraphStretchAccumulator rejected_stretch_accumulator;
		mtrc::numeric::observe_graph_stretch_pair(rejected_stretch_accumulator, stretch_paths, 0, 3,
													[](std::size_t, std::size_t) { return 1.0; });
	}));
	mtrc::numeric::observe_graph_shortest_path_edge(stretch_paths, 1, 2, 6, true);
	mtrc::numeric::GraphStretchAccumulator bulk_stretch_accumulator;
	mtrc::numeric::observe_graph_stretch_pairs(bulk_stretch_accumulator, stretch_paths,
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
	NumericStretchDiagnostics applied_stretch_summary;
	mtrc::numeric::apply_graph_stretch_summary(applied_stretch_summary, stretch_summary);
	assert(applied_stretch_summary.pair_count == 3);
	assert(applied_stretch_summary.reachable_pair_count == 2);
	assert(applied_stretch_summary.unreachable_pair_count == 1);
	assert(applied_stretch_summary.zero_metric_pair_count == 1);
	assert(applied_stretch_summary.max_stretch == 2.5);
	assert(applied_stretch_summary.average_stretch == 2.0);

	mtrc::numeric::checked_graph_symmetrization_policy("union");
	mtrc::numeric::checked_graph_symmetrization_policy("mutual");
	assert(rejects_invalid_argument([] { mtrc::numeric::checked_graph_symmetrization_policy("intersection"); }));
	mtrc::numeric::checked_graph_weighting_policy("minimum_distance");
	mtrc::numeric::checked_graph_weighting_policy("maximum_distance");
	assert(rejects_invalid_argument([] { mtrc::numeric::checked_graph_weighting_policy("median_distance"); }));
	assert(mtrc::numeric::merge_graph_edge_weight(4, 2, "minimum_distance") == 2);
	assert(mtrc::numeric::merge_graph_edge_weight(4, 2, "maximum_distance") == 4);
	assert(rejects_invalid_argument([] { (void)mtrc::numeric::merge_graph_edge_weight(4, 2, "median_distance"); }));

	std::optional<int> accumulated_weight;
	mtrc::numeric::accumulate_graph_edge_weight(accumulated_weight, 4, "minimum_distance");
	assert(accumulated_weight.has_value());
	assert(accumulated_weight.value() == 4);
	mtrc::numeric::accumulate_graph_edge_weight(accumulated_weight, 2, "minimum_distance");
	assert(accumulated_weight.value() == 2);
	mtrc::numeric::accumulate_graph_edge_weight(accumulated_weight, 5, "maximum_distance");
	assert(accumulated_weight.value() == 5);

	std::optional<int> rejected_accumulated_weight;
	assert(rejects_invalid_argument([&rejected_accumulated_weight] {
		mtrc::numeric::accumulate_graph_edge_weight(rejected_accumulated_weight, 4, "median_distance");
	}));
	assert(!rejected_accumulated_weight.has_value());

	std::map<std::pair<std::size_t, std::size_t>, mtrc::numeric::GraphSymmetricEdgeAccumulator<int>>
		symmetric_accumulators;
	mtrc::numeric::accumulate_graph_symmetric_edge(symmetric_accumulators, 2, 1, 4, "minimum_distance");
	mtrc::numeric::accumulate_graph_symmetric_edge(symmetric_accumulators, 1, 2, 3, "minimum_distance");
	mtrc::numeric::accumulate_graph_symmetric_edge(symmetric_accumulators, 1, 2, 1, "minimum_distance");
	mtrc::numeric::accumulate_graph_symmetric_edge(symmetric_accumulators, 0, 0, 9, "minimum_distance");
	assert(symmetric_accumulators.size() == 1);
	const auto symmetric_key = std::pair<std::size_t, std::size_t>{1, 2};
	assert(symmetric_accumulators[symmetric_key].forward.value() == 1);
	assert(symmetric_accumulators[symmetric_key].reverse.value() == 4);

	std::map<std::pair<std::size_t, std::size_t>, mtrc::numeric::GraphSymmetricEdgeAccumulator<int>>
		maximum_accumulators;
	mtrc::numeric::accumulate_graph_symmetric_edge(maximum_accumulators, 0, 2, 4, "maximum_distance");
	mtrc::numeric::accumulate_graph_symmetric_edge(maximum_accumulators, 0, 2, 6, "maximum_distance");
	mtrc::numeric::accumulate_graph_symmetric_edge(maximum_accumulators, 2, 0, 1, "maximum_distance");
	const auto maximum_key = std::pair<std::size_t, std::size_t>{0, 2};
	assert(maximum_accumulators[maximum_key].forward.value() == 6);
	assert(maximum_accumulators[maximum_key].reverse.value() == 1);

	const auto accumulated_symmetric_edges = mtrc::numeric::accumulate_graph_symmetric_edges(
		std::vector<Edge>{{2, 1, 4}, {1, 2, 3}, {1, 2, 1}, {0, 0, 9}}, "minimum_distance");
	assert(accumulated_symmetric_edges.size() == 1);
	assert(accumulated_symmetric_edges.at(symmetric_key).forward.value() == 1);
	assert(accumulated_symmetric_edges.at(symmetric_key).reverse.value() == 4);
	assert(rejects_invalid_argument([] {
		(void)mtrc::numeric::accumulate_graph_symmetric_edges(std::vector<Edge>{{0, 0, 9}}, "median_distance");
	}));

	const mtrc::numeric::GraphSymmetricEdgeAccumulator<int> forward_only_edge{std::optional<int>{3}, std::nullopt};
	const auto union_forward_edge =
		mtrc::numeric::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 1},
														  forward_only_edge, "union", "minimum_distance");
	assert(union_forward_edge.has_value());
	assert((union_forward_edge.value() == Edge{0, 1, 3}));
	const auto mutual_forward_edge =
		mtrc::numeric::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 1},
														  forward_only_edge, "mutual", "minimum_distance");
	assert(!mutual_forward_edge.has_value());
	assert(rejects_invalid_argument([&forward_only_edge] {
		(void)mtrc::numeric::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 1},
																forward_only_edge, "intersection", "minimum_distance");
	}));
	assert(rejects_invalid_argument([&forward_only_edge] {
		(void)mtrc::numeric::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 1},
																forward_only_edge, "union", "median_distance");
	}));

	const mtrc::numeric::GraphSymmetricEdgeAccumulator<int> bidirectional_edge{std::optional<int>{2},
																				std::optional<int>{5}};
	const auto minimum_bidirectional_edge =
		mtrc::numeric::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 2},
														  bidirectional_edge, "mutual", "minimum_distance");
	assert(minimum_bidirectional_edge.has_value());
	assert((minimum_bidirectional_edge.value() == Edge{0, 2, 2}));
	const auto maximum_bidirectional_edge =
		mtrc::numeric::materialize_graph_symmetric_edge(std::pair<std::size_t, std::size_t>{0, 2},
														  bidirectional_edge, "mutual", "maximum_distance");
	assert(maximum_bidirectional_edge.has_value());
	assert((maximum_bidirectional_edge.value() == Edge{0, 2, 5}));

	std::map<std::pair<std::size_t, std::size_t>, mtrc::numeric::GraphSymmetricEdgeAccumulator<int>>
		materialization_accumulators;
	materialization_accumulators[{0, 1}] = forward_only_edge;
	materialization_accumulators[{0, 2}] = bidirectional_edge;
	const auto union_materialized_edges =
		mtrc::numeric::materialize_graph_symmetric_edges(materialization_accumulators, "union", "minimum_distance");
	assert((union_materialized_edges == std::vector<Edge>{{0, 1, 3}, {0, 2, 2}}));
	const auto mutual_materialized_edges =
		mtrc::numeric::materialize_graph_symmetric_edges(materialization_accumulators, "mutual", "maximum_distance");
	assert((mutual_materialized_edges == std::vector<Edge>{{0, 2, 5}}));
	assert(rejects_invalid_argument([&materialization_accumulators] {
		(void)mtrc::numeric::materialize_graph_symmetric_edges(materialization_accumulators, "intersection",
																 "minimum_distance");
	}));
	assert(rejects_invalid_argument([&materialization_accumulators] {
		(void)mtrc::numeric::materialize_graph_symmetric_edges(materialization_accumulators, "union",
																 "median_distance");
	}));

	return 0;
}
