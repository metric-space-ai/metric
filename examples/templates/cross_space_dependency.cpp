#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include <metric/workflow.hpp>

int main()
{
	using curve = std::vector<double>;
	using quality = std::vector<double>;

	const std::vector<curve> process_windows = {
		{0.0, 0.1, 0.2, 0.1},
		{0.1, 0.2, 0.4, 0.2},
		{0.4, 0.6, 0.8, 0.6},
		{0.6, 0.8, 1.0, 0.8},
		{0.8, 1.0, 1.2, 1.0},
	};
	const std::vector<quality> condition_indices = {
		{0.0},
		{0.2},
		{0.6},
		{0.8},
		{1.0},
	};
	const std::vector<quality> shuffled_condition_indices = {
		{0.6},
		{0.0},
		{1.0},
		{0.2},
		{0.8},
	};

	auto process_space = mtrc::space::build_checked(process_windows, mtrc::TWED<double>(0.02, 1.0));
	auto condition_space = mtrc::space::build_checked(condition_indices, mtrc::Euclidean<double>{});
	auto shuffled_space = mtrc::space::build_checked(shuffled_condition_indices, mtrc::Euclidean<double>{});

	const auto dependent = mtrc::compare(process_space, condition_space, mtrc::stats::correlate::mgc_options{});
	const auto shuffled = mtrc::compare(process_space, shuffled_space, mtrc::stats::correlate::mgc_options{});
	assert(dependent.algorithm == "mgc");
	assert(std::isfinite(dependent.value));
	assert(std::isfinite(shuffled.value));
	assert(dependent.value > shuffled.value);

	std::cout << "dependent MGC=" << dependent.value << "\n";
	std::cout << "shuffled MGC=" << shuffled.value << "\n";
	return 0;
}
