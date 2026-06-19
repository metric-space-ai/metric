// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <string>
#include <vector>

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

int main()
{
	auto strings = metric::make_space(std::vector<std::string>{"a", "bb", "ccc", "dddd"}, StringLengthDistance{});

	const auto direct_neighbors = metric::find_neighbors(strings, std::string("ee"), 2);
	assert(direct_neighbors.operator_name == "knn");
	assert(direct_neighbors.representation == "metric_space");
	assert(direct_neighbors.size() == 2);
	assert(direct_neighbors[0].id == strings.id(1));

	const auto cached_neighbors =
		metric::find_neighbors(strings, strings.id(0), 2, metric::strategies::matrix_cache{});
	assert(cached_neighbors.representation == "matrix_cache");
	assert(cached_neighbors.size() == 2);
	assert(cached_neighbors[0].id == strings.id(1));

	const auto tree_neighbors =
		metric::find_neighbors(strings, std::string("ee"), 2, metric::strategies::cover_tree{});
	assert(tree_neighbors.representation == "cover_tree_index");
	assert(tree_neighbors.size() == direct_neighbors.size());
	assert(tree_neighbors[0].id == direct_neighbors[0].id);
	assert(tree_neighbors[0].distance == direct_neighbors[0].distance);
	assert(tree_neighbors[1].id == direct_neighbors[1].id);
	assert(tree_neighbors[1].distance == direct_neighbors[1].distance);

	const auto graph_neighbors =
		metric::find_neighbors(strings, strings.id(0), 2, metric::strategies::knn_graph(2));
	assert(graph_neighbors.representation == "knn_graph_index");
	assert(graph_neighbors.size() == cached_neighbors.size());
	assert(graph_neighbors[0].id == cached_neighbors[0].id);
	assert(graph_neighbors[0].distance == cached_neighbors[0].distance);
	assert(graph_neighbors[1].id == cached_neighbors[1].id);
	assert(graph_neighbors[1].distance == cached_neighbors[1].distance);

	auto numbers = metric::make_space(std::vector<int>{0, 1, 10, 11, 30}, AbsoluteDistance{});
	const auto medoid_groups = metric::find_groups(numbers, metric::strategies::k_medoids(2));
	assert(medoid_groups.algorithm == "kmedoids");
	assert(medoid_groups.cluster_count == 2);
	assert(medoid_groups.representation == "metric_space");

	const auto default_groups = metric::find_groups(numbers, 2);
	assert(default_groups.algorithm == "kmedoids");
	assert(default_groups.assignments == medoid_groups.assignments);

	const auto density_groups = metric::find_groups(numbers, metric::strategies::dbscan(2.0, 2));
	assert(density_groups.algorithm == "dbscan");
	assert(density_groups.cluster_count == 2);
	assert(density_groups.noise_count == 1);
	assert(density_groups.noise_records[0] == numbers.id(4));

	return 0;
}
