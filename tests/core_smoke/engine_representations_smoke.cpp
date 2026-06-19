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
	assert(implicit.distance(id0, id2) == space.distance(id0, id2));

	metric::representations::MatrixCache<decltype(space)> matrix(space);
	static_assert(metric::DistanceProvider_v<decltype(matrix)>);
	assert(matrix.record_count() == space.size());
	assert(matrix.distance(id0, id2) == 5);
	assert(matrix.distance(id1, id3) == space.distance(id1, id3));

	metric::representations::CoverTreeIndex<decltype(space)> tree(space);
	static_assert(metric::NeighborSearchIndex_v<decltype(tree)>);
	const auto tree_neighbors = tree.knn(4, 2);
	assert(tree_neighbors.size() == 2);
	assert(tree_neighbors[0].id == id2);
	assert(tree_neighbors[0].distance == 1);
	assert(tree_neighbors[1].id == id1);
	assert(tree_neighbors[1].distance == 2);

	metric::representations::KnnGraphIndex<decltype(space)> graph(space, 1);
	static_assert(metric::NeighborSearchIndex_v<decltype(graph)>);
	assert(graph.k() == 1);
	assert(graph.neighbors(id0).size() == 1);
	assert(graph.neighbors(id0)[0].id == id1);
	assert(graph.neighbors(id3).size() == 1);
	assert(graph.neighbors(id3)[0].id == id2);

	metric::representations::GraphTopology<decltype(space)> topology(space);
	static_assert(metric::GraphTopology_v<decltype(topology)>);
	topology.add_edge(id0, id1, matrix.distance(id0, id1));
	assert(topology.record_count() == space.size());
	assert(topology.edge_count() == 1);
	assert(topology.edges()[0].source == id0);
	assert(topology.edges()[0].target == id1);
	assert(topology.edges()[0].distance == 2);

	assert(!implicit.is_stale());
	assert(!matrix.is_stale());
	assert(!tree.is_stale());
	assert(!graph.is_stale());
	assert(!topology.is_stale());

	space.touch();
	assert(implicit.is_stale());
	assert(matrix.is_stale());
	assert(tree.is_stale());
	assert(graph.is_stale());
	assert(topology.is_stale());

	return 0;
}
