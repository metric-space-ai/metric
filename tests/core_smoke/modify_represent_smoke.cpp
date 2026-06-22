// Smoke test for mtrc::modify::represent: lossless representative subspace
// construction, and its separation from lossy mtrc::modify::reduce::compress.

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
	const auto line = mtrc::make_space(std::vector<int>{0, 1, 2, 3, 4}, AbsoluteDistance{});

	const auto rep = mtrc::represent(line, 3);
	assert(rep.size() == 3);
	assert(rep.source_record_count == 5);
	assert(rep.mapping == "represent");
	assert(rep.strategy == "farthest_first");

	assert(rep.source_records.size() == 3);
	assert(rep.representative_records.size() == 3);
	for (std::size_t index = 0; index < rep.source_records.size(); ++index) {
		assert(rep.source_records[index].size() == 1);
		assert(rep.source_records[index][0] == rep.representative_records[index]);
		assert(line.contains(rep.representative_records[index]));
	}

	assert(rep.metric_status == mtrc::core::metric_traits<AbsoluteDistance>::law);
	assert(!rep.out_of_sample_supported);
	assert(rep.validity.find("representative subspace") != std::string::npos);

	assert(rep.space.record(rep.space.id(0)) == 0);
	assert(rep.space.record(rep.space.id(1)) == 4);
	assert(rep.space.record(rep.space.id(2)) == 2);

	const auto comp = mtrc::compress(line, 3);
	assert((comp.source_record_ids == rep.representative_records));
	assert(comp.assignments.size() == 5);
	assert(comp.lossy);
	assert(rep.source_records.size() == 3);
	assert(comp.metric_status == rep.metric_status);
	assert(comp.validity.find("record-set cardinality reduction") != std::string::npos);

	const auto exact = mtrc::represent(line, 2, mtrc::space::storage::exact());
	assert(exact.size() == 2);

	bool rejected_empty_count = false;
	try {
		(void)mtrc::represent(line, 0);
	} catch (const std::invalid_argument &) {
		rejected_empty_count = true;
	}
	assert(rejected_empty_count);

	return 0;
}
