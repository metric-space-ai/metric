#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

int main()
{
	auto space = metric::make_space(std::vector<int>{0, 1, 10, 11, 30}, AbsoluteDistance{});
	const auto groups = metric::operators::kmedoids(space, 2);

	auto mapping = metric::mappings::make_clustered_space_mapping(groups);
	static_assert(metric::Mapping_v<decltype(mapping), decltype(space)>);
	auto model = metric::mappings::fit(mapping, space);
	static_assert(metric::MappingModel_v<decltype(model), decltype(space)>);

	const auto reduced = metric::mappings::transform(model, space);
	using reduced_type = typename std::decay<decltype(reduced)>::type;
	static_assert(std::is_same<typename reduced_type::space_type::distance_type, int>::value);
	assert(reduced.mapping == "clustered_space");
	assert(reduced.strategy == "kmedoids");
	assert(reduced.representation == "metric_space");
	assert(!reduced.inverse_supported);
	assert(reduced.source_record_count == space.size());
	assert(reduced.space.size() == 2);
	assert(reduced.source_records.size() == 2);
	assert(reduced.representative_records == groups.medoids);

	const auto first_cluster = reduced.space.id(0);
	const auto second_cluster = reduced.space.id(1);
	assert(reduced.space.distance(first_cluster, first_cluster) == 0);
	assert(reduced.space.distance(first_cluster, second_cluster) ==
		   space.distance(groups.medoids[0], groups.medoids[1]));
	assert(reduced.space.record(first_cluster).members == reduced.source_records[0]);
	assert(reduced.space.record(second_cluster).representative == groups.medoids[1]);

	const auto direct = metric::mappings::clustered_space(space, groups);
	assert(direct.space.size() == reduced.space.size());
	assert(direct.source_records == reduced.source_records);
	assert(direct.space.distance(direct.space.id(0), direct.space.id(1)) ==
		   reduced.space.distance(first_cluster, second_cluster));

	const auto density_groups = metric::operators::dbscan(space, 2, 2);
	const auto density_space = metric::mappings::clustered_space(space, density_groups);
	assert(density_space.space.size() == 2);
	assert(density_space.source_record_count == space.size());
	assert(density_space.source_records[0].size() == 2);
	assert(density_space.source_records[1].size() == 2);

	auto invalid_groups = groups;
	invalid_groups.assignments.pop_back();
	bool rejected_invalid_clustering = false;
	try {
		(void)metric::mappings::clustered_space(space, invalid_groups);
	} catch (const std::invalid_argument &) {
		rejected_invalid_clustering = true;
	}
	assert(rejected_invalid_clustering);

	return 0;
}
