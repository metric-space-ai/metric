#include <cassert>
#include <cmath>
#include <stdexcept>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

namespace {

auto assert_det_rejects_non_square() -> void
{
	mtrc::numeric::DynamicMatrix<double> non_square(2, 3, 1.0);

	bool rejected = false;
	try {
		(void)mtrc::numeric::det(non_square);
	} catch (const std::invalid_argument &) {
		rejected = true;
	}

	assert(rejected);
}

} // namespace

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;
	using mtrc::test::close_to;

	Matrix covariance(2, 2);
	covariance(0, 0) = 4.0;
	covariance(0, 1) = 1.0;
	covariance(1, 0) = 1.0;
	covariance(1, 1) = 3.0;

	assert(close_to(mtrc::numeric::det(covariance), 11.0));
	assert(mtrc::numeric::isPositiveDefinite(covariance));

	const Matrix inverse_covariance = mtrc::numeric::inv(covariance);
	assert(close_to(mtrc::numeric::det(inverse_covariance), 1.0 / 11.0));

	Matrix dense_three_by_three(3, 3);
	dense_three_by_three(0, 0) = 6.0;
	dense_three_by_three(0, 1) = 1.0;
	dense_three_by_three(0, 2) = 1.0;
	dense_three_by_three(1, 0) = 4.0;
	dense_three_by_three(1, 1) = -2.0;
	dense_three_by_three(1, 2) = 5.0;
	dense_three_by_three(2, 0) = 2.0;
	dense_three_by_three(2, 1) = 8.0;
	dense_three_by_three(2, 2) = 7.0;

	assert(close_to(mtrc::numeric::det(dense_three_by_three), -306.0));

	Matrix indefinite(2, 2);
	indefinite(0, 0) = 1.0;
	indefinite(0, 1) = 2.0;
	indefinite(1, 0) = 2.0;
	indefinite(1, 1) = 1.0;

	assert(close_to(mtrc::numeric::det(indefinite), -3.0));
	assert(!mtrc::numeric::isPositiveDefinite(indefinite));

	Matrix non_square(2, 3, 1.0);
	assert(!mtrc::numeric::isPositiveDefinite(non_square));
	assert_det_rejects_non_square();

	Vector x(2);
	x[0] = 1.0;
	x[1] = 2.0;

	Vector mu(2, 0.0);
	const auto centered = x - mu;
	const auto mahalanobis = mtrc::numeric::trans(centered) * inverse_covariance * centered;
	assert(close_to(mahalanobis, 15.0 / 11.0));

	constexpr double pi = 3.141592653589793238462643383279502884;
	const double density =
		std::exp(-mahalanobis / 2.0) / std::sqrt(mtrc::numeric::det(covariance) * std::pow(2.0 * pi, x.size()));
	const double expected_density = std::exp(-15.0 / 22.0) / (2.0 * pi * std::sqrt(11.0));

	assert(close_to(density, expected_density));

	return 0;
}
