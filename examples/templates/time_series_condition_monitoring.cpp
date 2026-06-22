#include <cassert>
#include <iostream>
#include <vector>

#include <metric/workflow.hpp>

int main()
{
	using record_type = std::vector<double>;

	const std::vector<record_type> process_windows = {
		{0.00, 0.10, 0.18, 0.10, 0.00},
		{0.02, 0.12, 0.20, 0.11, 0.01},
		{0.50, 0.63, 0.77, 0.65, 0.52},
		{0.48, 0.60, 0.75, 0.64, 0.50},
	};

	const auto metric = mtrc::TWED<double>(0.05, 1.0);
	auto space = mtrc::space::build_checked(process_windows, metric);

	const record_type new_window = {0.47, 0.61, 0.74, 0.63, 0.51};
	const auto nearest = mtrc::space::query::nearest(space, new_window);
	assert(nearest.id == space.id(3));

	auto options = mtrc::stats::diagnose_options{};
	options.profile.include_distance_distribution = true;
	options.profile.distribution.bucket_count = 4;
	const auto diagnosis = mtrc::stats::diagnose_space(space, options);
	assert(diagnosis.discoverable_metric);
	assert(diagnosis.profile.record_count == process_windows.size());
	assert(diagnosis.has_neighbor_check);

	std::cout << "nearest process family id=" << nearest.id.index() << "\n";
	std::cout << "pair count=" << diagnosis.profile.pair_count << "\n";
	return 0;
}
