#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct DoubleAbsoluteDistance {
	auto operator()(double lhs, double rhs) const -> double
	{
		const auto difference = lhs - rhs;
		return difference < 0.0 ? -difference : difference;
	}
};

int main()
{
	auto space = mtrc::make_space(std::vector<int>{0, 1, 10, 11}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);

	const auto clustered = mtrc::stats::structural_analysis::kmedoids(matrix, 2);
	static_assert(std::is_same<decltype(clustered)::distance_type, int>::value);
	assert(clustered.algorithm == "kmedoids");
	assert(clustered.representation == "pairwise_distances");
	assert(clustered.record_count == space.size());
	assert(clustered.cluster_count == 2);
	assert(clustered.converged);
	assert(clustered.iterations > 0);
	assert(clustered.assignments.size() == space.size());
	assert(clustered.cluster_sizes.size() == 2);
	assert(clustered.medoids.size() == 2);
	assert(clustered.medoids[0] == space.id(0));
	assert(clustered.medoids[1] == space.id(2));
	assert(clustered.assignments[0] == clustered.assignments[1]);
	assert(clustered.assignments[2] == clustered.assignments[3]);
	assert(clustered.assignments[0] != clustered.assignments[2]);
	assert(clustered.cluster_sizes[0] == 2);
	assert(clustered.cluster_sizes[1] == 2);

	const auto implicit = mtrc::stats::structural_analysis::kmedoids(space, 2);
	assert(implicit.representation == "metric_space");
	assert(implicit.medoids == clustered.medoids);
	assert(implicit.assignments == clustered.assignments);

	bool rejected_too_many_clusters = false;
	try {
		(void)mtrc::stats::structural_analysis::kmedoids(matrix, 5);
	} catch (const std::invalid_argument &) {
		rejected_too_many_clusters = true;
	}
	assert(rejected_too_many_clusters);

	auto continuous_space = mtrc::make_space(std::vector<double>{0.0, 0.1, 10.0, 10.1}, DoubleAbsoluteDistance{});
	const auto affinity_groups = mtrc::stats::structural_analysis::affinity_propagation(continuous_space, 0.7);
	static_assert(std::is_same<decltype(affinity_groups)::distance_type, double>::value);
	assert(affinity_groups.algorithm == "affinity_propagation");
	assert(affinity_groups.representation == "metric_space");
	assert(affinity_groups.record_count == continuous_space.size());
	assert(affinity_groups.assignments.size() == continuous_space.size());
	assert(affinity_groups.cluster_count == affinity_groups.medoids.size());
	assert(affinity_groups.cluster_count == affinity_groups.cluster_sizes.size());
	assert(!affinity_groups.empty());
	assert(affinity_groups.iterations > 0);

	mtrc::space::storage::DistanceTable<decltype(continuous_space)> continuous_matrix(continuous_space);
	const auto cached_affinity_groups = mtrc::stats::structural_analysis::affinity_propagation(continuous_matrix, 0.7);
	assert(cached_affinity_groups.algorithm == "affinity_propagation");
	assert(cached_affinity_groups.representation == "pairwise_distances");
	assert(cached_affinity_groups.record_count == continuous_space.size());
	assert(cached_affinity_groups.assignments == affinity_groups.assignments);
	assert(cached_affinity_groups.medoids == affinity_groups.medoids);
	assert(cached_affinity_groups.cluster_sizes == affinity_groups.cluster_sizes);

	bool rejected_invalid_preference = false;
	try {
		(void)mtrc::stats::structural_analysis::affinity_propagation(continuous_space, 1.0);
	} catch (const std::invalid_argument &) {
		rejected_invalid_preference = true;
	}
	assert(rejected_invalid_preference);

	return 0;
}
