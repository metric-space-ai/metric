#include <array>
#include <cassert>
#include <cmath>
#include <deque>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "metric/correlation/entropy.hpp"
#include "metric/metric/catalog.hpp"

void assert_close(const char *label, double actual, double expected, double tolerance = 1e-7)
{
	if (std::abs(actual - expected) >= tolerance) {
		std::cerr << std::setprecision(17) << label << " = " << actual << ", expected " << expected << "\n";
	}

	assert(std::abs(actual - expected) < tolerance);
}

double expected_edit_entropy()
{
	return -6.4090895367392831;
}

int main()
{
	using Chebyshev = mtrc::Chebyshev<double>;

	const std::vector<std::vector<double>> vector_records = {
		{5.0, 5.0},
		{2.0, 2.0},
		{3.0, 3.0},
		{5.0, 1.0},
	};
	const std::deque<std::array<double, 2>> array_records = {
		{5.0, 5.0},
		{2.0, 2.0},
		{3.0, 3.0},
		{5.0, 1.0},
	};

	const mtrc::Entropy<void, Chebyshev> tuned_entropy(Chebyshev(), 3, 2);
	assert_close("tuned vector entropy", tuned_entropy(vector_records), -4.4489104772539489);
	assert_close("tuned array entropy", tuned_entropy(array_records), -4.4489104772539489);

	const mtrc::Entropy<void, Chebyshev> default_entropy;
	assert_close("default vector entropy", default_entropy(vector_records), -5.3989104772539491);

	const std::vector<std::string> strings = {"AAA", "BBB", "ABA", "ABB", "BAA", "BAB"};

	const mtrc::Entropy<void, mtrc::Edit<char>> edit_entropy(mtrc::Edit<char>(), 3, 2);
	assert_close("edit entropy", edit_entropy(strings), expected_edit_entropy());

	const std::vector<std::string> ragged_strings = {
		"AAA", "HJGJHFG", "BBB", "AAAA", "long long long long long long string", "abcdefghjklmnopqrstuvwxyz",
	};
	assert(std::isnan(edit_entropy(ragged_strings)));

	return 0;
}
