#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

int main()
{
	using mtrc::test::close_to;

	mtrc::numeric::DynamicVector<double> column(4);
	column[0] = 1.0;
	column[1] = -2.0;
	column[2] = 3.0;
	column[3] = 4.0;

	mtrc::numeric::DynamicVector<double, mtrc::numeric::rowVector> row(4);
	row[0] = 2.0;
	row[1] = 1.0;
	row[2] = -1.0;
	row[3] = 0.5;

	assert(column.size() == 4);
	assert(row.size() == 4);
	assert(mtrc::numeric::dot(column, mtrc::numeric::trans(row)) == -1.0);
	assert(close_to(mtrc::numeric::l2Norm(column), std::sqrt(30.0)));
	assert(mtrc::numeric::sum(column) == 6.0);
	assert(mtrc::numeric::mean(column) == 1.5);

	const auto shifted = column + mtrc::numeric::trans(row);
	assert(shifted.size() == 4);
	assert(shifted[0] == 3.0);
	assert(shifted[1] == -1.0);
	assert(shifted[2] == 2.0);
	assert(shifted[3] == 4.5);

	mtrc::numeric::DynamicMatrix<double> matrix(4, 2);
	matrix(0, 0) = 1.0;
	matrix(0, 1) = 2.0;
	matrix(1, 0) = 0.0;
	matrix(1, 1) = -1.0;
	matrix(2, 0) = 3.0;
	matrix(2, 1) = 1.0;
	matrix(3, 0) = 2.0;
	matrix(3, 1) = 0.0;

	const auto row_product = row * matrix;
	assert(row_product.size() == 2);
	assert(row_product[0] == 0.0);
	assert(row_product[1] == 2.0);

	const auto column_product = mtrc::numeric::trans(matrix) * column;
	assert(column_product.size() == 2);
	assert(column_product[0] == 18.0);
	assert(column_product[1] == 7.0);

	return 0;
}
