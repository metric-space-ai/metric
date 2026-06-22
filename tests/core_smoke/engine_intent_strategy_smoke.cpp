// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"

struct StringLengthDistance {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> int
	{
		const auto difference = static_cast<int>(lhs.size()) - static_cast<int>(rhs.size());
		return difference < 0 ? -difference : difference;
	}
};

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		const auto difference = lhs - rhs;
		return difference < 0 ? -difference : difference;
	}
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
	auto strings = mtrc::make_space(std::vector<std::string>{"a", "bb", "ccc", "dddd"}, StringLengthDistance{});

	const auto direct_neighbors = mtrc::find_neighbors(strings, std::string("ee"), 2);
	assert(direct_neighbors.operator_name == "knn");
	assert(direct_neighbors.representation == "metric_space");
	assert(direct_neighbors.size() == 2);
	assert(direct_neighbors[0].id == strings.id(1));

	const auto counted_neighbors = mtrc::find_neighbors(strings, std::string("ee"), mtrc::count{2});
	assert(counted_neighbors.requested_count == mtrc::count{2}.size());
	assert(counted_neighbors.size() == direct_neighbors.size());
	assert(counted_neighbors[0].id == direct_neighbors[0].id);
	assert(counted_neighbors[1].id == direct_neighbors[1].id);

	const auto cached_neighbors = mtrc::find_neighbors(strings, strings.id(0), 2, mtrc::stats::search::distance_table{});
	assert(cached_neighbors.representation == "distance_table");
	assert(cached_neighbors.size() == 2);
	assert(cached_neighbors[0].id == strings.id(1));

	const auto counted_cached_neighbors =
		mtrc::find_neighbors(strings, strings.id(0), mtrc::count{2}, mtrc::stats::search::distance_table{});
	assert(counted_cached_neighbors.representation == "distance_table");
	assert(counted_cached_neighbors[0].id == cached_neighbors[0].id);

	auto edit_strings =
		mtrc::make_space(std::vector<std::string>{"metric", "metrics", "matrix", "tree"}, mtrc::Edit<char>{});
	const auto edit_direct_neighbors = mtrc::find_neighbors(edit_strings, std::string("metricks"), 2);
	const auto tree_neighbors =
		mtrc::find_neighbors(edit_strings, std::string("metricks"), 2, mtrc::stats::search::cover_tree{});
	assert(tree_neighbors.representation == "cover_tree_index");
	assert(tree_neighbors.size() == edit_direct_neighbors.size());
	assert(tree_neighbors[0].id == edit_direct_neighbors[0].id);
	assert(tree_neighbors[0].distance == edit_direct_neighbors[0].distance);
	assert(tree_neighbors[1].id == edit_direct_neighbors[1].id);
	assert(tree_neighbors[1].distance == edit_direct_neighbors[1].distance);

	const auto counted_tree_neighbors = mtrc::find_neighbors(edit_strings, std::string("metricks"), mtrc::count{2},
															   mtrc::stats::search::cover_tree{});
	assert(counted_tree_neighbors.representation == "cover_tree_index");
	assert(counted_tree_neighbors[0].id == tree_neighbors[0].id);

	const auto graph_neighbors = mtrc::find_neighbors(strings, strings.id(0), 2, mtrc::stats::search::knn_graph(2));
	assert(graph_neighbors.representation == "knn_graph_index");
	assert(graph_neighbors.exact);
	assert(graph_neighbors.size() == cached_neighbors.size());
	assert(graph_neighbors[0].id == cached_neighbors[0].id);
	assert(graph_neighbors[0].distance == cached_neighbors[0].distance);
	assert(graph_neighbors[1].id == cached_neighbors[1].id);
	assert(graph_neighbors[1].distance == cached_neighbors[1].distance);

	const auto counted_graph_neighbors =
		mtrc::find_neighbors(strings, strings.id(0), mtrc::count{2}, mtrc::stats::search::knn_graph(2));
	assert(counted_graph_neighbors.representation == "knn_graph_index");
	assert(counted_graph_neighbors.exact);
	assert(counted_graph_neighbors[0].id == graph_neighbors[0].id);
	const auto default_degree_graph_neighbors =
		mtrc::find_neighbors(strings, strings.id(0), 2, mtrc::stats::search::knn_graph{});
	assert(default_degree_graph_neighbors.representation == "knn_graph_index");
	assert(default_degree_graph_neighbors.exact);
	assert(default_degree_graph_neighbors[0].id == graph_neighbors[0].id);

	bool rejected_underprovisioned_graph = false;
	try {
		(void)mtrc::find_neighbors(strings, strings.id(0), 2, mtrc::stats::search::knn_graph(1));
	} catch (const mtrc::RepresentationError &) {
		rejected_underprovisioned_graph = true;
	}
	assert(rejected_underprovisioned_graph);

	auto numbers = mtrc::make_space(std::vector<int>{0, 1, 10, 11, 30}, AbsoluteDistance{});
	const auto medoid_groups = mtrc::find_groups(numbers, mtrc::stats::structural_analysis::k_medoids_options(2));
	assert(medoid_groups.algorithm == "kmedoids");
	assert(medoid_groups.cluster_count == 2);
	assert(medoid_groups.representation == "metric_space");

	const auto default_groups = mtrc::find_groups(numbers, 2);
	assert(default_groups.algorithm == "kmedoids");
	assert(default_groups.assignments == medoid_groups.assignments);

	const auto density_groups = mtrc::find_groups(numbers, mtrc::stats::structural_analysis::dbscan_options(2.0, 2));
	assert(density_groups.algorithm == "dbscan");
	assert(density_groups.cluster_count == 2);
	assert(density_groups.noise_count == 1);
	assert(density_groups.noise_records[0] == numbers.id(4));

	auto continuous = mtrc::make_space(std::vector<double>{0.0, 0.1, 10.0, 10.1}, DoubleAbsoluteDistance{});
	const auto affinity_groups = mtrc::find_groups(continuous, mtrc::stats::structural_analysis::affinity_propagation_options(0.7));
	assert(affinity_groups.algorithm == "affinity_propagation");
	assert(affinity_groups.representation == "metric_space");
	assert(affinity_groups.record_count == continuous.size());
	assert(affinity_groups.cluster_count == affinity_groups.medoids.size());

	return 0;
}
