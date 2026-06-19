#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <metric/space.hpp>

struct AbsoluteDistance {
	using distance_type = int;

	auto operator()(const int lhs, const int rhs) const -> distance_type
	{
		return std::abs(lhs - rhs);
	}
};

int main()
{
	const std::vector<int> records = {3, 5, -10, 50, 1, -200, 200};
	const AbsoluteDistance metric;

	metric::MatrixSpace<int, AbsoluteDistance> matrix(records);
	metric::TreeSpace<int, AbsoluteDistance> tree(records);
	metric::GraphSpace<int, AbsoluteDistance> graph(records, 3, records.size());

	assert(matrix.size() == records.size());
	assert(tree.size() == records.size());
	assert(graph.size() == records.size());
	assert(tree.check_covering());

	for (std::size_t i = 0; i < records.size(); ++i) {
		for (std::size_t j = 0; j < records.size(); ++j) {
			const auto expected = metric(records[i], records[j]);
			assert(matrix(i, j) == expected);
			assert(graph(i, j) == expected);
		}
	}

	const auto matrix_neighbors = matrix.knn(4, 3);
	const auto tree_neighbors = tree.knn(4, 3);
	assert(matrix_neighbors.size() == tree_neighbors.size());

	std::vector<int> matrix_values;
	std::vector<int> tree_values;
	matrix_values.reserve(matrix_neighbors.size());
	tree_values.reserve(tree_neighbors.size());

	for (const auto &neighbor : matrix_neighbors) {
		matrix_values.push_back(records[neighbor.first]);
	}
	for (const auto &neighbor : tree_neighbors) {
		tree_values.push_back(neighbor.first->get_data());
	}

	std::sort(matrix_values.begin(), matrix_values.end());
	std::sort(tree_values.begin(), tree_values.end());
	assert(matrix_values == tree_values);

	std::cout << "nearest to 4:";
	for (const auto value : matrix_values) {
		std::cout << " " << value;
	}
	std::cout << "\n";

	return 0;
}
