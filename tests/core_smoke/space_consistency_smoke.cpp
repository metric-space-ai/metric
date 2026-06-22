#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <type_traits>
#include <vector>

#include "metric/space.hpp"

struct AbsoluteDistance {
	using distance_type = int;

	auto operator()(const int lhs, const int rhs) const -> distance_type { return std::abs(lhs - rhs); }
};

int main()
{
	const std::vector<int> records = {3, 5, -10, 50, 1, -200, 200};
	const AbsoluteDistance metric;

	static_assert(std::is_same_v<mtrc::MatrixSpace<int, AbsoluteDistance>, mtrc::Matrix<int, AbsoluteDistance>>);
	static_assert(std::is_same_v<mtrc::TreeSpace<int, AbsoluteDistance>, mtrc::Tree<int, AbsoluteDistance>>);
	static_assert(std::is_same_v<mtrc::GraphSpace<int, AbsoluteDistance>, mtrc::KNNGraph<int, AbsoluteDistance>>);

	mtrc::MatrixSpace<int, AbsoluteDistance> matrix(records);
	mtrc::TreeSpace<int, AbsoluteDistance> tree(records);
	mtrc::GraphSpace<int, AbsoluteDistance> graph(records, 3, records.size());

	assert(matrix.size() == records.size());
	assert(tree.size() == records.size());
	assert(graph.size() == records.size());
	assert(tree.check_covering());

	for (std::size_t i = 0; i < records.size(); ++i) {
		assert(matrix[i] == records[i]);
		assert(graph[i] == records[i]);

		for (std::size_t j = 0; j < records.size(); ++j) {
			const auto expected = metric(records[i], records[j]);
			assert(matrix(i, j) == expected);
			assert(graph(i, j) == expected);
		}
	}

	assert(tree.nn(4)->get_data() == 3 || tree.nn(4)->get_data() == 5);
	assert(matrix.nn(4) == 0 || matrix.nn(4) == 1);

	const auto matrix_neighbors = matrix.knn(4, 3);
	const auto tree_neighbors = tree.knn(4, 3);

	assert(matrix_neighbors.size() == 3);
	assert(tree_neighbors.size() == 3);

	std::vector<int> matrix_values;
	std::vector<int> tree_values;
	matrix_values.reserve(3);
	tree_values.reserve(3);

	for (const auto &neighbor : matrix_neighbors) {
		matrix_values.push_back(records[neighbor.first]);
	}
	for (const auto &neighbor : tree_neighbors) {
		tree_values.push_back(neighbor.first->get_data());
	}

	std::sort(matrix_values.begin(), matrix_values.end());
	std::sort(tree_values.begin(), tree_values.end());

	assert(matrix_values == tree_values);

	return 0;
}
