#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;
	using mtrc::test::assert_matrix_close;
	using mtrc::test::assert_vector_close;
	using mtrc::test::close_to;

	Matrix covariance(2, 2);
	covariance(0, 0) = 4.0;
	covariance(0, 1) = 1.0;
	covariance(1, 0) = 1.0;
	covariance(1, 1) = 3.0;

	const Matrix inverse = mtrc::numeric::inv(covariance);
	assert(close_to(inverse(0, 0), 3.0 / 11.0));
	assert(close_to(inverse(0, 1), -1.0 / 11.0));
	assert(close_to(inverse(1, 0), -1.0 / 11.0));
	assert(close_to(inverse(1, 1), 4.0 / 11.0));

	Matrix identity(2, 2, 0.0);
	identity(0, 0) = 1.0;
	identity(1, 1) = 1.0;
	assert_matrix_close(covariance * inverse, identity);
	assert_matrix_close(inverse * covariance, identity);

	Vector observation(2);
	observation[0] = 5.0;
	observation[1] = 7.0;

	const Vector solved = inverse * observation;
	assert(close_to(solved[0], 8.0 / 11.0));
	assert(close_to(solved[1], 23.0 / 11.0));
	assert_vector_close(covariance * solved, observation);

	Matrix lower;
	mtrc::numeric::llh(covariance, lower);
	assert(lower.rows() == 2);
	assert(lower.columns() == 2);
	assert(close_to(lower(0, 0), 2.0));
	assert(close_to(lower(0, 1), 0.0));
	assert(close_to(lower(1, 0), 0.5));
	assert(close_to(lower(1, 1), std::sqrt(11.0) / 2.0));

	assert_matrix_close(lower * mtrc::numeric::trans(lower), covariance);

	Matrix three_by_three(3, 3);
	three_by_three(0, 0) = 6.0;
	three_by_three(0, 1) = 15.0;
	three_by_three(0, 2) = 55.0;
	three_by_three(1, 0) = 15.0;
	three_by_three(1, 1) = 55.0;
	three_by_three(1, 2) = 225.0;
	three_by_three(2, 0) = 55.0;
	three_by_three(2, 1) = 225.0;
	three_by_three(2, 2) = 979.0;

	Matrix lower_three;
	mtrc::numeric::llh(three_by_three, lower_three);
	assert(close_to(lower_three(0, 0), std::sqrt(6.0)));
	assert(close_to(lower_three(0, 1), 0.0));
	assert(close_to(lower_three(0, 2), 0.0));
	assert(close_to(lower_three(1, 0), 15.0 / std::sqrt(6.0)));
	assert(close_to(lower_three(1, 1), std::sqrt(17.5)));
	assert(close_to(lower_three(1, 2), 0.0));
	assert(close_to(lower_three(2, 0), 55.0 / std::sqrt(6.0)));
	assert(close_to(lower_three(2, 1), 87.5 / std::sqrt(17.5)));
	assert(close_to(lower_three(2, 2), std::sqrt(37.333333333333333)));
	assert_matrix_close(lower_three * mtrc::numeric::trans(lower_three), three_by_three);

	return 0;
}
