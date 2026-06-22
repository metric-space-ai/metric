// Regression coverage for the legacy explicit metric-space representations (mtrc::Matrix / mtrc::Tree /
// mtrc::KNNGraph). These guard the correctness/UB fixes made in the LEVEL1_SPACE hardening pass:
//   * Tree: empty-tree guards (nn/knn/rnn/erase/distance), operator() resolves IDs through index_map,
//     and index_map is renumbered by data position after erase.
//   * KNNGraph: _distance_matrix is rebuilt (not appended) on mutation, knn() is callable, and empty-graph
//     nn()/insert_if() no longer index an empty result.
//   * Matrix: operator[] performs the documented bounds check.

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "metric/space.hpp"

struct AbsoluteDistance {
	using distance_type = int;
	auto operator()(int lhs, int rhs) const -> distance_type { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

template <typename Callable> static auto throws(Callable &&call) -> bool
{
	try {
		call();
	} catch (...) {
		return true;
	}
	return false;
}

static void tree_empty_tree_is_safe()
{
	mtrc::Tree<int, AbsoluteDistance> empty;
	assert(empty.empty());
	assert(empty.size() == 0);
	assert(empty.nn(5) == nullptr);        // no null-root dereference
	assert(empty.knn(5, 3).empty());
	assert(empty.rnn(5, 100).empty());
	assert(!empty.erase(5));               // erase on empty returns false, no crash
	assert(empty.distance(1, 2) == 0);     // nn(1) == nn(2) == nullptr -> 0

	// insert_if on an empty tree must insert the first record rather than dereference a null root
	std::vector<int> batch{4, 5};
	mtrc::Tree<int, AbsoluteDistance> seeded;
	const auto inserted = seeded.insert_if(batch, 1);
	assert(inserted >= 1);
	assert(!seeded.empty());
}

static void tree_operator_resolves_ids_after_erase()
{
	std::vector<int> records{0, 10, 20, 30};
	mtrc::Tree<int, AbsoluteDistance> tree(records);
	assert(tree.size() == 4);

	// IDs are assigned in insertion order: 0->0, 1->10, 2->20, 3->30.
	assert(tree(0, 0) == 0);
	assert(tree(2, 0) == tree(0, 2)); // symmetric

	// Erase the value behind ID 1. Surviving high IDs must still resolve correctly through index_map
	// (before the fix operator() indexed data[] by raw ID, which is out of range / wrong after compaction).
	assert(tree.erase(10));
	assert(tree.size() == 3);
	assert(throws([&tree]() { (void)tree(1, 0); })); // erased ID throws
	assert(tree(3, 3) == 0);
	assert(tree(2, 3) == tree(3, 2));

	auto nearest = tree.nn(20);
	assert(nearest != nullptr && nearest->get_data() == 20);
}

static void knn_graph_mutation_and_guards()
{
	std::vector<int> records{0, 1, 2, 3, 4, 5};
	mtrc::KNNGraph<int, AbsoluteDistance> graph(records, 3, records.size());
	assert(graph.size() == 6);

	// knn() is now callable (it was declared const while calling a non-const greedy walk).
	const auto neighbours = graph.knn(2, 3);
	assert(neighbours.size() <= 3);
	assert(graph.nn(2) < graph.size());

	// insert() rebuilds _distance_matrix instead of appending; operator() must read the fresh NxN block.
	const auto inserted = graph.insert(6);
	assert(inserted == 6);
	assert(graph.size() == 7);
	const AbsoluteDistance metric;
	assert(graph(0, 6) == metric(0, 6));
	assert(graph(6, 0) == metric(6, 0));

	graph.erase(0);
	assert(graph.size() == 6);

	// Empty graph: nn() throws instead of indexing an empty result; insert_if() inserts the first record.
	mtrc::KNNGraph<int, AbsoluteDistance> empty(std::vector<int>{}, 3, 1);
	assert(empty.size() == 0);
	assert(throws([&empty]() { (void)empty.nn(1); }));
	const auto decision = empty.insert_if(7, 1);
	assert(decision.second);
	assert(empty.size() == 1);
}

static void matrix_bounds_check()
{
	std::vector<int> records{0, 10, 20};
	mtrc::Matrix<int, AbsoluteDistance> matrix(records);
	assert(matrix.size() == 3);
	assert(matrix[1] == 10);
	assert(matrix(0, 2) == 20);
	assert(throws([&matrix]() { (void)matrix[5]; })); // operator[] now bounds-checks
}

int main()
{
	tree_empty_tree_is_safe();
	tree_operator_resolves_ids_after_erase();
	knn_graph_mutation_and_guards();
	matrix_bounds_check();
	return 0;
}
