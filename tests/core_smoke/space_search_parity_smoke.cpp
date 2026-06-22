// Exact-search parity for the legacy explicit representations: the cover Tree and the distance Matrix must
// return the same nearest neighbours as a brute-force scan, for both a signed and an UNSIGNED distance type.
// The unsigned case guards the cover-tree pruning fix (the previous `dist_child - 2*covdist` underflowed for
// unsigned Distance and could prune the branch holding the true nearest neighbour). Also covers the batch
// insert_if() dedup (must test the nearest existing record, not just the root) and Matrix edge behaviour.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "metric/space.hpp"

struct SignedAbsoluteDistance {
	using distance_type = int;
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct UnsignedAbsoluteDistance {
	using distance_type = unsigned;
	auto operator()(int lhs, int rhs) const -> unsigned
	{
		return static_cast<unsigned>(lhs > rhs ? lhs - rhs : rhs - lhs);
	}
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

template <typename Metric>
static auto brute_force_distances(const std::vector<int> &data, int query, std::size_t k, const Metric &metric)
	-> std::vector<decltype(metric(query, query))>
{
	std::vector<decltype(metric(query, query))> distances;
	distances.reserve(data.size());
	for (const int value : data) {
		distances.push_back(metric(query, value));
	}
	std::sort(distances.begin(), distances.end());
	if (distances.size() > k) {
		distances.resize(k);
	}
	return distances;
}

template <typename Metric> static void run_parity(const Metric &metric)
{
	const std::vector<int> data{0, 3, 3, 7, 12, 18, 25, 25, 31, 40, 41, 55, 60, 60, 72, 88};
	const std::vector<int> queries{5, 25, 0, 41, 100, 13, 59};
	const std::vector<std::size_t> ks{1, 3, 5, 16, 99};

	mtrc::Tree<int, Metric> tree(data, -1, metric);
	mtrc::Matrix<int, Metric> matrix(data, metric);
	assert(tree.size() == data.size());
	assert(matrix.size() == data.size());

	for (const int query : queries) {
		for (const std::size_t k : ks) {
			const auto expected = brute_force_distances(data, query, k, metric);

			// cover tree kNN — compare the multiset of distances (tie order is implementation-defined).
			auto tree_pairs = tree.knn(query, static_cast<unsigned>(k));
			std::vector<decltype(metric(query, query))> tree_distances;
			for (const auto &pair : tree_pairs) {
				tree_distances.push_back(pair.second);
			}
			std::sort(tree_distances.begin(), tree_distances.end());
			assert(tree_distances == expected);

			// distance matrix kNN.
			auto matrix_pairs = matrix.knn(query, static_cast<unsigned>(k));
			std::vector<decltype(metric(query, query))> matrix_distances;
			for (const auto &pair : matrix_pairs) {
				matrix_distances.push_back(pair.second);
			}
			std::sort(matrix_distances.begin(), matrix_distances.end());
			assert(matrix_distances == expected);
		}

		// nn() must return a record at the true minimum distance (the specific tie can differ).
		const auto minimum = brute_force_distances(data, query, 1, metric).front();
		auto tree_nn = tree.nn(query);
		assert(tree_nn != nullptr);
		assert(metric(query, tree_nn->get_data()) == minimum);
		assert(metric(query, matrix[matrix.nn(query)]) == minimum);
	}
}

static void tree_batch_insert_if_uses_nearest_neighbour()
{
	// Records spread far apart; each probe sits next to a DIFFERENT existing record (only one of which can be
	// the tree root). With the nearest-neighbour test all probes are rejected; testing only the root distance
	// would wrongly accept the probes whose nearest record is not the root.
	mtrc::Tree<int, SignedAbsoluteDistance> tree(std::vector<int>{0, 50, 100});
	assert(tree.size() == 3);
	const auto inserted = tree.insert_if(std::vector<int>{1, 51, 101}, 2);
	assert(inserted == 0);
	assert(tree.size() == 3);

	// A genuinely isolated probe is still inserted.
	const auto inserted_far = tree.insert_if(std::vector<int>{500}, 2);
	assert(inserted_far == 1);
	assert(tree.size() == 4);

	// A zero-neighbour request is valid and returns no neighbours. It must not
	// invoke UB: knn_ previously read nnList.back() on an empty nnList for k == 0.
	assert(tree.knn(0, 0).empty());
}

static void matrix_edges()
{
	mtrc::Matrix<int, SignedAbsoluteDistance> matrix(std::vector<int>{0, 10, 20});
	// insert_if dedups against the nearest existing record.
	assert(matrix.insert_if(11, 5).second == false); // nearest is 10 (dist 1) <= 5 -> rejected
	assert(matrix.insert_if(40, 5).second == true);  // nearest is 20 (dist 20) > 5 -> inserted
	assert(matrix.size() == 4);

	// knn clamps to size and rnn respects the range.
	assert(matrix.knn(0, 99).size() == matrix.size());
	for (const auto &pair : matrix.rnn(0, 12)) {
		assert(pair.second <= 12);
	}

	// erase on an out-of-range id throws (documented behaviour).
	assert(throws([&matrix]() { (void)matrix.erase(99); }));
}

int main()
{
	run_parity(SignedAbsoluteDistance{});
	run_parity(UnsignedAbsoluteDistance{});
	tree_batch_insert_if_uses_nearest_neighbour();
	matrix_edges();
	return 0;
}
