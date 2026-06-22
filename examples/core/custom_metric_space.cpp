#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include <metric/concepts.hpp>
#include <metric/space.hpp>

struct PaddedHamming {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> std::size_t
	{
		const auto limit = std::max(lhs.size(), rhs.size());
		std::size_t distance = 0;

		for (std::size_t i = 0; i < limit; ++i) {
			const char left = i < lhs.size() ? lhs[i] : '\0';
			const char right = i < rhs.size() ? rhs[i] : '\0';
			if (left != right) {
				++distance;
			}
		}

		return distance;
	}
};

int main()
{
	std::vector<std::string> records = {"red", "reed", "road", "blue"};
	auto distance = mtrc::make_metric<std::string>(PaddedHamming{});
	auto space = mtrc::Space::from_records(records, distance);

	assert(space.size() == records.size());
	assert(space(0, 0) == 0);
	assert(space(0, 1) == 2);

	const auto nearest = space.neighbors(std::string("read"), 2);
	assert(nearest.size() == 2);
	assert(nearest[0].second == 1);

	std::cout << records[nearest[0].first] << ": " << nearest[0].second << "\n";
	return 0;
}
