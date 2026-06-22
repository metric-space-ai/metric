#include <cassert>

#include <metric/numeric/Math.h>

int main()
{
	mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor> matrix(4, 4);
	matrix.reserve(6);

	matrix.append(0, 1, 2.0);
	matrix.append(0, 3, 5.0);
	matrix.finalize(0);

	matrix.append(1, 0, -1.0);
	matrix.finalize(1);

	matrix.append(2, 2, 4.0);
	matrix.finalize(2);

	matrix.append(3, 1, 3.0);
	matrix.append(3, 3, 1.0);
	matrix.finalize(3);

	assert(matrix.rows() == 4);
	assert(matrix.columns() == 4);
	assert(matrix.nonZeros() == 6);
	assert(matrix(0, 1) == 2.0);
	assert(matrix(1, 0) == -1.0);
	assert(matrix(3, 3) == 1.0);

	mtrc::numeric::DynamicVector<double> input(4);
	input[0] = 1.0;
	input[1] = 2.0;
	input[2] = 3.0;
	input[3] = 4.0;

	const auto product = matrix * input;
	assert(product.size() == 4);
	assert(product[0] == 24.0);
	assert(product[1] == -1.0);
	assert(product[2] == 12.0);
	assert(product[3] == 10.0);

	const auto transposed_product = mtrc::numeric::trans(matrix) * input;
	assert(transposed_product.size() == 4);
	assert(transposed_product[0] == -2.0);
	assert(transposed_product[1] == 14.0);
	assert(transposed_product[2] == 12.0);
	assert(transposed_product[3] == 9.0);

	return 0;
}
