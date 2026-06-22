#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

int main()
{
	const auto line = mtrc::make_space(std::vector<int>{0, 1, 2, 3, 4}, AbsoluteDistance{});

	const auto built_representatives = mtrc::core::make_representative_set(
		std::vector<mtrc::RecordId>{line.id(0), line.id(4)}, std::vector<int>{0, 3, 1}, line.size(), 2,
		"custom_representatives", "custom_provider", false);
	assert(built_representatives.record_count == line.size());
	assert(built_representatives.requested_count == 2);
	assert(built_representatives.size() == 2);
	assert(built_representatives[0] == line.id(0));
	assert(built_representatives[1] == line.id(4));
	assert((built_representatives.nearest_representative_distances == std::vector<int>{0, 3, 1}));
	assert(built_representatives.coverage_radius == 3);
	assert(std::abs(built_representatives.average_nearest_distance - (4.0 / 3.0)) < 1e-12);
	assert(!built_representatives.exact);
	assert(built_representatives.operator_name == "find_representatives");
	assert(built_representatives.strategy == "custom_representatives");
	assert(built_representatives.representation == "custom_provider");

	const auto empty_representatives = mtrc::core::make_representative_set(
		std::vector<mtrc::RecordId>{}, std::vector<int>{}, line.size(), 0, "empty_strategy", "empty_provider");
	assert(empty_representatives.empty());
	assert(empty_representatives.record_count == line.size());
	assert(empty_representatives.requested_count == 0);
	assert(empty_representatives.coverage_radius == 0);
	assert(empty_representatives.average_nearest_distance == 0.0);
	assert(empty_representatives.exact);
	assert(empty_representatives.strategy == "empty_strategy");
	assert(empty_representatives.representation == "empty_provider");

	const auto representatives = mtrc::find_representatives(line, 3);
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

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto materialized = mtrc::find_representatives(line, 3, materialized_policy);
	assert(materialized.representation == "distance_table");
	assert(materialized.representatives == representatives.representatives);
	assert(materialized.nearest_representative_distances == representatives.nearest_representative_distances);

	const auto materialized_seeded =
		mtrc::find_representatives(line, 2, mtrc::space::select::farthest_first(2), materialized_policy);
	assert(materialized_seeded.representation == "distance_table");
	assert(materialized_seeded[0] == line.id(2));
	assert(materialized_seeded[1] == line.id(0));

	const auto seeded = mtrc::find_representatives(line, 2, mtrc::space::select::farthest_first(2));
	assert(seeded[0] == line.id(2));
	assert(seeded[1] == line.id(0));

	const auto none = mtrc::find_representatives(line, 0);
	assert(none.empty());
	assert(none.record_count == line.size());

	bool rejected_empty = false;
	try {
		const auto empty = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
		(void)mtrc::find_representatives(empty, 1);
	} catch (const std::invalid_argument &) {
		rejected_empty = true;
	}
	assert(rejected_empty);

	bool rejected_too_many = false;
	try {
		(void)mtrc::find_representatives(line, line.size() + 1);
	} catch (const std::invalid_argument &) {
		rejected_too_many = true;
	}
	assert(rejected_too_many);

	bool rejected_approximate_runtime = false;
	try {
		(void)mtrc::find_representatives(line, 2, mtrc::space::storage::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	bool rejected_seed = false;
	try {
		(void)mtrc::find_representatives(line, 1, mtrc::space::select::farthest_first(line.size()));
	} catch (const std::out_of_range &) {
		rejected_seed = true;
	}
	assert(rejected_seed);

	return 0;
}
