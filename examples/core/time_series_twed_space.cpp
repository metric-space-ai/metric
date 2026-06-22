#include <cassert>
#include <iostream>
#include <vector>

#include <metric/metric/catalog.hpp>
#include <metric/space.hpp>

int main()
{
	const std::vector<double> baseline = {0, 1, 1, 1, 1, 1, 2, 3};
	const std::vector<double> shifted = {1, 1, 1, 1, 1, 2, 3, 4};
	const std::vector<double> flat = {2, 2, 2, 2, 2, 2, 2, 2};

	const mtrc::TWED<double> distance(0, 1);
	assert(distance(baseline, shifted) == 7.0);
	assert(distance(baseline, flat) == 15.0);

	const std::vector<std::vector<double>> records = {baseline, shifted, flat};
	mtrc::MatrixSpace<std::vector<double>, mtrc::TWED<double>> space(records, distance);

	assert(space.size() == records.size());
	assert(space(0, 1) == 7.0);
	assert(space(0, 2) == 15.0);

	const auto neighbors = space.knn(baseline, 2);
	assert(neighbors.size() == 2);
	assert(neighbors[0].first == 0);
	assert(neighbors[0].second == 0.0);
	assert(neighbors[1].first == 1);
	assert(neighbors[1].second == 7.0);

	std::cout << "TWED(baseline, shifted) = " << space(0, 1) << "\n";
	std::cout << "TWED(baseline, flat) = " << space(0, 2) << "\n";

	return 0;
}
