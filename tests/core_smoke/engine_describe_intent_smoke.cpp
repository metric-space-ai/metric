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
	const auto description = metric::describe_structure(line);

	assert(description.record_count == 5);
	assert(description.pair_count == 10);
	assert(description.zero_distance_pair_count == 0);
	assert(description.has_nonzero_distances);
	assert(description.minimum_nonzero_distance == 1);
	assert(description.maximum_distance == 4);
	assert(std::abs(description.average_distance - 2.0) < 1e-12);
	assert(std::abs(description.intrinsic_dimension - (std::log(5.0 / 3.0) / std::log(2.0))) < 1e-12);
	assert(description.exact);
	assert(description.operator_name == "describe_structure");
	assert(description.strategy == "exact_all_pairs");
	assert(description.representation == "metric_space");

	const auto materialized_policy = metric::runtime::materialized(metric::runtime::exact());
	const auto materialized = metric::describe_structure(line, materialized_policy);
	assert(materialized.representation == "matrix_cache");
	assert(materialized.record_count == description.record_count);
	assert(materialized.pair_count == description.pair_count);
	assert(materialized.zero_distance_pair_count == description.zero_distance_pair_count);
	assert(materialized.minimum_nonzero_distance == description.minimum_nonzero_distance);
	assert(materialized.maximum_distance == description.maximum_distance);
	assert(std::abs(materialized.average_distance - description.average_distance) < 1e-12);
	assert(std::abs(materialized.intrinsic_dimension - description.intrinsic_dimension) < 1e-12);

	const auto duplicate = metric::make_space(std::vector<int>{0, 0, 1}, AbsoluteDistance{});
	const auto duplicate_description = metric::describe_structure(duplicate);
	assert(duplicate_description.record_count == 3);
	assert(duplicate_description.pair_count == 3);
	assert(duplicate_description.zero_distance_pair_count == 1);
	assert(duplicate_description.minimum_nonzero_distance == 1);
	assert(duplicate_description.maximum_distance == 1);

	bool rejected_approximate_runtime = false;
	try {
		(void)metric::describe_structure(line, metric::runtime::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	return 0;
}
