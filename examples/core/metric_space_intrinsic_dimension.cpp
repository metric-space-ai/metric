#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include <metric/space/index/operators.hpp>

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

int main()
{
	const std::vector<int> records = {0, 1, 2, 3, 4};
	const double dimension = mtrc::stats::properties::intrinsic_dimension(records, AbsoluteDistance{});
	const double expected = std::log(5.0 / 3.0) / std::log(2.0);

	std::cout << "expansion dimension estimate = " << dimension << "\n";
	assert(std::abs(dimension - expected) < 1e-12);

	return 0;
}
