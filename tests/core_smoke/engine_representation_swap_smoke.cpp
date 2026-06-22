#include <cassert>
#include <string>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"

namespace {

template <typename NeighborSet>
auto assert_top_neighbor(const NeighborSet &neighbors, mtrc::RecordId expected,
						 typename NeighborSet::distance_type distance) -> void
{
	assert(neighbors.operator_name == "knn");
	assert(neighbors.size() == 2);
	assert(neighbors[0].id == expected);
	assert(neighbors[0].distance == distance);
}

} // namespace

int main()
{
	std::vector<std::string> records = {"metric", "metrics", "matrix", "tree"};
	auto space = mtrc::make_space(records, mtrc::Edit<char>{});
	const std::string query = "metricks";

	const auto implicit = mtrc::find_neighbors(space, query, mtrc::count{2});
	assert(implicit.representation == "metric_space");
	assert_top_neighbor(implicit, space.id(1), 1);

	const auto tree = mtrc::find_neighbors(space, query, mtrc::count{2}, mtrc::stats::search::cover_tree{});
	assert(tree.representation == "cover_tree_index");
	assert_top_neighbor(tree, implicit[0].id, implicit[0].distance);

	const auto graph = mtrc::find_neighbors(space, query, mtrc::count{2}, mtrc::stats::search::knn_graph{3});
	assert(graph.representation == "knn_graph_index");
	assert_top_neighbor(graph, implicit[0].id, implicit[0].distance);

	const auto query_id = space.id(1);
	const auto id_neighbors = mtrc::find_neighbors(space, query_id, mtrc::count{2});
	const auto matrix_neighbors =
		mtrc::find_neighbors(space, query_id, mtrc::count{2}, mtrc::stats::search::distance_table{});
	assert(matrix_neighbors.representation == "distance_table");
	assert_top_neighbor(matrix_neighbors, id_neighbors[0].id, id_neighbors[0].distance);

	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);
	mtrc::space::storage::CoverTreeIndex<decltype(space)> tree_index(space);
	mtrc::space::storage::KnnGraphIndex<decltype(space)> graph_index(space, 2);
	assert(!matrix.is_stale());
	assert(!tree_index.is_stale());
	assert(!graph_index.is_stale());

	(void)space.insert("metrica");
	assert(matrix.is_stale());
	assert(tree_index.is_stale());
	assert(graph_index.is_stale());

	return 0;
}
