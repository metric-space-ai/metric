// Smoke test for derived-space provenance across mtrc::modify operators:
// metric status, validity bounds, out-of-sample support, and source lineage.

#include <cassert>
#include <string>
#include <vector>

#include "metric/engine.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

} // namespace

int main()
{
	const auto line = mtrc::make_space(std::vector<int>{0, 1, 2, 3, 4, 5}, AbsoluteDistance{});
	const auto source_law = mtrc::core::metric_traits<AbsoluteDistance>::law;

	const auto mapped = mtrc::map(line, [](int value) { return value * 2; }, AbsoluteDistance{});
	assert(mapped.metric_status == mtrc::core::metric_traits<AbsoluteDistance>::law);
	assert(mapped.out_of_sample_supported);
	assert(!mapped.validity.empty());
	assert(mapped.source_records.size() == line.size());
	for (std::size_t index = 0; index < line.size(); ++index) {
		assert(mapped.source_records[index].size() == 1);
		assert(mapped.source_records[index][0] == line.id(index));
	}

	const auto compressed = mtrc::compress(line, 3);
	assert(compressed.metric_status == source_law);
	assert(compressed.validity.find("not dimension reduction") != std::string::npos);
	for (const auto id : compressed.source_record_ids) {
		assert(line.contains(id));
	}

	const auto rep = mtrc::represent(line, 3);
	assert(rep.metric_status == source_law);
	assert(!rep.out_of_sample_supported);
	assert(rep.validity.find("selected records only") != std::string::npos);

	const auto line_with_far_record = mtrc::make_space(std::vector<int>{0, 1, 2, 3, 4, 50}, AbsoluteDistance{});
	const auto density_filtered = mtrc::density_filter(line_with_far_record, 2.0, 2);
	assert(density_filtered.metric_status == source_law);
	assert(!density_filtered.out_of_sample_supported);
	assert(density_filtered.validity.find("density-based record filtering") != std::string::npos);
	for (const auto &row : density_filtered.source_records) {
		assert(row.size() == 1);
		assert(line_with_far_record.contains(row[0]));
	}

	return 0;
}
