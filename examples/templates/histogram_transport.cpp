#include <cassert>
#include <iostream>
#include <vector>

#include <metric/workflow.hpp>

int main()
{
	using histogram = std::vector<double>;

	const std::vector<histogram> distributions = {
		{0.70, 0.20, 0.10, 0.00},
		{0.68, 0.22, 0.10, 0.00},
		{0.00, 0.10, 0.25, 0.65},
		{0.00, 0.08, 0.28, 0.64},
	};

	auto transport = mtrc::Wasserstein<double>::on_line(4);
	auto space = mtrc::space::build_checked(distributions, transport);

	const histogram probe = {0.00, 0.09, 0.27, 0.64};
	const auto nearest = mtrc::space::query::nearest(space, probe);
	assert(nearest.id == space.id(3));

	const auto report = mtrc::metric::discover_metrics(mtrc::record_kind::structured);
	assert(report.has_discoverable_metric());

	const auto pairs = mtrc::space::distances::pairs(space);
	assert(pairs.size() == 6);

	std::cout << "nearest histogram id=" << nearest.id.index() << "\n";
	std::cout << "discoverable structured metrics=" << report.size() << "\n";
	return 0;
}
