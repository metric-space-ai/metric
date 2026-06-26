#include <cassert>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/engine.hpp"

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
	assert(mtrc::summary(representatives).find("RepresentativeSet") != std::string::npos);

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

	const auto approximate = mtrc::find_representatives(line, 2, mtrc::space::storage::approximate());
	assert(!approximate.exact);
	assert(approximate.representation == "sampled_metric_space");
	assert(approximate.strategy == "sampled_farthest_first");
	assert(approximate.size() == 2);

	const auto approximate_coverage =
		mtrc::find_representatives(line, 2, mtrc::space::select::coverage{}, mtrc::space::storage::approximate());
	assert(!approximate_coverage.exact);
	assert(approximate_coverage.representation == "sampled_metric_space");
	assert(approximate_coverage.strategy == "sampled_coverage");

	const auto approximate_radius = mtrc::find_representatives(
		line, mtrc::space::select::radius_coverage{1}, mtrc::space::storage::approximate());
	assert(!approximate_radius.exact);
	assert(approximate_radius.representation == "sampled_metric_space");
	assert(approximate_radius.strategy == "sampled_radius_coverage");

	const auto chunked_records = line_records(48);
	auto chunked_calls = std::make_shared<std::size_t>(0);
	const auto chunked_space = mtrc::make_space(chunked_records, CountingAbsoluteDistance{chunked_calls});
	auto chunked_policy = mtrc::space::storage::using_distance_table();
	chunked_policy = mtrc::space::storage::with_distance_table_budget(chunked_policy, 8, 0);
	chunked_policy = mtrc::space::storage::allow_approximate_fallback(chunked_policy);
	chunked_policy = mtrc::space::storage::allow_chunking_fallback(chunked_policy);
	const auto chunked_plan = mtrc::space::storage::estimate_cost(chunked_space, "representatives", chunked_policy);
	assert(chunked_plan.allowed);
	assert(chunked_plan.downgraded);
	assert(!chunked_plan.exact);
	assert(chunked_plan.representation == "chunked_space_view");
	const auto chunked_representatives = mtrc::find_representatives(chunked_space, 4, chunked_policy);
	assert(!chunked_representatives.exact);
	assert(chunked_representatives.representation == "chunked_space_view");
	assert(chunked_representatives.strategy == "chunked_farthest_first");
	assert(chunked_representatives.size() == 4);
	assert(*chunked_calls < chunked_space.size() * (chunked_space.size() - 1) / 2);

	const auto sampled_records = line_records(80);
	auto sampled_calls = std::make_shared<std::size_t>(0);
	const auto sampled_space = mtrc::make_space(sampled_records, CountingAbsoluteDistance{sampled_calls});
	auto sampled_policy = mtrc::space::storage::using_knn_graph(16);
	sampled_policy = mtrc::space::storage::with_distance_table_budget(sampled_policy, 8, 0);
	sampled_policy = mtrc::space::storage::allow_approximate_fallback(sampled_policy);
	const auto sampled_plan = mtrc::space::storage::estimate_cost(
		sampled_space, "representatives", mtrc::space::storage::using_distance_table(sampled_policy));
	assert(sampled_plan.allowed);
	assert(sampled_plan.downgraded);
	assert(!sampled_plan.exact);
	assert(sampled_plan.representation == "sampled_metric_space");
	const auto sampled_representatives =
		mtrc::find_representatives(sampled_space, 4, mtrc::space::select::k_center{}, sampled_policy);
	assert(!sampled_representatives.exact);
	assert(sampled_representatives.representation == "sampled_metric_space");
	assert(sampled_representatives.strategy == "sampled_k_center");
	assert(sampled_representatives.size() == 4);
	assert(*sampled_calls < sampled_space.size() * (sampled_space.size() - 1) / 2);

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

	bool rejected_seed = false;
	try {
		(void)mtrc::find_representatives(line, 1, mtrc::space::select::farthest_first(line.size()));
	} catch (const std::out_of_range &) {
		rejected_seed = true;
	}
	assert(rejected_seed);

	return 0;
}
