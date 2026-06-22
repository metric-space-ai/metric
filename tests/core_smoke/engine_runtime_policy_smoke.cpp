#include <cassert>
#include <cstddef>
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

struct DoubleAbsoluteDistance {
	auto operator()(double lhs, double rhs) const -> double
	{
		const auto difference = lhs - rhs;
		return difference < 0.0 ? -difference : difference;
	}
};

struct StatefulLengthDistance {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> int
	{
		const auto difference = static_cast<int>(lhs.size()) - static_cast<int>(rhs.size());
		return difference < 0 ? -difference : difference;
	}
};

namespace mtrc::core {

template <> struct metric_traits<StatefulLengthDistance> {
	static constexpr auto law = metric_law::distance;
	static constexpr auto records = record_kind::string;
	static constexpr bool thread_safe = false;
};

} // namespace mtrc::core

int main()
{
	auto space = mtrc::make_space(std::vector<std::string>{"a", "bb", "ccc", "dddd"}, StringLengthDistance{});

	const auto lazy_policy = mtrc::space::storage::exact();
	assert(lazy_policy.name() == "exact_lazy_serial");
	assert(lazy_policy.representation_preference() == "implicit");
	const auto lazy_diagnostics = mtrc::space::storage::diagnostics(lazy_policy, {}, "neighbors");
	assert(lazy_diagnostics.policy_name == "exact_lazy_serial");
	assert(lazy_diagnostics.exact);
	assert(!lazy_diagnostics.parallel);
	assert(!lazy_diagnostics.materialized);
	assert(lazy_diagnostics.representation == "metric_space");
	assert(lazy_diagnostics.intent == "neighbors");
	assert(lazy_diagnostics.supported);
	assert(lazy_diagnostics.reason.empty());

	const auto lazy_neighbors = mtrc::find_neighbors(space, space.id(0), 2, lazy_policy);
	assert(lazy_neighbors.exact);
	assert(lazy_neighbors.representation == "metric_space");
	assert(lazy_neighbors.size() == 2);
	assert(lazy_neighbors[0].id == space.id(1));

	const auto explicit_implicit_policy = mtrc::space::storage::using_implicit();
	assert(explicit_implicit_policy.name() == "exact_lazy_serial");
	assert(explicit_implicit_policy.representation_preference() == "implicit");
	const auto explicit_implicit_diagnostics = mtrc::space::storage::diagnostics(explicit_implicit_policy, {}, "neighbors");
	assert(explicit_implicit_diagnostics.representation == "implicit");
	const auto explicit_implicit_neighbors =
		mtrc::find_neighbors(space, std::string("ee"), 2, explicit_implicit_policy);
	assert(explicit_implicit_neighbors.representation == "implicit");
	assert(explicit_implicit_neighbors[0].id == space.id(1));

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	assert(materialized_policy.name() == "exact_materialized_serial");
	assert(materialized_policy.representation_preference() == "distance_table");
	const auto materialized_diagnostics = mtrc::space::storage::diagnostics(materialized_policy, {}, "neighbors");
	assert(materialized_diagnostics.policy_name == "exact_materialized_serial");
	assert(materialized_diagnostics.materialized);
	assert(materialized_diagnostics.representation == "distance_table");

	const auto materialized_neighbors = mtrc::find_neighbors(space, space.id(0), 2, materialized_policy);
	assert(materialized_neighbors.exact);
	assert(materialized_neighbors.representation == "distance_table");
	assert(materialized_neighbors.size() == lazy_neighbors.size());
	assert(materialized_neighbors[0].id == lazy_neighbors[0].id);
	assert(materialized_neighbors[0].distance == lazy_neighbors[0].distance);

	const auto counted_materialized_neighbors =
		mtrc::find_neighbors(space, space.id(0), mtrc::count{2}, materialized_policy);
	assert(counted_materialized_neighbors.representation == "distance_table");
	assert(counted_materialized_neighbors[0].id == materialized_neighbors[0].id);

	const auto matrix_policy = mtrc::space::storage::using_distance_table();
	assert(matrix_policy.name() == "exact_materialized_serial");
	assert(matrix_policy.representation_preference() == "distance_table");
	const auto matrix_policy_neighbors = mtrc::find_neighbors(space, space.id(0), 2, matrix_policy);
	assert(matrix_policy_neighbors.representation == "distance_table");
	assert(matrix_policy_neighbors[0].id == materialized_neighbors[0].id);

	const auto tree_policy = mtrc::space::storage::using_cover_tree();
	assert(tree_policy.name() == "exact_materialized_serial");
	assert(tree_policy.representation_preference() == "cover_tree_index");
	const auto tree_diagnostics = mtrc::space::storage::diagnostics(tree_policy, {}, "neighbors");
	assert(tree_diagnostics.materialized);
	assert(tree_diagnostics.representation == "cover_tree_index");
	auto edit_space =
		mtrc::make_space(std::vector<std::string>{"metric", "metrics", "matrix", "tree"}, mtrc::Edit<char>{});
	const auto edit_lazy_neighbors = mtrc::find_neighbors(edit_space, std::string("metricks"), 2, lazy_policy);
	const auto tree_query_neighbors = mtrc::find_neighbors(edit_space, std::string("metricks"), 2, tree_policy);
	assert(tree_query_neighbors.representation == "cover_tree_index");
	assert(tree_query_neighbors[0].id == edit_lazy_neighbors[0].id);
	assert(tree_query_neighbors[0].distance == edit_lazy_neighbors[0].distance);
	const auto tree_id_neighbors = mtrc::find_neighbors(edit_space, edit_space.id(0), 2, tree_policy);
	assert(tree_id_neighbors.representation == "cover_tree_index");
	assert(tree_id_neighbors.size() == edit_lazy_neighbors.size());
	const auto edit_lazy_id_neighbors = mtrc::find_neighbors(edit_space, edit_space.id(0), 2, lazy_policy);
	assert(tree_id_neighbors[0].id == edit_lazy_id_neighbors[0].id);
	assert(tree_id_neighbors[0].distance == edit_lazy_id_neighbors[0].distance);
	const auto empty_tree_id_neighbors = mtrc::find_neighbors(edit_space, edit_space.id(0), 0, tree_policy);
	assert(empty_tree_id_neighbors.representation == "cover_tree_index");
	assert(empty_tree_id_neighbors.empty());

	const auto graph_policy = mtrc::space::storage::using_knn_graph(2);
	assert(graph_policy.name() == "exact_materialized_serial");
	assert(graph_policy.graph_neighbors() == 2);
	assert(graph_policy.representation_preference() == "knn_graph_index");
	const auto graph_diagnostics = mtrc::space::storage::diagnostics(graph_policy, {}, "neighbors");
	assert(graph_diagnostics.representation == "knn_graph_index");
	const auto graph_query_neighbors = mtrc::find_neighbors(space, std::string("ee"), 2, graph_policy);
	assert(graph_query_neighbors.representation == "knn_graph_index");
	assert(graph_query_neighbors[0].id == space.id(1));
	const auto graph_id_neighbors = mtrc::find_neighbors(space, space.id(0), 2, graph_policy);
	assert(graph_id_neighbors.representation == "knn_graph_index");
	assert(graph_id_neighbors.exact);
	assert(graph_id_neighbors[0].id == materialized_neighbors[0].id);
	const auto default_graph_policy = mtrc::space::storage::using_knn_graph();
	const auto default_graph_id_neighbors = mtrc::find_neighbors(space, space.id(0), 2, default_graph_policy);
	assert(default_graph_id_neighbors.representation == "knn_graph_index");
	assert(default_graph_id_neighbors.exact);
	bool rejected_underprovisioned_graph_policy = false;
	try {
		(void)mtrc::find_neighbors(space, space.id(0), 2, mtrc::space::storage::using_knn_graph(1));
	} catch (const mtrc::RepresentationError &) {
		rejected_underprovisioned_graph_policy = true;
	}
	assert(rejected_underprovisioned_graph_policy);

	const auto parallel_policy = mtrc::space::storage::parallel(materialized_policy);
	assert(parallel_policy.name() == "exact_materialized_parallel");
	assert(parallel_policy.uses_parallel_execution());
	const auto explicit_diagnostics = mtrc::space::storage::diagnostics(parallel_policy, "cover_tree_index", "neighbors");
	assert(explicit_diagnostics.parallel);
	assert(explicit_diagnostics.materialized);
	assert(explicit_diagnostics.representation == "cover_tree_index");
	const auto parallel_neighbors = mtrc::find_neighbors(space, space.id(0), 2, parallel_policy);
	assert(parallel_neighbors.representation == "distance_table");

	auto stateful_space =
		mtrc::make_space(std::vector<std::string>{"a", "bb", "ccc", "dddd"}, StatefulLengthDistance{});
	const auto stateful_policy = mtrc::space::storage::parallel(mtrc::space::storage::exact());
	static_assert(!mtrc::metric_thread_safe_v<StatefulLengthDistance>);
	const auto stateful_diagnostics =
		mtrc::space::storage::diagnostics_for_metric<StatefulLengthDistance>(stateful_policy, {}, "neighbors");
	assert(stateful_diagnostics.parallel);
	assert(!stateful_diagnostics.supported);
	assert(stateful_diagnostics.reason.find("thread-safe") != std::string::npos);
	const auto stateful_space_diagnostics =
		mtrc::space::storage::diagnostics_for_space(stateful_space, stateful_policy, {}, "neighbors");
	assert(!stateful_space_diagnostics.supported);

	bool rejected_stateful_parallel_neighbors = false;
	try {
		(void)mtrc::find_neighbors(stateful_space, stateful_space.id(0), 2, stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_neighbors = true;
	}
	assert(rejected_stateful_parallel_neighbors);

	bool rejected_stateful_parallel_groups = false;
	try {
		(void)mtrc::find_groups(stateful_space, mtrc::stats::structural_analysis::k_medoids_options(2), stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_groups = true;
	}
	assert(rejected_stateful_parallel_groups);

	bool rejected_stateful_parallel_describe = false;
	try {
		(void)mtrc::describe_structure(stateful_space, stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_describe = true;
	}
	assert(rejected_stateful_parallel_describe);

	bool rejected_stateful_parallel_compare = false;
	try {
		(void)mtrc::compare(stateful_space, stateful_space, stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_compare = true;
	}
	assert(rejected_stateful_parallel_compare);

	auto size_distance = [](std::size_t lhs, std::size_t rhs) -> std::size_t {
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	};
	bool rejected_stateful_parallel_map = false;
	try {
		(void)mtrc::map(
			stateful_space, [](const std::string &record) -> std::size_t { return record.size(); }, size_distance,
			stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_map = true;
	}
	assert(rejected_stateful_parallel_map);

	const auto lazy_groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2), lazy_policy);
	assert(lazy_groups.algorithm == "kmedoids");
	assert(lazy_groups.representation == "metric_space");
	assert(lazy_groups.cluster_count == 2);

	const auto materialized_groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2), materialized_policy);
	assert(materialized_groups.algorithm == "kmedoids");
	assert(materialized_groups.representation == "distance_table");
	assert(materialized_groups.assignments == lazy_groups.assignments);
	assert(materialized_groups.medoids == lazy_groups.medoids);

	const auto counted_materialized_groups = mtrc::find_groups(space, 2, materialized_policy);
	assert(counted_materialized_groups.representation == "distance_table");
	assert(counted_materialized_groups.assignments == materialized_groups.assignments);

	const auto density_groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::dbscan_options(1.0, 2), materialized_policy);
	assert(density_groups.algorithm == "dbscan");
	assert(density_groups.representation == "distance_table");
	assert(density_groups.record_count == space.size());

	auto continuous_space = mtrc::make_space(std::vector<double>{0.0, 0.1, 10.0, 10.1}, DoubleAbsoluteDistance{});
	const auto affinity_groups =
		mtrc::find_groups(continuous_space, mtrc::stats::structural_analysis::affinity_propagation_options(0.7), materialized_policy);
	assert(affinity_groups.algorithm == "affinity_propagation");
	assert(affinity_groups.representation == "distance_table");
	assert(affinity_groups.record_count == continuous_space.size());
	assert(affinity_groups.cluster_count == affinity_groups.medoids.size());

	auto cached_matrix = mtrc::space::storage::cache(mtrc::space::storage::DistanceTable<decltype(space)>(space));
	assert(!cached_matrix.is_stale());
	assert(cached_matrix.version() == space.version());
	space.touch();
	assert(cached_matrix.is_stale());

	bool rejected_approximate = false;
	try {
		(void)mtrc::find_neighbors(space, space.id(0), 2, mtrc::space::storage::approximate());
	} catch (const mtrc::InvalidRuntimePolicyError &) {
		rejected_approximate = true;
	}
	assert(rejected_approximate);
	const auto approximate_diagnostics = mtrc::space::storage::diagnostics(mtrc::space::storage::approximate(), {}, "neighbors");
	assert(!approximate_diagnostics.exact);
	assert(!approximate_diagnostics.supported);
	assert(!approximate_diagnostics.reason.empty());

	bool rejected_approximate_groups = false;
	try {
		(void)mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2), mtrc::space::storage::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_groups = true;
	}
	assert(rejected_approximate_groups);
	const auto group_diagnostics = mtrc::space::storage::diagnostics(materialized_policy, {}, "groups");
	assert(group_diagnostics.representation == "distance_table");
	assert(group_diagnostics.intent == "groups");

	bool rejected_materialized_record_query = false;
	try {
		(void)mtrc::find_neighbors(space, std::string("ee"), 2, materialized_policy);
	} catch (const mtrc::InvalidRuntimePolicyError &) {
		rejected_materialized_record_query = true;
	}
	assert(rejected_materialized_record_query);

	return 0;
}
