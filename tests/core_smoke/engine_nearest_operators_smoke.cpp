#include <cassert>
#include <type_traits>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

int main()
{
	auto space = metric::make_space(std::vector<int>{0, 2, 5, 9}, AbsoluteDistance{});

	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);

	const auto exact = metric::operators::knn(space, 4, 3);
	static_assert(std::is_same<decltype(exact)::distance_type, int>::value);
	assert(exact.operator_name == "knn");
	assert(exact.representation == "metric_space");
	assert(exact.record_count == space.size());
	assert(exact.requested_count == 3);
	assert(exact.size() == 3);
	assert(exact[0].id == id2);
	assert(exact[0].distance == 1);
	assert(exact[1].id == id1);
	assert(exact[1].distance == 2);
	assert(exact[2].id == id0);
	assert(exact[2].distance == 4);

	const auto record_neighbors = metric::operators::knn(space, id0, 2);
	assert(record_neighbors.size() == 2);
	assert(record_neighbors[0].id == id1);
	assert(record_neighbors[0].distance == 2);
	assert(record_neighbors[1].id == id2);
	assert(record_neighbors[1].distance == 5);

	metric::representations::MatrixCache<decltype(space)> matrix(space);
	const auto cached = metric::operators::knn(matrix, id0, 2);
	assert(cached.representation == "distance_provider");
	assert(cached.size() == 2);
	assert(cached[0].id == id1);
	assert(cached[0].distance == record_neighbors[0].distance);
	assert(cached[1].id == id2);
	assert(cached[1].distance == record_neighbors[1].distance);

	const auto close_cached = metric::operators::range(matrix, id0, 5);
	assert(close_cached.operator_name == "range");
	assert(close_cached.size() == 2);
	assert(close_cached[0].id == id1);
	assert(close_cached[1].id == id2);

	metric::representations::CoverTreeIndex<decltype(space)> tree(space);
	const auto indexed = metric::operators::knn(tree, 4, 2);
	assert(indexed.representation == "neighbor_index");
	assert(indexed.size() == 2);
	assert(indexed[0].id == id2);
	assert(indexed[0].distance == 1);
	assert(indexed[1].id == id1);
	assert(indexed[1].distance == 2);

	return 0;
}
