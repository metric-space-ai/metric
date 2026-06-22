#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;
	using mtrc::test::assert_matrix_close;
	using mtrc::test::close_to;

	const double sqrt_half = std::sqrt(0.5);

	Matrix left_rotation(2, 2);
	left_rotation(0, 0) = sqrt_half;
	left_rotation(0, 1) = -sqrt_half;
	left_rotation(1, 0) = sqrt_half;
	left_rotation(1, 1) = sqrt_half;

	Matrix right_rotation(2, 2);
	right_rotation(0, 0) = sqrt_half;
	right_rotation(0, 1) = sqrt_half;
	right_rotation(1, 0) = -sqrt_half;
	right_rotation(1, 1) = sqrt_half;

	Matrix singular_diagonal(2, 2, 0.0);
	singular_diagonal(0, 0) = 5.0;
	singular_diagonal(1, 1) = 2.0;

	const Matrix matrix = left_rotation * singular_diagonal * right_rotation;

	Vector values;
	mtrc::numeric::svd(matrix, values);
	assert(values.size() == 2);
	assert(close_to(values[0], 5.0));
	assert(close_to(values[1], 2.0));

	Matrix u;
	Matrix v;
	Vector decomposed_values;
	mtrc::numeric::svd(matrix, u, decomposed_values, v);

	Matrix diagonal(2, 2, 0.0);
	diagonal(0, 0) = decomposed_values[0];
	diagonal(1, 1) = decomposed_values[1];

	assert_matrix_close(u * diagonal * v, matrix);
	assert_matrix_close(mtrc::numeric::trans(u) * u, Matrix(2, 2, 0.0) + mtrc::numeric::IdentityMatrix<double>(2));
	assert_matrix_close(v * mtrc::numeric::trans(v), Matrix(2, 2, 0.0) + mtrc::numeric::IdentityMatrix<double>(2));

	Vector top_value;
	const auto top_count = mtrc::numeric::svd(matrix, top_value, 0UL, 0UL);
	assert(top_count == 1UL);
	assert(top_value.size() == 1);
	assert(close_to(top_value[0], 5.0));

	Vector middle_values;
	const auto middle_count = mtrc::numeric::svd(matrix, middle_values, 1.0, 4.0);
	assert(middle_count == 1UL);
	assert(middle_values.size() == 1);
	assert(close_to(middle_values[0], 2.0));

	Matrix partial_u;
	Matrix partial_v;
	Vector partial_values;
	const auto partial_count = mtrc::numeric::svd(matrix, partial_u, partial_values, partial_v, 0UL, 0UL);
	assert(partial_count == 1UL);
	assert(partial_u.rows() == 2);
	assert(partial_u.columns() == 1);
	assert(partial_values.size() == 1);
	assert(partial_v.rows() == 1);
	assert(partial_v.columns() == 2);
	assert(close_to(partial_values[0], 5.0));

	Matrix partial_diagonal(1, 1);
	partial_diagonal(0, 0) = partial_values[0];
	assert_matrix_close(partial_u * partial_diagonal * partial_v,
						mtrc::numeric::submatrix(u, 0UL, 0UL, 2UL, 1UL) *
							mtrc::numeric::submatrix(diagonal, 0UL, 0UL, 1UL, 1UL) *
							mtrc::numeric::submatrix(v, 0UL, 0UL, 1UL, 2UL));

	Matrix rank_two(3, 2, 0.0);
	rank_two(0, 0) = 4.0;
	rank_two(1, 1) = 2.0;
	assert(mtrc::numeric::rank(rank_two) == 2UL);

	Matrix rank_one(3, 2, 0.0);
	rank_one(0, 0) = 3.0;
	rank_one(1, 0) = 6.0;
	rank_one(2, 0) = 9.0;
	rank_one(0, 1) = 1.0;
	rank_one(1, 1) = 2.0;
	rank_one(2, 1) = 3.0;
	assert(mtrc::numeric::rank(rank_one) == 1UL);

	return 0;
}
