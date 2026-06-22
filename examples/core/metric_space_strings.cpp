#include <iostream>
#include <string>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/space.hpp"

int main()
{
	std::vector<std::string> records = {"cat", "cot", "coat", "dog"};
	auto space = mtrc::Space::from_records(records, mtrc::Edit<std::string>{});

	std::cout << "distance(cat, cot) = " << space(0, 1) << "\n";

	const auto neighbors = space.neighbors(std::string("cut"), 2);
	for (const auto &neighbor : neighbors) {
		std::cout << records[neighbor.first] << ": " << neighbor.second << "\n";
	}

	return neighbors.size() == 2 && neighbors[0].second == 1 ? 0 : 1;
}
