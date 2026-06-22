#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

int main()
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;
	using mtrc::test::close_to;

	Matrix matrix(2, 3);
	matrix(0, 0) = 4.0;
	matrix(0, 1) = 9.0;
	matrix(0, 2) = 16.0;
	matrix(1, 0) = 25.0;
	matrix(1, 1) = 36.0;
	matrix(1, 2) = 49.0;

	const Matrix roots = mtrc::numeric::sqrt(matrix);
	assert(close_to(roots(0, 0), 2.0));
	assert(close_to(roots(0, 1), 3.0));
	assert(close_to(roots(1, 2), 7.0));

	const Matrix inverse_roots = mtrc::numeric::invsqrt(matrix);
	assert(close_to(inverse_roots(0, 0), 0.5));
	assert(close_to(inverse_roots(0, 2), 0.25));
	assert(close_to(inverse_roots(1, 2), 1.0 / 7.0));
	assert(close_to(mtrc::numeric::sum(inverse_roots), 0.5 + 1.0 / 3.0 + 0.25 + 0.2 + 1.0 / 6.0 + 1.0 / 7.0));

	Matrix signed_matrix(2, 2);
	signed_matrix(0, 0) = -2.0;
	signed_matrix(0, 1) = 3.0;
	signed_matrix(1, 0) = -4.0;
	signed_matrix(1, 1) = 5.0;

	const Matrix absolute = mtrc::numeric::abs(signed_matrix);
	const Matrix squared = mtrc::numeric::pow(absolute, 2.0);
	assert(close_to(squared(0, 0), 4.0));
	assert(close_to(squared(0, 1), 9.0));
	assert(close_to(squared(1, 0), 16.0));
	assert(close_to(squared(1, 1), 25.0));

	Vector vector(4);
	vector[0] = 1.0;
	vector[1] = 4.0;
	vector[2] = 9.0;
	vector[3] = 16.0;

	const Vector vector_roots = mtrc::numeric::sqrt(vector);
	const Vector vector_inverse_roots = mtrc::numeric::invsqrt(vector);
	const Vector vector_powers = mtrc::numeric::pow(vector_roots, 3.0);
	assert(close_to(vector_roots[3], 4.0));
	assert(close_to(vector_inverse_roots[0], 1.0));
	assert(close_to(vector_inverse_roots[3], 0.25));
	assert(close_to(vector_powers[0], 1.0));
	assert(close_to(vector_powers[1], 8.0));
	assert(close_to(vector_powers[3], 64.0));

	mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor> sparse_matrix(3, 3);
	sparse_matrix.reserve(3);
	sparse_matrix.append(0, 1, 4.0);
	sparse_matrix.finalize(0);
	sparse_matrix.append(1, 2, 9.0);
	sparse_matrix.finalize(1);
	sparse_matrix.append(2, 0, 16.0);
	sparse_matrix.finalize(2);

	const mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor> sparse_roots =
		mtrc::numeric::sqrt(sparse_matrix);
	const mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor> sparse_inverse_roots =
		mtrc::numeric::invsqrt(sparse_matrix);
	const mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor> sparse_squared =
		mtrc::numeric::pow(sparse_roots, 2.0);
	assert(sparse_roots.nonZeros() == 3);
	assert(close_to(sparse_roots(0, 1), 2.0));
	assert(close_to(sparse_roots(1, 2), 3.0));
	assert(close_to(sparse_roots(2, 0), 4.0));
	assert(close_to(sparse_roots(0, 0), 0.0));
	assert(close_to(sparse_inverse_roots(0, 1), 0.5));
	assert(close_to(sparse_inverse_roots(1, 2), 1.0 / 3.0));
	assert(close_to(sparse_inverse_roots(2, 0), 0.25));
	assert(close_to(sparse_squared(2, 0), 16.0));

	mtrc::numeric::CompressedVector<double> sparse_vector(5);
	sparse_vector.reserve(3);
	sparse_vector.append(0, 4.0);
	sparse_vector.append(2, 9.0);
	sparse_vector.append(4, 16.0);

	const mtrc::numeric::CompressedVector<double> sparse_vector_roots = mtrc::numeric::sqrt(sparse_vector);
	const mtrc::numeric::CompressedVector<double> sparse_vector_inverse_roots =
		mtrc::numeric::invsqrt(sparse_vector);
	const mtrc::numeric::CompressedVector<double> sparse_vector_powers =
		mtrc::numeric::pow(sparse_vector_roots, 2.0);
	assert(sparse_vector_roots.nonZeros() == 3);
	assert(close_to(sparse_vector_roots[0], 2.0));
	assert(close_to(sparse_vector_roots[2], 3.0));
	assert(close_to(sparse_vector_roots[4], 4.0));
	assert(close_to(sparse_vector_roots[1], 0.0));
	assert(close_to(sparse_vector_inverse_roots[4], 0.25));
	assert(close_to(sparse_vector_powers[2], 9.0));

	return 0;
}
