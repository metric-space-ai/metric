// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Investigating an existing finite metric space with the mtrc::stats user API.
//
// mtrc::stats is read-only: it inspects, ranks, samples, compares, and describes a finite
// metric space without ever defining a metric or building a derived space. This example
// uses small integer records with an absolute-difference metric -- vectors/coordinates are
// not required; everything is computed from metric values.

#include <iostream>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/metric/catalog.hpp"
#include "metric/stats/correlate/significance.hpp"
#include "metric/stats/properties/profile.hpp"
#include "metric/stats/search/nearest.hpp"
#include "metric/stats/structural_analysis/clustering.hpp"
#include "metric/stats/structural_analysis/diagnostics.hpp"
#include "metric/stats/structural_analysis/outliers.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

} // namespace

int main()
{
	namespace properties = mtrc::stats::properties;
	namespace search = mtrc::stats::search;
	namespace structural = mtrc::stats::structural_analysis;
	namespace correlate = mtrc::stats::correlate;

	// Two well-separated groups plus one far outlier.
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 50, 51, 52, 200}, AbsoluteDistance{});

	// 1. One cohesive diagnostic summary, with the optional distribution section requested.
	properties::profile_options options;
	options.include_distance_distribution = true;
	const auto report = properties::profile(space, options);
	std::cout << "records=" << report.record_count << " pairs=" << report.pair_count
			  << " min_nonzero=" << report.minimum_nonzero_distance << " max=" << report.maximum_distance
			  << " avg=" << report.average_distance << " intrinsic_dim=" << report.intrinsic_dimension << "\n";
	std::cout << "median pairwise distance=" << report.distance_distribution.median << "\n";

	// 2. A nearest-neighbor query (results carry exactness + representation metadata).
	const auto nearest = search::knn(space, space.id(0), 2);
	std::cout << "two nearest to record 0:";
	for (const auto &neighbor : nearest) {
		std::cout << ' ' << neighbor.id.index() << "(d=" << neighbor.distance << ')';
	}
	std::cout << "  [" << nearest.representation << (nearest.exact ? ", exact]\n" : "]\n");

	// 3. Cluster the space and check validity with metric-space silhouette diagnostics.
	const auto clusters = structural::kmedoids(space, 2);
	const auto diagnostics = structural::cluster_diagnostics(space, clusters);
	std::cout << "kmedoids clusters=" << diagnostics.cluster_count
			  << " mean_silhouette=" << diagnostics.mean_silhouette
			  << " mean_intra=" << diagnostics.mean_intra_cluster_distance << "\n";

	// 4. Score isolation with a k-NN distance outlier score (the far record ranks first).
	const auto outliers = structural::nearest_neighbor_outliers(space, 1);
	std::cout << "strongest outlier=record " << outliers[0].id.index() << " score=" << outliers[0].score << "\n";

	// 5. Test dependence between two paired finite metric spaces. MGC is a dependence
	//    statistic in [-1, 1], not a metric; mgc_significance adds a seeded permutation
	//    p-value. Here the right space is an affine image of the left -> strong dependence.
	std::vector<std::vector<double>> left, right;
	for (int i = 0; i < 12; ++i) {
		left.push_back({static_cast<double>(i)});
		right.push_back({static_cast<double>(i) * 3.0 - 4.0});
	}
	const auto left_space = mtrc::make_space(left, mtrc::Euclidean<double>());
	const auto right_space = mtrc::make_space(right, mtrc::Euclidean<double>());
	const auto significance = correlate::mgc_significance(left_space, right_space, correlate::significance_options(99));
	std::cout << "mgc statistic=" << significance.statistic << " p_value=" << significance.p_value << " over "
			  << significance.permutations << " permutations\n";

	return 0;
}
