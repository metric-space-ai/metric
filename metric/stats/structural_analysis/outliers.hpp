// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_STRUCTURAL_ANALYSIS_OUTLIERS_HPP
#define _METRIC_STATS_STRUCTURAL_ANALYSIS_OUTLIERS_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/result.hpp>
#include <metric/stats/structural_analysis/clustering.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/stats/structural_analysis/options.hpp>

namespace mtrc::stats::structural_analysis {
namespace outlier_detail {

// Distance from source_id to its k-th nearest OTHER record (self excluded). This is the
// classic k-NN outlier score (Ramaswamy et al. 2000): a record far from its k-th neighbor
// sits in a sparse region of the finite metric space. Precondition: at least k other
// records exist.
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto kth_nearest_other_distance(const Provider &provider, RecordId source_id, std::size_t k)
	-> typename Provider::distance_type
{
	using distance_type = typename Provider::distance_type;

	std::vector<distance_type> distances;
	distances.reserve(provider.record_count());
	for (std::size_t position = 0; position < provider.record_count(); ++position) {
		const auto candidate_id = provider.id(position);
		if (candidate_id == source_id) {
			continue;
		}
		distances.push_back(provider.distance(source_id, candidate_id));
	}

	std::nth_element(distances.begin(), distances.begin() + static_cast<std::ptrdiff_t>(k - 1), distances.end());
	return distances[k - 1];
}

template <typename Provider>
auto outlier_result_from_groups(const Provider &provider,
								const ClusteringResult<typename Provider::distance_type> &groups)
	-> OutlierResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	const auto references = ::mtrc::non_noise_record_ids(provider, groups);

	auto outliers = core::scored_outliers<distance_type>(groups.noise_records, [&](RecordId id) {
		return references.empty()
			? core::nearest_other_record_distance_or(provider, id, distance_type{})
			: core::nearest_distance_to_record_ids(provider, id, references, "outlier references must not be empty");
	});

	return core::make_outlier_result(std::move(outliers), groups.record_count, groups.cluster_count, groups.noise_count,
									 "dbscan_noise", groups.representation);
}

} // namespace outlier_detail

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_outliers(const Provider &provider, stats::structural_analysis::dbscan_options strategy)
	-> OutlierResult<typename Provider::distance_type>
{
	const auto groups = stats::structural_analysis::dbscan(provider, strategy.radius, strategy.min_points);
	return outlier_detail::outlier_result_from_groups(provider, groups);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, stats::structural_analysis::dbscan_options strategy) -> OutlierResult<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = find_outliers(provider, strategy);
	result.representation = "metric_space";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, stats::structural_analysis::dbscan_options strategy, space::storage::policy runtime_policy)
	-> OutlierResult<typename Space::distance_type>
{
	space::storage::require_exact_outliers(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		space::storage::DistanceTable<Space> matrix(space);
		auto result = find_outliers(matrix, strategy);
		result.representation = space::storage::outlier_representation(runtime_policy);
		return result;
	}

	auto result = find_outliers(space, strategy);
	result.representation = space::storage::outlier_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, double radius, std::size_t min_points)
	-> OutlierResult<typename Space::distance_type>
{
	return find_outliers(space, stats::structural_analysis::dbscan_options(radius, min_points));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_outliers(const Space &space, double radius, std::size_t min_points, space::storage::policy runtime_policy)
	-> OutlierResult<typename Space::distance_type>
{
	return find_outliers(space, stats::structural_analysis::dbscan_options(radius, min_points), runtime_policy);
}

// k-NN distance outlier score. Unlike DBSCAN-noise outliers this scores EVERY record by how
// isolated it is (distance to its k-th nearest neighbor), so there is no radius/min_points
// to tune. The result is sorted by score descending, ties broken by RecordId ascending. The
// reported cluster_count/noise_count are 0 (this is a per-record score, not a partition).
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto nearest_neighbor_outliers(const Provider &provider, std::size_t k)
	-> OutlierResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (k == 0) {
		throw std::invalid_argument("nearest_neighbor_outliers requires k >= 1");
	}
	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot score outliers in an empty distance provider");
	}
	if (k > provider.record_count() - 1) {
		throw std::invalid_argument("k must not exceed record_count - 1");
	}

	std::vector<RecordId> ids;
	ids.reserve(provider.record_count());
	for (std::size_t position = 0; position < provider.record_count(); ++position) {
		ids.push_back(provider.id(position));
	}

	auto outliers = core::scored_outliers<distance_type>(ids, [&provider, k](RecordId id) {
		return outlier_detail::kth_nearest_other_distance(provider, id, k);
	});

	return core::make_outlier_result(std::move(outliers), provider.record_count(), 0, 0,
									 "nearest_neighbor_distance", "pairwise_distances");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto nearest_neighbor_outliers(const Space &space, std::size_t k) -> OutlierResult<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = nearest_neighbor_outliers(provider, k);
	result.representation = "metric_space";
	return result;
}

} // namespace mtrc::stats::structural_analysis

namespace mtrc {
using stats::structural_analysis::find_outliers;
} // namespace mtrc

#endif
