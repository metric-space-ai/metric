// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		const auto difference = lhs - rhs;
		return difference < 0 ? -difference : difference;
	}
};

int main()
{
	auto space = mtrc::make_space(std::vector<int>{0, 1, 10, 11, 30}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);

	using ManualResult = mtrc::ClusteringResult<int>;
	const auto manual = mtrc::core::make_clustering_result<int>(
		std::vector<std::size_t>{0, 1, ManualResult::noise_label},
		std::vector<mtrc::RecordId>{space.id(0), space.id(1)}, std::vector<mtrc::RecordId>{space.id(0)},
		std::vector<mtrc::RecordId>{space.id(2)}, std::vector<std::size_t>{1, 1}, 3, true, "manual_cluster",
		"manual_provider");
	assert(manual.record_count == 3);
	assert(manual.cluster_count == 2);
	assert(manual.noise_count == 1);
	assert(manual.iterations == 3);
	assert(manual.converged);
	assert(manual.algorithm == "manual_cluster");
	assert(manual.representation == "manual_provider");
	assert(manual.noise_records[0] == space.id(2));

	const auto empty_manual = mtrc::core::make_clustering_result<int>(
		std::vector<std::size_t>{}, std::vector<mtrc::RecordId>{}, std::vector<mtrc::RecordId>{},
		std::vector<mtrc::RecordId>{}, std::vector<std::size_t>{}, 0, false, "empty_cluster",
		"empty_provider");
	assert(empty_manual.empty());
	assert(empty_manual.record_count == 0);
	assert(empty_manual.cluster_count == 0);
	assert(empty_manual.noise_count == 0);
	assert(!empty_manual.converged);
	assert(empty_manual.algorithm == "empty_cluster");
	assert(empty_manual.representation == "empty_provider");

	const auto clustered = mtrc::stats::structural_analysis::dbscan(matrix, 2, 2);
	using Result = decltype(clustered);
	static_assert(std::is_same<typename Result::distance_type, int>::value);

	assert(clustered.algorithm == "dbscan");
	assert(clustered.representation == "pairwise_distances");
	assert(clustered.record_count == space.size());
	assert(clustered.cluster_count == 2);
	assert(clustered.noise_count == 1);
	assert(clustered.iterations == 1);
	assert(clustered.converged);
	assert(clustered.assignments.size() == space.size());
	assert(clustered.assignments[0] == 0);
	assert(clustered.assignments[1] == 0);
	assert(clustered.assignments[2] == 1);
	assert(clustered.assignments[3] == 1);
	assert(clustered.assignments[4] == Result::noise_label);
	assert(clustered.cluster_sizes[0] == 2);
	assert(clustered.cluster_sizes[1] == 2);
	assert(clustered.medoids[0] == space.id(0));
	assert(clustered.medoids[1] == space.id(2));
	assert(clustered.core_records.size() == 4);
	assert(clustered.core_records[0] == space.id(0));
	assert(clustered.core_records[3] == space.id(3));
	assert(clustered.noise_records.size() == 1);
	assert(clustered.noise_records[0] == space.id(4));

	const auto implicit = mtrc::stats::structural_analysis::dbscan(space, 2, 2);
	assert(implicit.representation == "metric_space");
	assert(implicit.assignments == clustered.assignments);
	assert(implicit.noise_records == clustered.noise_records);
	assert(implicit.medoids == clustered.medoids);

	const auto self_core = mtrc::stats::structural_analysis::dbscan(space, 0, 1);
	assert(self_core.cluster_count == space.size());
	assert(self_core.noise_count == 0);
	assert(self_core.core_records.size() == space.size());
	assert(self_core.core_records[0] == space.id(0));
	assert(self_core.core_records[4] == space.id(4));
	assert(self_core.cluster_sizes.size() == space.size());
	assert(self_core.cluster_sizes[0] == 1);
	assert(self_core.cluster_sizes[4] == 1);

	bool rejected_bad_radius = false;
	try {
		(void)mtrc::stats::structural_analysis::dbscan(matrix, -1, 2);
	} catch (const std::invalid_argument &) {
		rejected_bad_radius = true;
	}
	assert(rejected_bad_radius);

	bool rejected_bad_min_points = false;
	try {
		(void)mtrc::stats::structural_analysis::dbscan(matrix, 1, 0);
	} catch (const std::invalid_argument &) {
		rejected_bad_min_points = true;
	}
	assert(rejected_bad_min_points);

	return 0;
}
