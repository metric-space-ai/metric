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
	auto space = mtrc::make_space(std::vector<int>{0, 1, 10, 11, 30}, AbsoluteDistance{});
	const auto groups = mtrc::stats::structural_analysis::kmedoids(space, 2);

	auto mapping = mtrc::modify::map::make_clustered_space_mapping(groups);
	static_assert(mtrc::Mapping_v<decltype(mapping), decltype(space)>);
	auto mapping_artifact = mtrc::modify::map::derive_from(mapping, space);
	static_assert(mtrc::DerivedSpaceTransform_v<decltype(mapping_artifact), decltype(space)>);

	const auto reduced = mtrc::modify::map::transform(mapping_artifact, space);
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

	const auto direct = mtrc::modify::map::clustered_space(space, groups);
	assert(direct.space.size() == reduced.space.size());
	assert(direct.source_records == reduced.source_records);
	assert(direct.space.distance(direct.space.id(0), direct.space.id(1)) ==
		   reduced.space.distance(first_cluster, second_cluster));

	const auto density_groups = mtrc::stats::structural_analysis::dbscan(space, 2, 2);
	const auto density_space = mtrc::modify::map::clustered_space(space, density_groups);
	assert(density_space.space.size() == 2);
	assert(density_space.source_record_count == space.size());
	assert(density_space.source_records[0].size() == 2);
	assert(density_space.source_records[1].size() == 2);

	auto invalid_groups = groups;
	invalid_groups.assignments.pop_back();
	bool rejected_invalid_clustering = false;
	try {
		(void)mtrc::modify::map::clustered_space(space, invalid_groups);
	} catch (const std::invalid_argument &) {
		rejected_invalid_clustering = true;
	}
	assert(rejected_invalid_clustering);

	return 0;
}
