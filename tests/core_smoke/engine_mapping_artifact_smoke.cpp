#include <cassert>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct CountingAbsoluteDistance {
	int *calls{};

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
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

	int counted_metric_calls = 0;
	auto counted_space =
		mtrc::make_space(std::vector<int>{0, 1, 2, 3, 4}, CountingAbsoluteDistance{&counted_metric_calls});
	const auto singleton_groups = mtrc::core::make_clustering_result<int>(
		std::vector<std::size_t>{0, 1, 2, 3, 4},
		std::vector<mtrc::RecordId>{counted_space.id(0), counted_space.id(1), counted_space.id(2),
									counted_space.id(3), counted_space.id(4)},
		std::vector<mtrc::RecordId>{}, std::vector<mtrc::RecordId>{}, std::vector<std::size_t>{1, 1, 1, 1, 1},
		0, true, "manual_singletons", "metric_space");
	auto low_distance_budget = mtrc::modify::map::ClusteredSpaceOptions{};
	low_distance_budget.max_representatives = 0;
	low_distance_budget.max_memory_bytes = 0;
	low_distance_budget.max_distance_evaluations = counted_space.size() * counted_space.size() - 1;
	bool rejected_large_representative_table = false;
	try {
		const auto guarded_mapping =
			mtrc::modify::map::make_clustered_space_mapping(singleton_groups, low_distance_budget);
		(void)mtrc::modify::map::derive_from(guarded_mapping, counted_space);
	} catch (const mtrc::RepresentationError &error) {
		rejected_large_representative_table = true;
		const std::string message = error.what();
		assert(message.find("clustered_space") != std::string::npos);
		assert(message.find("max_distance_evaluations") != std::string::npos);
	}
	assert(rejected_large_representative_table);
	assert(counted_metric_calls == 0);

	auto explicit_unbounded_budget = mtrc::modify::map::ClusteredSpaceOptions{};
	explicit_unbounded_budget.max_representatives = 0;
	explicit_unbounded_budget.max_memory_bytes = 0;
	explicit_unbounded_budget.max_distance_evaluations = 0;
	const auto singleton_space =
		mtrc::modify::map::clustered_space(counted_space, singleton_groups, explicit_unbounded_budget);
	assert(singleton_space.space.size() == counted_space.size());
	assert(counted_metric_calls == static_cast<int>(counted_space.size() * counted_space.size()));

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
