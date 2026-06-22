#include <array>
#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

int main()
{
	using mtrc::test::close_to;

	std::array<double, 6> row_major_buffer{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
	mtrc::numeric::CustomMatrix<double, mtrc::numeric::unaligned, mtrc::numeric::unpadded,
								  mtrc::numeric::rowMajor>
		row_major(row_major_buffer.data(), 2, 3);

	assert(row_major.rows() == 2);
	assert(row_major.columns() == 3);
	assert(close_to(row_major(0, 2), 3.0));
	row_major(1, 1) = 42.0;
	assert(close_to(row_major_buffer[4], 42.0));

	mtrc::numeric::row(row_major, 0)[1] = -2.0;
	assert(close_to(row_major_buffer[1], -2.0));

	mtrc::numeric::DynamicVector<double> replacement_column(2);
	replacement_column[0] = 10.0;
	replacement_column[1] = 20.0;
	mtrc::numeric::column(row_major, 2) = replacement_column;
	assert(close_to(row_major_buffer[2], 10.0));
	assert(close_to(row_major_buffer[5], 20.0));

	mtrc::numeric::DynamicMatrix<double> copied = row_major;
	assert(close_to(copied(0, 0), 1.0));
	assert(close_to(copied(0, 1), -2.0));
	assert(close_to(copied(1, 1), 42.0));
	assert(close_to(copied(1, 2), 20.0));

	std::array<double, 6> column_major_buffer{1.0, 4.0, 2.0, 5.0, 3.0, 6.0};
	mtrc::numeric::CustomMatrix<double, mtrc::numeric::unaligned, mtrc::numeric::unpadded,
								  mtrc::numeric::columnMajor>
		column_major(column_major_buffer.data(), 2, 3);

	assert(column_major.rows() == 2);
	assert(column_major.columns() == 3);
	assert(close_to(column_major(0, 0), 1.0));
	assert(close_to(column_major(1, 0), 4.0));
	assert(close_to(column_major(0, 2), 3.0));
	assert(close_to(column_major(1, 2), 6.0));

	mtrc::numeric::DynamicVector<double, mtrc::numeric::rowVector> replacement_row(3);
	replacement_row[0] = 7.0;
	replacement_row[1] = 8.0;
	replacement_row[2] = 9.0;
	mtrc::numeric::row(column_major, 1) = replacement_row;
	assert(close_to(column_major_buffer[1], 7.0));
	assert(close_to(column_major_buffer[3], 8.0));
	assert(close_to(column_major_buffer[5], 9.0));

	const auto product = column_major * mtrc::numeric::trans(replacement_row);
	assert(product.size() == 2);
	assert(close_to(product[0], 1.0 * 7.0 + 2.0 * 8.0 + 3.0 * 9.0));
	assert(close_to(product[1], 7.0 * 7.0 + 8.0 * 8.0 + 9.0 * 9.0));

	return 0;
}
