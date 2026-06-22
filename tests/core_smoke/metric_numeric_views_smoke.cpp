#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

int main()
{
	using mtrc::test::close_to;

	mtrc::numeric::DynamicMatrix<double> matrix(3, 4);
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			matrix(row, column) = static_cast<double>(10 * row + column);
		}
	}

	auto second_row = mtrc::numeric::row(matrix, 1);
	assert(second_row.size() == matrix.columns());
	assert(close_to(second_row[2], 12.0));
	second_row[0] = 20.0;
	assert(close_to(matrix(1, 0), 20.0));

	mtrc::numeric::DynamicVector<double> replacement_column(3);
	replacement_column[0] = 7.0;
	replacement_column[1] = 8.0;
	replacement_column[2] = 9.0;
	mtrc::numeric::column(matrix, 3) = replacement_column;
	assert(close_to(matrix(0, 3), 7.0));
	assert(close_to(matrix(1, 3), 8.0));
	assert(close_to(matrix(2, 3), 9.0));

	mtrc::numeric::DynamicMatrix<double> replacement_block(2, 2);
	replacement_block(0, 0) = -1.0;
	replacement_block(0, 1) = -2.0;
	replacement_block(1, 0) = -3.0;
	replacement_block(1, 1) = -4.0;
	mtrc::numeric::submatrix(matrix, 0, 1, 2, 2) = replacement_block;
	assert(close_to(matrix(0, 1), -1.0));
	assert(close_to(matrix(0, 2), -2.0));
	assert(close_to(matrix(1, 1), -3.0));
	assert(close_to(matrix(1, 2), -4.0));

	mtrc::numeric::DynamicVector<double> vector(5);
	for (std::size_t index = 0; index < vector.size(); ++index) {
		vector[index] = static_cast<double>(index + 1);
	}

	auto middle = mtrc::numeric::subvector(vector, 1, 3);
	assert(middle.size() == 3);
	assert(close_to(middle[0], 2.0));
	assert(close_to(middle[2], 4.0));
	middle[1] = 42.0;
	assert(close_to(vector[2], 42.0));

	mtrc::numeric::DynamicVector<double, mtrc::numeric::rowVector> row_vector(5);
	for (std::size_t index = 0; index < row_vector.size(); ++index) {
		row_vector[index] = static_cast<double>(10 + index);
	}
	mtrc::numeric::DynamicVector<double, mtrc::numeric::rowVector> replacement_slice(2);
	replacement_slice[0] = 7.0;
	replacement_slice[1] = 8.0;
	mtrc::numeric::subvector(row_vector, 2, 2) = replacement_slice;
	assert(close_to(row_vector[2], 7.0));
	assert(close_to(row_vector[3], 8.0));

	mtrc::numeric::DynamicMatrix<double, mtrc::numeric::columnMajor> column_major(3, 2);
	mtrc::numeric::column(column_major, 0) = replacement_column;
	assert(close_to(column_major(0, 0), 7.0));
	assert(close_to(column_major(1, 0), 8.0));
	assert(close_to(column_major(2, 0), 9.0));

	mtrc::numeric::DynamicVector<double, mtrc::numeric::rowVector> replacement_row(2);
	replacement_row[0] = 11.0;
	replacement_row[1] = 12.0;
	mtrc::numeric::row(column_major, 1) = replacement_row;
	assert(close_to(column_major(1, 0), 11.0));
	assert(close_to(column_major(1, 1), 12.0));

	return 0;
}
