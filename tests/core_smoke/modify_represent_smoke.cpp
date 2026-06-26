// Smoke test for mtrc::modify::represent: lossless representative subspace
// construction, and its separation from lossy mtrc::modify::reduce::compress.

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "metric/engine.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct CountingAbsoluteDistance {
	std::shared_ptr<std::size_t> calls;

	auto operator()(int lhs, int rhs) const -> int
	{
		++*calls;
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

auto line_records(std::size_t count) -> std::vector<int>
{
	std::vector<int> records;
	records.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		records.push_back(static_cast<int>(index));
	}
	return records;
}

} // namespace

int main()
{
	const auto line = mtrc::make_space(std::vector<int>{0, 1, 2, 3, 4}, AbsoluteDistance{});

	const auto rep = mtrc::represent(line, 3);
	assert(rep.size() == 3);
	assert(rep.source_record_count == 5);
	assert(rep.mapping == "represent");
	assert(rep.strategy == "farthest_first");
	assert(mtrc::summary(rep).find("MappingResult") != std::string::npos);

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
	assert(mtrc::summary(comp).find("CompressionResult") != std::string::npos);

	const auto exact = mtrc::represent(line, 2, mtrc::space::storage::exact());
	assert(exact.size() == 2);

	const auto large_records = line_records(4097);
	auto large_calls = std::make_shared<std::size_t>(0);
	const auto large_line = mtrc::make_space(large_records, CountingAbsoluteDistance{large_calls});
	const auto large_represented = mtrc::represent(large_line, 4);
	assert(large_represented.mapping == "represent");
	assert(large_represented.strategy == "sampled_farthest_first");
	assert(large_represented.representation == "sampled_metric_space");
	assert(large_represented.size() == 4);
	assert(large_represented.source_record_count == large_line.size());
	assert(*large_calls < large_line.size() * (large_line.size() - 1) / 2);

	bool rejected_empty_count = false;
	try {
		(void)mtrc::represent(line, 0);
	} catch (const std::invalid_argument &) {
		rejected_empty_count = true;
	}
	assert(rejected_empty_count);

	return 0;
}
