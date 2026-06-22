#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include <metric/workflow.hpp>

int main()
{
	const std::vector<std::string> records = {
		"pump_ok",
		"pump_warn",
		"pump_fail",
		"valve_ok",
		"valve_warn",
		"valve_fail",
	};

	auto space = mtrc::space::build_checked(records, mtrc::Edit<char>{});

	const auto sample = mtrc::stats::sample::regular_sample(space, 3);
	assert(sample.size() == 3);
	assert(sample.algorithm == "regular_sample");

	const auto reduced = mtrc::modify::reduce::compress(space, 3);
	assert(reduced.size() == 3);
	assert(reduced.source_record_count == records.size());
	assert(reduced.assignments.size() == records.size());
	assert(reduced.metric_status == mtrc::metric_law::metric);

	std::cout << "sample size=" << sample.size() << "\n";
	std::cout << "reduced records=" << reduced.size() << "\n";
	return 0;
}
