// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Records+metric interop: base algorithms should not force callers to hand-build a
// MetricSpace when they already have records and an authoritative metric callable.

#include <cassert>
#include <cmath>
#include <string>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/space/select/representatives.hpp"
#include "metric/space/storage/implicit.hpp"
#include "metric/stats/properties/describe.hpp"
#include "metric/stats/properties/distribution.hpp"
#include "metric/stats/properties/local_volume.hpp"
#include "metric/stats/properties/profile.hpp"
#include "metric/stats/sample/sample.hpp"
#include "metric/stats/search/neighbors.hpp"
#include "metric/stats/structural_analysis/groups.hpp"
#include "metric/stats/structural_analysis/outliers.hpp"

namespace {

struct Item {
	int position{};
	std::string family;
};

struct ItemMetric {
	auto operator()(const Item &lhs, const Item &rhs) const -> double
	{
		const auto delta = lhs.position > rhs.position ? lhs.position - rhs.position : rhs.position - lhs.position;
		return static_cast<double>(delta) + (lhs.family == rhs.family ? 0.0 : 0.25);
	}
};

auto records() -> std::vector<Item>
{
	return {{0, "a"}, {1, "a"}, {5, "b"}, {6, "b"}, {12, "c"}};
}

void records_metric_properties_match_space_entry_points()
{
	const auto data = records();
	const ItemMetric metric;

	const auto structure = mtrc::describe_structure(data, metric);
	assert(structure.record_count == data.size());
	assert(structure.pair_count == 10);
	assert(structure.representation == "records");

	mtrc::profile_options options;
	options.include_distance_distribution = true;
	options.include_local_volume = true;
	options.local_volume_radius = 1.5;
	const auto prof = mtrc::profile(data, metric, options);
	assert(prof.record_count == data.size());
	assert(prof.has_distance_distribution);
	assert(prof.has_local_volume);
	assert(prof.representation == "records");
	assert(prof.distance_distribution.representation == "records");
	assert(prof.local_volume.representation == "records");

	const auto distribution = mtrc::distance_distribution(data, metric);
	assert(distribution.record_count == data.size());
	assert(distribution.pair_count == 10);
	assert(distribution.representation == "records");

	const auto volume = mtrc::local_volume(data, metric, 1.5);
	assert(volume.record_count == data.size());
	assert(volume.representation == "records");
	assert(volume.counts[0] == 2);
	assert(volume.counts[2] == 2);

	const auto growth = mtrc::local_volume_profile(data, metric, std::vector<double>{0.0, 1.5});
	assert(growth.record_count == data.size());
	assert(growth.size() == 2);
	assert(growth.representation == "records");
}

void records_metric_search_sample_and_selection_share_the_same_metric()
{
	const auto data = records();
	const ItemMetric metric;
	const Item query{1, "b"};

	const auto neighbors = mtrc::find_neighbors(data, metric, query, 2);
	assert(neighbors.size() == 2);
	assert(neighbors.representation == "records");
	assert(neighbors[0].id.index() == 1);
	assert(neighbors[1].id.index() == 0);

	const auto indexed_neighbors =
		mtrc::find_neighbors(data, metric, query, 2, mtrc::stats::search::exact_scan{});
	assert(indexed_neighbors.size() == 2);
	assert(indexed_neighbors.representation == "exact_scan_index");

	const auto within = mtrc::stats::search::range(data, metric, query, 1.5);
	assert(within.size() == 2);
	assert(within.representation == "records");

	const auto regular = mtrc::regular_sample(data, metric, 3);
	assert(regular.size() == 3);
	assert(regular.representation == "records");

	const auto farthest = mtrc::farthest_first(data, metric, 2);
	assert(farthest.size() == 2);
	assert(farthest.representation == "records");

	const auto walk = mtrc::metric_walk(data, metric, 3);
	assert(walk.size() == 3);
	assert(walk.representation == "records");

	const auto representatives = mtrc::find_representatives(data, metric, 2);
	assert(representatives.size() == 2);
	assert(representatives.representation == "records");
}

void records_metric_structural_algorithms_share_the_same_metric()
{
	const auto data = records();
	const ItemMetric metric;

	const auto groups = mtrc::find_groups(data, metric, mtrc::k_medoids_options(2, 8));
	assert(groups.record_count == data.size());
	assert(groups.cluster_count == 2);
	assert(groups.representation == "records");

	const auto dbscan_groups = mtrc::find_groups(data, metric, mtrc::dbscan_options(1.5, 2));
	assert(dbscan_groups.record_count == data.size());
	assert(dbscan_groups.cluster_count == 2);
	assert(dbscan_groups.representation == "records");

	const auto outliers = mtrc::find_outliers(data, metric, mtrc::dbscan_options(1.5, 2));
	assert(outliers.record_count == data.size());
	assert(outliers.representation == "records");

	const auto scored = mtrc::nearest_neighbor_outliers(data, metric, 1);
	assert(scored.record_count == data.size());
	assert(scored.size() == data.size());
	assert(scored.representation == "records");
}

void provider_entry_points_still_work()
{
	const auto space = mtrc::make_space(records(), ItemMetric{});
	const mtrc::space::storage::LiveDistances<decltype(space)> provider(space);

	const auto groups = mtrc::find_groups(provider, mtrc::k_medoids_options(2, 8));
	assert(groups.record_count == space.size());
	assert(groups.representation == "pairwise_distances");

	const auto outliers = mtrc::find_outliers(provider, mtrc::dbscan_options(1.5, 2));
	assert(outliers.record_count == space.size());
	assert(outliers.representation == "pairwise_distances");
}

} // namespace

int main()
{
	records_metric_properties_match_space_entry_points();
	records_metric_search_sample_and_selection_share_the_same_metric();
	records_metric_structural_algorithms_share_the_same_metric();
	provider_entry_points_still_work();
	return 0;
}
