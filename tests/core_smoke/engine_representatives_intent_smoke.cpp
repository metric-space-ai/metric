#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

int main()
{
	const auto line = metric::make_space(std::vector<int>{0, 1, 2, 3, 4}, AbsoluteDistance{});

	const auto representatives = metric::find_representatives(line, 3);
	assert(representatives.record_count == 5);
	assert(representatives.requested_count == 3);
	assert(representatives.size() == 3);
	assert(representatives[0] == line.id(0));
	assert(representatives[1] == line.id(4));
	assert(representatives[2] == line.id(2));
	assert(representatives.coverage_radius == 1);
	assert(std::abs(representatives.average_nearest_distance - 0.4) < 1e-12);
	assert((representatives.nearest_representative_distances == std::vector<int>{0, 1, 0, 1, 0}));
	assert(representatives.exact);
	assert(representatives.operator_name == "find_representatives");
	assert(representatives.strategy == "farthest_first");
	assert(representatives.representation == "metric_space");

	const auto materialized_policy = metric::runtime::materialized(metric::runtime::exact());
	const auto materialized = metric::find_representatives(line, 3, materialized_policy);
	assert(materialized.representation == "matrix_cache");
	assert(materialized.representatives == representatives.representatives);
	assert(materialized.nearest_representative_distances == representatives.nearest_representative_distances);

	const auto materialized_seeded =
		metric::find_representatives(line, 2, metric::strategies::farthest_first(2), materialized_policy);
	assert(materialized_seeded.representation == "matrix_cache");
	assert(materialized_seeded[0] == line.id(2));
	assert(materialized_seeded[1] == line.id(0));

	const auto seeded = metric::find_representatives(line, 2, metric::strategies::farthest_first(2));
	assert(seeded[0] == line.id(2));
	assert(seeded[1] == line.id(0));

	const auto none = metric::find_representatives(line, 0);
	assert(none.empty());
	assert(none.record_count == line.size());

	bool rejected_empty = false;
	try {
		const auto empty = metric::make_space(std::vector<int>{}, AbsoluteDistance{});
		(void)metric::find_representatives(empty, 1);
	} catch (const std::invalid_argument &) {
		rejected_empty = true;
	}
	assert(rejected_empty);

	bool rejected_too_many = false;
	try {
		(void)metric::find_representatives(line, line.size() + 1);
	} catch (const std::invalid_argument &) {
		rejected_too_many = true;
	}
	assert(rejected_too_many);

	bool rejected_approximate_runtime = false;
	try {
		(void)metric::find_representatives(line, 2, metric::runtime::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	bool rejected_seed = false;
	try {
		(void)metric::find_representatives(line, 1, metric::strategies::farthest_first(line.size()));
	} catch (const std::out_of_range &) {
		rejected_seed = true;
	}
	assert(rejected_seed);

	return 0;
}
