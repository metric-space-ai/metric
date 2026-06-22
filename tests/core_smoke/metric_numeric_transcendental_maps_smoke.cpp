#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;
	using mtrc::test::close_to;

	Matrix logits(2, 3);
	logits(0, 0) = -1.0;
	logits(0, 1) = 0.0;
	logits(0, 2) = 1.0;
	logits(1, 0) = 2.0;
	logits(1, 1) = -2.0;
	logits(1, 2) = 0.5;

	const Matrix exponentials = mtrc::numeric::exp(logits);
	assert(close_to(exponentials(0, 0), std::exp(-1.0)));
	assert(close_to(exponentials(0, 1), 1.0));
	assert(close_to(exponentials(0, 2), std::exp(1.0)));
	assert(close_to(exponentials(1, 0), std::exp(2.0)));

	const Matrix sigmoid = 1.0 / (1.0 + mtrc::numeric::exp(-logits));
	assert(close_to(sigmoid(0, 0), 1.0 / (1.0 + std::exp(1.0))));
	assert(close_to(sigmoid(0, 1), 0.5));
	assert(close_to(sigmoid(1, 0), 1.0 / (1.0 + std::exp(-2.0))));

	Matrix angles(2, 2);
	angles(0, 0) = 0.0;
	angles(0, 1) = 1.0;
	angles(1, 0) = -1.0;
	angles(1, 1) = std::sqrt(3.0);

	const Matrix arctangent = mtrc::numeric::atan(angles);
	assert(close_to(arctangent(0, 0), 0.0));
	assert(close_to(arctangent(0, 1), std::atan(1.0)));
	assert(close_to(arctangent(1, 0), std::atan(-1.0)));
	assert(close_to(arctangent(1, 1), std::atan(std::sqrt(3.0))));

	Vector eigenvalues(4);
	eigenvalues[0] = 1.0;
	eigenvalues[1] = std::exp(1.0);
	eigenvalues[2] = std::exp(2.0);
	eigenvalues[3] = std::exp(-3.0);

	const Vector logarithms = mtrc::numeric::log(eigenvalues);
	assert(close_to(logarithms[0], 0.0));
	assert(close_to(logarithms[1], 1.0));
	assert(close_to(logarithms[2], 2.0));
	assert(close_to(logarithms[3], -3.0));
	assert(close_to(std::sqrt(mtrc::numeric::sum(mtrc::numeric::pow(logarithms, 2.0))), std::sqrt(14.0)));

	const Vector activations = mtrc::numeric::tanh(logarithms);
	assert(close_to(activations[0], 0.0));
	assert(close_to(activations[1], std::tanh(1.0)));
	assert(close_to(activations[2], std::tanh(2.0)));
	assert(close_to(activations[3], std::tanh(-3.0)));

	Matrix fractional(2, 3);
	fractional(0, 0) = -1.75;
	fractional(0, 1) = -0.25;
	fractional(0, 2) = 0.25;
	fractional(1, 0) = 1.25;
	fractional(1, 1) = 1.5;
	fractional(1, 2) = 2.75;

	const Matrix rounded = mtrc::numeric::round(fractional);
	assert(close_to(rounded(0, 0), -2.0));
	assert(close_to(rounded(0, 1), 0.0));
	assert(close_to(rounded(1, 1), 2.0));
	assert(close_to(rounded(1, 2), 3.0));

	const Matrix floored = mtrc::numeric::floor(fractional);
	assert(close_to(floored(0, 0), -2.0));
	assert(close_to(floored(0, 1), -1.0));
	assert(close_to(floored(1, 2), 2.0));

	const Matrix ceiled = mtrc::numeric::ceil(fractional);
	assert(close_to(ceiled(0, 0), -1.0));
	assert(close_to(ceiled(0, 1), 0.0));
	assert(close_to(ceiled(1, 2), 3.0));

	const Matrix truncated = mtrc::numeric::trunc(fractional);
	assert(close_to(truncated(0, 0), -1.0));
	assert(close_to(truncated(0, 1), 0.0));
	assert(close_to(truncated(1, 2), 2.0));

	const Matrix signs = mtrc::numeric::sign(fractional);
	assert(close_to(signs(0, 0), -1.0));
	assert(close_to(signs(0, 1), -1.0));
	assert(close_to(signs(0, 2), 1.0));
	assert(close_to(signs(1, 2), 1.0));

	return 0;
}
