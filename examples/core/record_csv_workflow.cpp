// Load numeric records from a real CSV file, build a finite metric space, query
// it, and export the records again through the native record layer.

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <metric/workflow.hpp>

#ifndef METRIC_CORE_VECTOR_RECORDS_CSV
#define METRIC_CORE_VECTOR_RECORDS_CSV "assets/vector_records.csv"
#endif

int main()
{
	mtrc::CsvReadOptions read_options;
	read_options.has_header = true;

	const auto records = mtrc::record::read_csv<double>(METRIC_CORE_VECTOR_RECORDS_CSV, read_options);
	assert(records.size() == 4);
	assert(records.front().size() == 3);

	auto space = mtrc::space::build_checked(records, mtrc::Euclidean<double>{});
	const auto nearest = mtrc::space::query::k_nearest(space, space.id(0), 2);
	assert(nearest.size() == 2);
	assert(nearest[0].distance == 1.0);
	assert(nearest[1].distance == 1.0);

	mtrc::CsvWriteOptions write_options;
	write_options.header = {"x", "y", "z"};
	std::ostringstream exported;
	mtrc::record::write_csv(exported, records, write_options);
	assert(exported.str().find("0,0,0") != std::string::npos);

	std::cout << "loaded " << records.size() << " records; nearest distances from anchor are "
			  << nearest[0].distance << " and " << nearest[1].distance << "\n";
	return 0;
}
