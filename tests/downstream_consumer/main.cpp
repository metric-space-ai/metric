#include <cassert>
#include <string>
#include <vector>

#include <metric/concepts.hpp>
#include <metric/metric/catalog.hpp>
#include <metric/space/index/operators.hpp>
#include <metric/space.hpp>

int main()
{
	std::vector<std::string> records = {"cat", "cot", "coat", "dog"};
	auto edit = mtrc::make_metric<std::string>(mtrc::Edit<std::string>{});
	auto space = mtrc::Space::from_records(records, edit);

	assert(space.size() == records.size());
	assert(space[0] == "cat");
	assert(space(0, 0) == 0);
	assert(space(0, 1) == 1);
	assert(space.distance(0, 2) == 1);

	const auto distances = space.pairwise_distances();
	assert(distances.size() == records.size());
	assert(distances[0][1] == 1);

	const auto nearest = space.neighbors(std::string("cut"), 2);
	assert(nearest.size() == 2);
	assert(nearest[0].second == 1);
	assert(space.nearest(std::string("cut")).second == 1);
	assert(space.within_radius(std::string("cut"), 1).size() == 2);

	const auto operator_nearest = mtrc::space::index::nearest_neighbors(records, edit, std::string("cut"), 2);
	assert(operator_nearest.size() == 2);
	assert(operator_nearest[0].second == 1);

	return 0;
}
