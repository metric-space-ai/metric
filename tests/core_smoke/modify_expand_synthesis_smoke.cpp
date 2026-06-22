// Smoke test for mtrc::modify::expand midpoint synthesis: interpolation-like
// record generation for arithmetic vector records, with a domain guard that
// rejects record domains lacking an interpolation contract.

#include <cassert>
#include <string>
#include <vector>

#include "metric/engine.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct StringLengthDistance {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> std::size_t
	{
		return lhs.size() > rhs.size() ? lhs.size() - rhs.size() : rhs.size() - lhs.size();
	}
};

} // namespace

int main()
{
	std::vector<std::vector<double>> records = {{0.0, 0.0}, {2.0, 0.0}, {0.0, 2.0}};
	const auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});

	const auto expanded = mtrc::modify::expand::expand(space);
	assert(expanded.mapping == "expand");
	assert(expanded.strategy == "nearest_neighbor_midpoint");

	assert(expanded.size() >= space.size());
	for (std::size_t index = 0; index < space.size(); ++index) {
		assert(expanded.source_records[index].size() == 1);
		assert(expanded.source_records[index][0] == space.id(index));
		assert(expanded.space.record(expanded.space.id(index)) == records[index]);
	}

	bool found_synthetic = false;
	for (std::size_t index = space.size(); index < expanded.source_records.size(); ++index) {
		assert(expanded.source_records[index].size() == 2);
		assert(space.contains(expanded.source_records[index][0]));
		assert(space.contains(expanded.source_records[index][1]));
		found_synthetic = true;
	}
	assert(found_synthetic);

	assert(expanded.metric_status == mtrc::core::metric_traits<mtrc::Euclidean<double>>::law);
	assert(!expanded.out_of_sample_supported);
	assert(expanded.validity.find("interpolation-like expansion") != std::string::npos);

	bool found_midpoint = false;
	for (std::size_t index = space.size(); index < expanded.space.size(); ++index) {
		const auto &record = expanded.space.record(expanded.space.id(index));
		if (record.size() == 2 && record[0] == 1.0 && record[1] == 0.0) {
			found_midpoint = true;
		}
	}
	assert(found_midpoint);

	const auto line = mtrc::make_space(std::vector<int>{0, 1, 2}, AbsoluteDistance{});
	bool rejected = false;
	try {
		(void)mtrc::modify::expand::expand(line);
	} catch (const mtrc::StrategyUnavailableError &) {
		rejected = true;
	}
	assert(rejected);

	const auto words = mtrc::make_space(std::vector<std::string>{"a", "bb", "ccc"}, StringLengthDistance{});
	bool rejected_strings = false;
	try {
		(void)mtrc::modify::expand::expand(words);
	} catch (const mtrc::StrategyUnavailableError &) {
		rejected_strings = true;
	}
	assert(rejected_strings);

	return 0;
}
