#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <metric/correlation/entropy.hpp>
#include <metric/distance.hpp>

namespace {

void assert_close(const double actual, const double expected)
{
	assert(std::abs(actual - expected) < 1e-7);
}

double expected_edit_entropy()
{
#if defined(__APPLE__)
	return -9.2300109378544839;
#else
	return -9.3586207190266926;
#endif
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

	const metric::Entropy<void, metric::Chebyshev<double>> vector_entropy(metric::Chebyshev<double>(), 3, 2);
	const double vector_result = vector_entropy(vector_records);
	assert_close(vector_result, -4.4489104772539489);

	const std::vector<std::string> string_records = {
		"AAA",
		"HJGJHFG",
		"BBB",
		"AAAA",
		"long long long long long long string",
		"abcdefghjklmnopqrstuvwxyz",
	};

	const metric::Entropy<void, metric::Edit<int>> string_entropy(metric::Edit<int>(), 3, 2);
	const double string_result = string_entropy(string_records);
	assert_close(string_result, expected_edit_entropy());

	std::cout << std::setprecision(12)
			  << "vector entropy = " << vector_result << "\n"
			  << "string entropy = " << string_result << "\n";

	return 0;
}
