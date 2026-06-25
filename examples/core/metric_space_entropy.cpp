#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <metric/correlation/entropy.hpp>
#include <metric/metric/catalog.hpp>

// This example exercises the low-level kpN entropy estimator directly for regression
// coverage. Framework-level entropy should be reached through an explicit coordinate
// mapping/embedding step and `mtrc::stats::properties::entropy(mapped_result)`.

namespace {

void assert_close(const double actual, const double expected) { assert(std::abs(actual - expected) < 1e-7); }

double expected_edit_entropy()
{
	return -6.4090895367392831;
}

} // namespace

int main()
{
	const std::vector<std::vector<double>> vector_records = {
		{5.0, 5.0},
		{2.0, 2.0},
		{3.0, 3.0},
		{5.0, 1.0},
	};

	const mtrc::Entropy<void, mtrc::Chebyshev<double>> vector_entropy(mtrc::Chebyshev<double>(), 3, 2);
	const double vector_result = vector_entropy(vector_records);
	assert_close(vector_result, -4.4489104772539489);

	const std::vector<std::string> string_records = {"AAA", "BBB", "ABA", "ABB", "BAA", "BAB"};

	const mtrc::Entropy<void, mtrc::Edit<char>> string_entropy(mtrc::Edit<char>(), 3, 2);
	const double string_result = string_entropy(string_records);
	assert_close(string_result, expected_edit_entropy());

	const std::vector<std::string> ragged_records = {
		"AAA", "HJGJHFG", "BBB", "AAAA", "long long long long long long string", "abcdefghjklmnopqrstuvwxyz",
	};
	assert(std::isnan(string_entropy(ragged_records)));

	std::cout << std::setprecision(12) << "vector entropy = " << vector_result << "\n"
			  << "string entropy = " << string_result << "\n";

	return 0;
}
