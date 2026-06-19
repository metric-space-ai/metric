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

	bool rejected_materialized_record_query = false;
	try {
		(void)metric::find_neighbors(space, std::string("ee"), 2, materialized_policy);
	} catch (const std::invalid_argument &) {
		rejected_materialized_record_query = true;
	}
	assert(rejected_materialized_record_query);

	return 0;
}
