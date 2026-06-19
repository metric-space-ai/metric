#include <cassert>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

int main()
{
	auto space = metric::make_space(std::vector<int>{0, 2, 5, 9}, AbsoluteDistance{});
	static_assert(metric::MetricSpaceLike_v<decltype(space)>);

	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);
	const auto id3 = space.id(3);

	metric::representations::ImplicitDistanceProvider<decltype(space)> implicit(space);
	static_assert(metric::DistanceProvider_v<decltype(implicit)>);
	assert(implicit.record_count() == space.size());
	assert(implicit.id(0) == id0);
	assert(implicit.contains(id2));
	assert(implicit.position_of(id2) == 2);
	assert(implicit.distance(id0, id2) == space.distance(id0, id2));
	const auto implicit_diagnostics = implicit.diagnostics();
	assert(implicit_diagnostics.kind == metric::representations::representation_kind::implicit_distance_provider);
	assert(implicit_diagnostics.exact == metric::representations::exactness::exact);
	assert(implicit_diagnostics.materialized == metric::representations::materialization::lazy);
	assert(implicit_diagnostics.updates == metric::representations::update_mode::live);
	assert(!implicit_diagnostics.stale);

	metric::representations::MatrixCache<decltype(space)> matrix(space);
	static_assert(metric::DistanceProvider_v<decltype(matrix)>);
	assert(matrix.record_count() == space.size());
	assert(matrix.id(2) == id2);
	assert(matrix.contains(id1));
	assert(matrix.distance(id0, id2) == 5);
	assert(matrix.distance(id1, id3) == space.distance(id1, id3));
	const auto matrix_diagnostics = matrix.diagnostics();
	assert(matrix_diagnostics.kind == metric::representations::representation_kind::matrix_cache);
	assert(matrix_diagnostics.cached_distances == space.size() * space.size());
	assert(matrix_diagnostics.distance_evaluations == space.size() * space.size());
	assert(matrix_diagnostics.built_for_version == space.version());
	assert(!matrix_diagnostics.stale);
	assert(matrix.stats().fill_ratio == 1.0);

	metric::representations::CoverTreeIndex<decltype(space)> tree(space);
	static_assert(metric::NeighborSearchIndex_v<decltype(tree)>);
	assert(tree.id(3) == id3);
	assert(tree.contains(id0));
	const auto tree_neighbors = tree.knn(4, 2);
	assert(tree_neighbors.size() == 2);
	assert(tree_neighbors[0].id == id2);
	assert(tree_neighbors[0].distance == 1);
	assert(tree_neighbors[1].id == id1);
	assert(tree_neighbors[1].distance == 2);
	const auto tree_diagnostics = tree.diagnostics();
	assert(tree_diagnostics.kind == metric::representations::representation_kind::cover_tree_index);
	assert(tree_diagnostics.records == space.size());
	assert(tree.stats().nodes == space.size());

	metric::representations::KnnGraphIndex<decltype(space)> graph(space, 1);
	static_assert(metric::NeighborSearchIndex_v<decltype(graph)>);
	assert(graph.k() == 1);
	assert(graph.id(0) == id0);
	assert(graph.contains(id3));
	assert(graph.neighbors(id0).size() == 1);
	assert(graph.neighbors(id0)[0].id == id1);
	assert(graph.neighbors(id3).size() == 1);
	assert(graph.neighbors(id3)[0].id == id2);
	const auto graph_diagnostics = graph.diagnostics();
	assert(graph_diagnostics.kind == metric::representations::representation_kind::knn_graph_index);
	assert(graph_diagnostics.exact == metric::representations::exactness::approximate);
	assert(graph.stats().edges == space.size());

	metric::representations::GraphTopology<decltype(space)> topology(space);
	static_assert(metric::GraphTopology_v<decltype(topology)>);
	assert(topology.id(1) == id1);
	assert(topology.contains(id2));
	topology.add_edge(id0, id1, matrix.distance(id0, id1));
	assert(topology.record_count() == space.size());
	assert(topology.edge_count() == 1);
	assert(topology.edges()[0].source == id0);
	assert(topology.edges()[0].target == id1);
	assert(topology.edges()[0].distance == 2);
	const auto topology_diagnostics = topology.diagnostics();
	assert(topology_diagnostics.kind == metric::representations::representation_kind::graph_topology);
	assert(topology_diagnostics.materialized == metric::representations::materialization::topology);

	assert(!implicit.is_stale());
	assert(!matrix.is_stale());
	assert(!tree.is_stale());
	assert(!graph.is_stale());
	assert(!topology.is_stale());

	const auto inserted_id = space.insert(12);
	assert(!implicit.is_stale());
	assert(implicit.contains(inserted_id));
	assert(implicit.position_of(inserted_id) == 4);
	assert(matrix.is_stale());
	assert(tree.is_stale());
	assert(graph.is_stale());
	assert(topology.is_stale());
	assert(matrix.diagnostics().stale);
	assert(!matrix.diagnostics().warnings.empty());

	assert(matrix.distance(id1, id3) == 7);
	assert(space.erase(id1));
	assert(!space.contains(id1));
	assert(space.position_of(id2) == 1);
	assert(matrix.contains(id1));
	assert(matrix.position_of(id3) == 3);
	assert(matrix.distance(id1, id3) == 7);

	metric::representations::MatrixCache<decltype(space)> refreshed_matrix(space);
	assert(refreshed_matrix.record_count() == space.size());
	assert(!refreshed_matrix.contains(id1));
	assert(refreshed_matrix.id(1) == id2);
	assert(refreshed_matrix.contains(inserted_id));
	assert(refreshed_matrix.distance(id0, id2) == space.distance(id0, id2));

	return 0;
}
