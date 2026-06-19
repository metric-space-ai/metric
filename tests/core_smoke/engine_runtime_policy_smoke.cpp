#include <cassert>
#include <stdexcept>
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

namespace metric::core {

template <> struct metric_traits<StatefulLengthDistance> {
	static constexpr auto law = metric_law::distance;
	static constexpr auto records = record_kind::string;
	static constexpr bool thread_safe = false;
};

} // namespace metric::core

int main()
{
	auto space = metric::make_space(std::vector<std::string>{"a", "bb", "ccc", "dddd"}, StringLengthDistance{});

	const auto lazy_policy = metric::runtime::exact();
	assert(lazy_policy.name() == "exact_lazy_serial");
	assert(lazy_policy.representation_preference() == "implicit");
	const auto lazy_diagnostics = metric::runtime::diagnostics(lazy_policy, {}, "neighbors");
	assert(lazy_diagnostics.policy_name == "exact_lazy_serial");
	assert(lazy_diagnostics.exact);
	assert(!lazy_diagnostics.parallel);
	assert(!lazy_diagnostics.materialized);
	assert(lazy_diagnostics.representation == "metric_space");
	assert(lazy_diagnostics.intent == "neighbors");
	assert(lazy_diagnostics.supported);
	assert(lazy_diagnostics.reason.empty());

	const auto lazy_neighbors = metric::find_neighbors(space, space.id(0), 2, lazy_policy);
	assert(lazy_neighbors.exact);
	assert(lazy_neighbors.representation == "metric_space");
	assert(lazy_neighbors.size() == 2);
	assert(lazy_neighbors[0].id == space.id(1));

	const auto materialized_policy = metric::runtime::materialized(metric::runtime::exact());
	assert(materialized_policy.name() == "exact_materialized_serial");
	assert(materialized_policy.representation_preference() == "matrix_cache");
	const auto materialized_diagnostics = metric::runtime::diagnostics(materialized_policy, {}, "neighbors");
	assert(materialized_diagnostics.policy_name == "exact_materialized_serial");
	assert(materialized_diagnostics.materialized);
	assert(materialized_diagnostics.representation == "matrix_cache");

	const auto materialized_neighbors = metric::find_neighbors(space, space.id(0), 2, materialized_policy);
	assert(materialized_neighbors.exact);
	assert(materialized_neighbors.representation == "matrix_cache");
	assert(materialized_neighbors.size() == lazy_neighbors.size());
	assert(materialized_neighbors[0].id == lazy_neighbors[0].id);
	assert(materialized_neighbors[0].distance == lazy_neighbors[0].distance);

	const auto counted_materialized_neighbors =
		metric::find_neighbors(space, space.id(0), metric::count{2}, materialized_policy);
	assert(counted_materialized_neighbors.representation == "matrix_cache");
	assert(counted_materialized_neighbors[0].id == materialized_neighbors[0].id);

	const auto parallel_policy = metric::runtime::parallel(materialized_policy);
	assert(parallel_policy.name() == "exact_materialized_parallel");
	assert(parallel_policy.uses_parallel_execution());
	const auto explicit_diagnostics = metric::runtime::diagnostics(parallel_policy, "cover_tree_index", "neighbors");
	assert(explicit_diagnostics.parallel);
	assert(explicit_diagnostics.materialized);
	assert(explicit_diagnostics.representation == "cover_tree_index");
	const auto parallel_neighbors = metric::find_neighbors(space, space.id(0), 2, parallel_policy);
	assert(parallel_neighbors.representation == "matrix_cache");

	auto stateful_space =
		metric::make_space(std::vector<std::string>{"a", "bb", "ccc", "dddd"}, StatefulLengthDistance{});
	const auto stateful_policy = metric::runtime::parallel(metric::runtime::exact());
	static_assert(!metric::metric_thread_safe_v<StatefulLengthDistance>);
	const auto stateful_diagnostics =
		metric::runtime::diagnostics_for_metric<StatefulLengthDistance>(stateful_policy, {}, "neighbors");
	assert(stateful_diagnostics.parallel);
	assert(!stateful_diagnostics.supported);
	assert(stateful_diagnostics.reason.find("thread-safe") != std::string::npos);
	const auto stateful_space_diagnostics =
		metric::runtime::diagnostics_for_space(stateful_space, stateful_policy, {}, "neighbors");
	assert(!stateful_space_diagnostics.supported);

	bool rejected_stateful_parallel_neighbors = false;
	try {
		(void)metric::find_neighbors(stateful_space, stateful_space.id(0), 2, stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_neighbors = true;
	}
	assert(rejected_stateful_parallel_neighbors);

	bool rejected_stateful_parallel_groups = false;
	try {
		(void)metric::find_groups(stateful_space, metric::strategies::k_medoids(2), stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_groups = true;
	}
	assert(rejected_stateful_parallel_groups);

	bool rejected_stateful_parallel_describe = false;
	try {
		(void)metric::describe_structure(stateful_space, stateful_policy);
	} catch (const std::invalid_argument &) {
		rejected_stateful_parallel_describe = true;
	}
	assert(rejected_stateful_parallel_describe);

	const auto lazy_groups = metric::find_groups(space, metric::strategies::k_medoids(2), lazy_policy);
	assert(lazy_groups.algorithm == "kmedoids");
	assert(lazy_groups.representation == "metric_space");
	assert(lazy_groups.cluster_count == 2);

	const auto materialized_groups = metric::find_groups(space, metric::strategies::k_medoids(2), materialized_policy);
	assert(materialized_groups.algorithm == "kmedoids");
	assert(materialized_groups.representation == "matrix_cache");
	assert(materialized_groups.assignments == lazy_groups.assignments);
	assert(materialized_groups.medoids == lazy_groups.medoids);

	const auto counted_materialized_groups = metric::find_groups(space, 2, materialized_policy);
	assert(counted_materialized_groups.representation == "matrix_cache");
	assert(counted_materialized_groups.assignments == materialized_groups.assignments);

	const auto density_groups = metric::find_groups(space, metric::strategies::dbscan(1.0, 2), materialized_policy);
	assert(density_groups.algorithm == "dbscan");
	assert(density_groups.representation == "matrix_cache");
	assert(density_groups.record_count == space.size());

	auto continuous_space = metric::make_space(std::vector<double>{0.0, 0.1, 10.0, 10.1}, DoubleAbsoluteDistance{});
	const auto affinity_groups =
		metric::find_groups(continuous_space, metric::strategies::affinity_propagation(0.7), materialized_policy);
	assert(affinity_groups.algorithm == "affinity_propagation");
	assert(affinity_groups.representation == "matrix_cache");
	assert(affinity_groups.record_count == continuous_space.size());
	assert(affinity_groups.cluster_count == affinity_groups.medoids.size());

	auto cached_matrix = metric::runtime::cache(metric::representations::MatrixCache<decltype(space)>(space));
	assert(!cached_matrix.is_stale());
	assert(cached_matrix.version() == space.version());
	space.touch();
	assert(cached_matrix.is_stale());

	bool rejected_approximate = false;
	try {
		(void)metric::find_neighbors(space, space.id(0), 2, metric::runtime::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate = true;
	}
	assert(rejected_approximate);
	const auto approximate_diagnostics = metric::runtime::diagnostics(metric::runtime::approximate(), {}, "neighbors");
	assert(!approximate_diagnostics.exact);
	assert(!approximate_diagnostics.supported);
	assert(!approximate_diagnostics.reason.empty());

	bool rejected_approximate_groups = false;
	try {
		(void)metric::find_groups(space, metric::strategies::k_medoids(2), metric::runtime::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_groups = true;
	}
	assert(rejected_approximate_groups);
	const auto group_diagnostics = metric::runtime::diagnostics(materialized_policy, {}, "groups");
	assert(group_diagnostics.representation == "matrix_cache");
	assert(group_diagnostics.intent == "groups");

	bool rejected_materialized_record_query = false;
	try {
		(void)metric::find_neighbors(space, std::string("ee"), 2, materialized_policy);
	} catch (const std::invalid_argument &) {
		rejected_materialized_record_query = true;
	}
	assert(rejected_materialized_record_query);

	return 0;
}
