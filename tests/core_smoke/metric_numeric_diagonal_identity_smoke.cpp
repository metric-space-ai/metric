#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

int main()
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using DiagonalMatrix = mtrc::numeric::DiagonalMatrix<Matrix>;
	using mtrc::test::close_to;

	DiagonalMatrix scale(3, 0.0);
	scale(0, 0) = 2.0;
	scale(1, 1) = 3.0;
	scale(2, 2) = 4.0;

	assert(scale.rows() == 3);
	assert(scale.columns() == 3);
	assert(close_to(scale(0, 0), 2.0));
	assert(close_to(scale(1, 1), 3.0));
	assert(close_to(scale(2, 2), 4.0));
	assert(close_to(scale(0, 1), 0.0));
	assert(close_to(scale(2, 0), 0.0));

	Matrix covariance(3, 3);
	covariance(0, 0) = 1.0;
	covariance(0, 1) = 2.0;
	covariance(0, 2) = 3.0;
	covariance(1, 0) = 4.0;
	covariance(1, 1) = 5.0;
	covariance(1, 2) = 6.0;
	covariance(2, 0) = 7.0;
	covariance(2, 1) = 8.0;
	covariance(2, 2) = 9.0;

	const Matrix sandwich = scale * covariance * scale;
	assert(close_to(sandwich(0, 0), 4.0));
	assert(close_to(sandwich(0, 1), 12.0));
	assert(close_to(sandwich(0, 2), 24.0));
	assert(close_to(sandwich(1, 0), 24.0));
	assert(close_to(sandwich(1, 1), 45.0));
	assert(close_to(sandwich(1, 2), 72.0));
	assert(close_to(sandwich(2, 0), 56.0));
	assert(close_to(sandwich(2, 1), 96.0));
	assert(close_to(sandwich(2, 2), 144.0));

	mtrc::numeric::IdentityMatrix<double> identity(3);
	const Matrix regularized = covariance + identity * 0.25;
	assert(close_to(regularized(0, 0), 1.25));
	assert(close_to(regularized(1, 1), 5.25));
	assert(close_to(regularized(2, 2), 9.25));
	assert(close_to(regularized(0, 1), 2.0));
	assert(close_to(regularized(2, 1), 8.0));

	mtrc::numeric::DynamicVector<double> vector(3);
	vector[0] = 2.0;
	vector[1] = -1.0;
	vector[2] = 0.5;

	const mtrc::numeric::DynamicVector<double> scaled = scale * vector;
	assert(close_to(scaled[0], 4.0));
	assert(close_to(scaled[1], -3.0));
	assert(close_to(scaled[2], 2.0));

	const mtrc::numeric::DynamicVector<double> unchanged = identity * vector;
	assert(close_to(unchanged[0], vector[0]));
	assert(close_to(unchanged[1], vector[1]));
	assert(close_to(unchanged[2], vector[2]));

	return 0;
}
