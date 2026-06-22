#include <cassert>
#include <cstddef>
#include <vector>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

// Exercises the selection views that callers use but that had no smoke coverage:
// rows(), columns(), band(), elements(), plus the unchecked view-creation tag.
int main()
{
	using mtrc::test::close_to;

	mtrc::numeric::DynamicMatrix<double> matrix(3, 4);
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			matrix(row, column) = static_cast<double>(10 * row + column);
		}
	}

	// rows(): a row-selection view that aliases the parent and is writable.
	auto selected_rows = mtrc::numeric::rows(matrix, {0UL, 2UL});
	assert(selected_rows.rows() == 2);
	assert(selected_rows.columns() == 4);
	assert(close_to(selected_rows(0, 1), 1.0));
	assert(close_to(selected_rows(1, 3), 23.0));
	selected_rows(1, 0) = 99.0;
	assert(close_to(matrix(2, 0), 99.0));

	// columns(): a column-selection view, also writable through to the parent.
	auto selected_columns = mtrc::numeric::columns(matrix, {1UL, 3UL});
	assert(selected_columns.rows() == 3);
	assert(selected_columns.columns() == 2);
	assert(close_to(selected_columns(0, 0), 1.0));
	assert(close_to(selected_columns(0, 1), 3.0));
	selected_columns(0, 1) = -5.0;
	assert(close_to(matrix(0, 3), -5.0));

	// band(): diagonal bands indexed by signed offset (0 = main, >0 super, <0 sub).
	auto main_diagonal = mtrc::numeric::band(matrix, 0L);
	assert(main_diagonal.size() == 3);
	assert(close_to(main_diagonal[0], 0.0));
	assert(close_to(main_diagonal[1], 11.0));
	assert(close_to(main_diagonal[2], 22.0));

	auto super_diagonal = mtrc::numeric::band(matrix, 1L);
	assert(super_diagonal.size() == 3);
	assert(close_to(super_diagonal[1], 12.0));
	main_diagonal[1] = 77.0;
	assert(close_to(matrix(1, 1), 77.0));

	// elements(): an index-selection view over a vector.
	mtrc::numeric::DynamicVector<double> vector(6);
	for (std::size_t index = 0; index < vector.size(); ++index) {
		vector[index] = static_cast<double>(index + 1);
	}
	const std::vector<std::size_t> picks{4UL, 1UL, 5UL};
	auto picked = mtrc::numeric::elements(vector, picks.data(), picks.size());
	assert(picked.size() == 3);
	assert(close_to(picked[0], 5.0));
	assert(close_to(picked[1], 2.0));
	assert(close_to(picked[2], 6.0));
	picked[1] = 42.0;
	assert(close_to(vector[1], 42.0));

	// unchecked view creation skips the bounds validation but yields the same aliasing view.
	auto unchecked_slice = mtrc::numeric::subvector(vector, 2UL, 3UL, mtrc::numeric::unchecked);
	assert(unchecked_slice.size() == 3);
	assert(close_to(unchecked_slice[0], 3.0));
	unchecked_slice[0] = -8.0;
	assert(close_to(vector[2], -8.0));

	return 0;
}
