// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Structural-analysis additions: metric-space cluster validity diagnostics (silhouette
// and intra/inter-cluster distances) and a k-NN distance outlier score. Both are read-only
// investigations of an existing finite metric space and are deterministic.

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/stats/structural_analysis/clustering.hpp"
#include "metric/stats/structural_analysis/diagnostics.hpp"
#include "metric/stats/structural_analysis/outliers.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool { return std::abs(lhs - rhs) < tolerance; }

template <typename Function> auto throws_invalid_argument(Function run) -> bool
{
	try {
		run();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

void cluster_diagnostics_on_two_clean_clusters()
{
	namespace structural = mtrc::stats::structural_analysis;
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 100, 101, 102}, AbsoluteDistance{});

	const auto clusters = structural::kmedoids(space, 2);
	const auto diagnostics = structural::cluster_diagnostics(space, clusters);

	assert(diagnostics.algorithm == "cluster_diagnostics");
	assert(diagnostics.cluster_count == 2);
	assert(diagnostics.evaluated_record_count == 6);
	assert(diagnostics.noise_count == 0);
	assert((diagnostics.cluster_sizes == std::vector<std::size_t>{3, 3}));
	assert(diagnostics.silhouettes.size() == 6);
	assert(diagnostics.evaluated_ids.size() == 6);
	assert(diagnostics.per_cluster_mean_intra_distance.size() == 2);

	// Tight, well-separated clusters -> silhouette near 1, small intra and large inter.
	assert(diagnostics.mean_silhouette > 0.95);
	assert(close(diagnostics.mean_intra_cluster_distance, 4.0 / 3.0));
	assert(close(diagnostics.mean_nearest_cluster_distance, 100.0));

	// Determinism: identical on a second run.
	const auto again = structural::cluster_diagnostics(space, structural::kmedoids(space, 2));
	assert(close(again.mean_silhouette, diagnostics.mean_silhouette));
	assert(again.silhouettes == diagnostics.silhouettes);
}

void cluster_diagnostics_single_cluster_is_zero()
{
	namespace structural = mtrc::stats::structural_analysis;
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, AbsoluteDistance{});

	const auto single = structural::kmedoids(space, 1);
	const auto diagnostics = structural::cluster_diagnostics(space, single);
	assert(diagnostics.cluster_count == 1);
	assert(diagnostics.evaluated_record_count == 4);
	// No second cluster -> silhouette and nearest-cluster distance are 0 by convention.
	assert(close(diagnostics.mean_silhouette, 0.0));
	assert(close(diagnostics.mean_nearest_cluster_distance, 0.0));
}

void cluster_diagnostics_excludes_noise()
{
	namespace structural = mtrc::stats::structural_analysis;
	// Two dense triplets plus a single far record that DBSCAN labels as noise.
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 20, 21, 22, 50}, AbsoluteDistance{});

	const auto clusters = structural::dbscan(space, 2, 2);
	const auto diagnostics = structural::cluster_diagnostics(space, clusters);
	assert(diagnostics.cluster_count == 2);
	assert(diagnostics.noise_count == 1);
	assert(diagnostics.evaluated_record_count == 6); // the noise record is not evaluated
	assert(diagnostics.mean_silhouette > 0.9);
}

void nearest_neighbor_outliers_score_isolation()
{
	namespace structural = mtrc::stats::structural_analysis;
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 100}, AbsoluteDistance{});

	const auto outliers = structural::nearest_neighbor_outliers(space, 1);
	assert(outliers.strategy == "nearest_neighbor_distance");
	assert(outliers.representation == "metric_space");
	assert(outliers.size() == 4);
	// The far record (value 100) is the strongest outlier: its nearest neighbor is at 98.
	assert(outliers[0].id == space.id(3));
	assert(outliers[0].score == 98);
	// Sorted by score descending.
	for (std::size_t index = 1; index < outliers.size(); ++index) {
		assert(outliers[index - 1].score >= outliers[index].score);
	}

	assert(throws_invalid_argument([&] { (void)structural::nearest_neighbor_outliers(space, 0); }));
	assert(throws_invalid_argument([&] { (void)structural::nearest_neighbor_outliers(space, 4); }));
}

} // namespace

int main()
{
	cluster_diagnostics_on_two_clean_clusters();
	cluster_diagnostics_single_cluster_is_zero();
	cluster_diagnostics_excludes_noise();
	nearest_neighbor_outliers_score_isolation();
	return 0;
}
