#include <cassert>
#include <cstddef>
#include <vector>

#include <metric/numeric/Math.h>
#include <metric/utils/graph/connected_components.hpp>

namespace {

using Matrix = mtrc::numeric::DynamicMatrix<bool>;

auto connect(Matrix &matrix, std::size_t lhs, std::size_t rhs) -> void
{
	matrix(lhs, rhs) = true;
	matrix(rhs, lhs) = true;
}

auto edge_count(const Matrix &matrix) -> std::size_t
{
	std::size_t count = 0;
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			if (matrix(row, column)) {
				++count;
			}
		}
	}
	return count;
}

} // namespace

int main()
{
	Matrix graph(6, 6, false);
	connect(graph, 0, 1);
	connect(graph, 1, 2);
	connect(graph, 3, 4);

	const auto components = mtrc::graph::all_connected_components(graph);
	assert(components.size() == 2);
	assert(edge_count(components[0]) == 4);
	assert(edge_count(components[1]) == 2);
	assert(components[0](0, 1));
	assert(components[0](1, 2));
	assert(!components[0](3, 4));
	assert(components[1](3, 4));
	assert(!components[1](0, 1));

	const auto largest = mtrc::graph::largest_connected_component(graph);
	assert(largest.size() == 1);
	assert(edge_count(largest[0]) == 4);
	assert(largest[0](0, 1));
	assert(largest[0](1, 2));
	assert(!largest[0](3, 4));

	const auto limited = mtrc::graph::connected_components(graph, 1);
	assert(limited.size() == 1);
	assert(edge_count(limited[0]) == 4);

	return 0;
}
