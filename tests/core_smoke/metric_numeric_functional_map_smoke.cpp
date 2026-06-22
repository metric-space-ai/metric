#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using SparseMatrix = mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor>;
	using SparseVector = mtrc::numeric::CompressedVector<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;
	using mtrc::test::close_to;

	Matrix distances(2, 3);
	distances(0, 0) = -1.0;
	distances(0, 1) = 0.25;
	distances(0, 2) = 2.5;
	distances(1, 0) = 3.0;
	distances(1, 1) = -4.0;
	distances(1, 2) = 0.75;

	const Matrix clipped = mtrc::numeric::map(distances, [](double value) { return value > 1.0 ? 1.0 : value; });
	assert(close_to(clipped(0, 0), -1.0));
	assert(close_to(clipped(0, 2), 1.0));
	assert(close_to(clipped(1, 0), 1.0));
	assert(close_to(clipped(1, 2), 0.75));

	const Matrix non_negative = mtrc::numeric::map(distances, [](double value) { return value < 0.0 ? 0.0 : value; });
	assert(close_to(non_negative(0, 0), 0.0));
	assert(close_to(non_negative(1, 1), 0.0));
	assert(close_to(non_negative(0, 2), 2.5));

	const mtrc::numeric::DynamicMatrix<bool> lower_bound =
		mtrc::numeric::map(distances, [](double value) { return value >= 0.0; });
	const mtrc::numeric::DynamicMatrix<bool> upper_bound =
		mtrc::numeric::map(distances, [](double value) { return value <= 1.0; });
	const mtrc::numeric::DynamicMatrix<bool> in_band = lower_bound && upper_bound;
	assert(!in_band(0, 0));
	assert(in_band(0, 1));
	assert(!in_band(0, 2));
	assert(!in_band(1, 0));
	assert(!in_band(1, 1));
	assert(in_band(1, 2));

	Vector values(4);
	values[0] = -2.0;
	values[1] = -0.5;
	values[2] = 0.25;
	values[3] = 3.0;

	const Vector thresholded_values =
		mtrc::numeric::map(values, [](double value) { return value < 0.0 ? 0.0 : value * value; });
	assert(close_to(thresholded_values[0], 0.0));
	assert(close_to(thresholded_values[1], 0.0));
	assert(close_to(thresholded_values[2], 0.0625));
	assert(close_to(thresholded_values[3], 9.0));

	SparseMatrix sparse(3, 3);
	sparse.reserve(4);
	sparse.append(0, 1, -4.0);
	sparse.append(0, 2, 9.0);
	sparse.finalize(0);
	sparse.append(1, 0, 2.0);
	sparse.finalize(1);
	sparse.append(2, 2, -3.0);
	sparse.finalize(2);

	const SparseMatrix sparse_clipped =
		mtrc::numeric::map(sparse, [](double value) { return value > 3.0 ? 3.0 : value; });
	assert(sparse_clipped.nonZeros() == 4);
	assert(close_to(sparse_clipped(0, 1), -4.0));
	assert(close_to(sparse_clipped(0, 2), 3.0));
	assert(close_to(sparse_clipped(1, 0), 2.0));
	assert(close_to(sparse_clipped(2, 2), -3.0));
	assert(close_to(sparse_clipped(2, 0), 0.0));

	SparseVector sparse_vector(5);
	sparse_vector.reserve(3);
	sparse_vector.append(0, -2.0);
	sparse_vector.append(2, 4.0);
	sparse_vector.set(4, 6.0);

	const SparseVector sparse_vector_clipped =
		mtrc::numeric::map(sparse_vector, [](double value) { return value > 3.0 ? 3.0 : value; });
	assert(sparse_vector_clipped.nonZeros() == 3);
	assert(close_to(sparse_vector_clipped[0], -2.0));
	assert(close_to(sparse_vector_clipped[2], 3.0));
	assert(close_to(sparse_vector_clipped[4], 3.0));
	assert(close_to(sparse_vector_clipped[1], 0.0));

	return 0;
}
