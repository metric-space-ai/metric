#include <cassert>
#include <cmath>

#include <metric/numeric.hpp>

#include "numeric_test_helpers.hpp"

int main()
{
	using mtrc::test::close_to;

	const auto original_seed = mtrc::numeric::getSeed();

	mtrc::numeric::setSeed(123456789U);
	assert(mtrc::numeric::getSeed() == 123456789U);

	const int first_integer = mtrc::numeric::rand<int>(-3, 7);
	const double first_double = mtrc::numeric::rand<double>(-1.0, 1.0);
	const auto first_complex = mtrc::numeric::rand<mtrc::numeric::complex<double>>(-2.0, 2.0);

	assert(first_integer >= -3);
	assert(first_integer <= 7);
	assert(first_double >= -1.0);
	assert(first_double <= 1.0);
	assert(mtrc::numeric::real(first_complex) >= -2.0);
	assert(mtrc::numeric::real(first_complex) <= 2.0);
	assert(mtrc::numeric::imag(first_complex) >= -2.0);
	assert(mtrc::numeric::imag(first_complex) <= 2.0);

	mtrc::numeric::setSeed(123456789U);
	assert(mtrc::numeric::rand<int>(-3, 7) == first_integer);
	assert(close_to(mtrc::numeric::rand<double>(-1.0, 1.0), first_double));
	const auto repeated_complex = mtrc::numeric::rand<mtrc::numeric::complex<double>>(-2.0, 2.0);
	assert(close_to(mtrc::numeric::real(repeated_complex), mtrc::numeric::real(first_complex)));
	assert(close_to(mtrc::numeric::imag(repeated_complex), mtrc::numeric::imag(first_complex)));

	double randomized_value = 0.0;
	mtrc::numeric::setSeed(987654321U);
	mtrc::numeric::randomize(randomized_value, 2.0, 3.0);
	assert(randomized_value >= 2.0);
	assert(randomized_value <= 3.0);

	mtrc::numeric::setSeed(original_seed);
	assert(mtrc::numeric::getSeed() == original_seed);

	return 0;
}
