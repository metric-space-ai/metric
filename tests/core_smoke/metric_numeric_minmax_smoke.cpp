#include <cassert>
#include <string>

#include <metric/numeric/util/algorithms/Max.h>
#include <metric/numeric/util/algorithms/Min.h>

namespace {

struct OrderedValue {
	int value{};
};

auto operator<(const OrderedValue &lhs, const OrderedValue &rhs) -> bool { return lhs.value < rhs.value; }

} // namespace

int main()
{
	assert(mtrc::numeric::min(3, 1) == 1);
	assert(mtrc::numeric::max(3, 1) == 3);

	assert(mtrc::numeric::min(3.5, 1.25, 2.0) == 1.25);
	assert(mtrc::numeric::max(-5, -2, -9, -1) == -1);

	const std::string beta = "beta";
	const std::string alpha = "alpha";
	const std::string gamma = "gamma";
	assert(mtrc::numeric::min(beta, alpha, gamma) == "alpha");
	assert(mtrc::numeric::max(beta, alpha, gamma) == "gamma");

	const auto minimum = mtrc::numeric::min(OrderedValue{4}, OrderedValue{2}, OrderedValue{9}, OrderedValue{7});
	const auto maximum = mtrc::numeric::max(OrderedValue{4}, OrderedValue{2}, OrderedValue{9}, OrderedValue{7});
	assert(minimum.value == 2);
	assert(maximum.value == 9);

	return 0;
}
