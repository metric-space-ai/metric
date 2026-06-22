// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Parity and edge-case coverage for mtrc::stats::search::range (Level-1 stats).
//
// range() answers a radius query over an existing finite metric space without modifying it.
// This test pins: (1) the RecordId overload (range(space, id, radius)) self-excludes the query
// and matches the materialized DistanceTable provider path exactly (exact/index parity);
// (2) the value overload includes an exact-match record; (3) duplicate/tied records keep their
// deterministic (distance, RecordId) ordering; (4) empty input and invalid radius are rejected
// or empty as documented; and (5) a degenerate metric that emits a NaN distance does not invoke
// undefined behavior in neighbor sorting -- NaN-distance neighbors are ordered deterministically
// last (a regression guard for the strict-weak-ordering fix in core/neighbor.hpp).

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/space/query.hpp"
#include "metric/space/storage/distance_table.hpp"
#include "metric/space/storage/policy.hpp"
#include "metric/stats/search/nearest.hpp"
#include "metric/stats/search/neighbors.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

} // namespace

namespace mtrc::core {

template <> struct metric_traits<AbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

} // namespace mtrc::core

namespace {

// A deliberately degenerate "metric": any pair touching the sentinel value (and not equal)
// yields NaN. Used only to prove neighbor ranking stays defined/deterministic under NaN; a
// real metric must return finite distances.
struct NaNSentinelDistance {
	static constexpr double sentinel = 1000.0;
	auto operator()(double lhs, double rhs) const -> double
	{
		if (lhs == rhs) {
			return 0.0;
		}
		if (lhs == sentinel || rhs == sentinel) {
			return std::numeric_limits<double>::quiet_NaN();
		}
		const auto difference = lhs - rhs;
		return difference < 0.0 ? -difference : difference;
	}
};

template <typename Callable> auto throws_out_of_range(Callable &&call) -> bool
{
	try {
		call();
	} catch (const std::out_of_range &) {
		return true;
	}
	return false;
}

template <typename Callable> auto throws_invalid_argument(Callable &&call) -> bool
{
	try {
		call();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

void record_id_range_matches_provider_path()
{
	const auto space = mtrc::make_space(std::vector<int>{0, 4, 8, 12}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(space)> table(space);
	const auto id1 = space.id(1); // value 4

	// RecordId query excludes self; only id0 (d=4) and id2 (d=4) lie within radius 4.
	const auto by_id = mtrc::stats::search::range(space, id1, 4);
	assert(by_id.operator_name == "range");
	assert(by_id.exact);
	assert(by_id.representation == "metric_space");
	assert(by_id.record_count == 4);
	assert(by_id.size() == 2);
	assert(by_id[0].id == space.id(0) && by_id[0].distance == 4); // tie broken by RecordId asc
	assert(by_id[1].id == space.id(2) && by_id[1].distance == 4);

	// Materialized provider path must produce identical neighbors (exact/index parity).
	const auto by_provider = mtrc::stats::search::range(table, id1, 4);
	assert(by_provider.representation == "pairwise_distances");
	assert(by_provider.size() == by_id.size());
	for (std::size_t index = 0; index < by_id.size(); ++index) {
		assert(by_provider[index].id == by_id[index].id);
		assert(by_provider[index].distance == by_id[index].distance);
	}

	// Value query includes the exact-match record (no self-exclusion for value queries).
	const auto by_value = mtrc::stats::search::range(space, 4, 4);
	assert(by_value.size() == 3);
	assert(by_value[0].id == id1 && by_value[0].distance == 0);
}

void duplicate_records_keep_deterministic_ties()
{
	const auto space = mtrc::make_space(std::vector<int>{0, 0, 5, 5}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(space)> table(space);
	const auto id0 = space.id(0);

	// Radius 0 from id0 excludes self but keeps the zero-distance duplicate id1.
	const auto by_id = mtrc::stats::search::range(space, id0, 0);
	assert(by_id.size() == 1);
	assert(by_id[0].id == space.id(1) && by_id[0].distance == 0);

	const auto by_provider = mtrc::stats::search::range(table, id0, 0);
	assert(by_provider.size() == 1);
	assert(by_provider[0].id == space.id(1));
}

template <typename Distance>
void assert_same_neighbors(const mtrc::NeighborSet<Distance> &actual, const mtrc::NeighborSet<Distance> &expected,
						   const char *representation)
{
	assert(actual.representation == representation);
	assert(actual.operator_name == "range");
	assert(actual.exact == expected.exact);
	assert(actual.record_count == expected.record_count);
	assert(actual.size() == expected.size());
	for (std::size_t index = 0; index < expected.size(); ++index) {
		assert(actual[index].id == expected[index].id);
		assert(actual[index].distance == expected[index].distance);
	}
}

void range_strategy_overloads_match_metric_space_path()
{
	const auto space = mtrc::make_space(std::vector<int>{0, 2, 5, 9, 14}, AbsoluteDistance{});
	const auto id0 = space.id(0);
	const auto expected_by_id = mtrc::stats::search::range(space, id0, 5);
	assert(expected_by_id.size() == 2);

	assert_same_neighbors(mtrc::stats::search::range(space, id0, 5, mtrc::stats::search::exact_scan{}),
						  expected_by_id, "exact_scan_index");
	assert_same_neighbors(mtrc::stats::search::range(space, id0, 5, mtrc::stats::search::brute_force{}),
						  expected_by_id, "exact_scan_index");
	assert_same_neighbors(mtrc::stats::search::range(space, id0, 5, mtrc::stats::search::distance_table{}),
						  expected_by_id, "distance_table");
	assert_same_neighbors(mtrc::stats::search::range(space, id0, 5, mtrc::stats::search::cover_tree{}),
						  expected_by_id, "cover_tree_index");
	assert_same_neighbors(mtrc::stats::search::range(space, id0, 5, mtrc::stats::search::knn_graph{}),
						  expected_by_id, "knn_graph_index");

	assert_same_neighbors(mtrc::stats::search::range(space, id0, 5, mtrc::space::storage::using_implicit()),
						  expected_by_id, "implicit");
	assert_same_neighbors(mtrc::stats::search::range(space, id0, 5, mtrc::space::storage::using_distance_table()),
						  expected_by_id, "distance_table");
	assert_same_neighbors(mtrc::stats::search::range(space, id0, 5, mtrc::space::storage::using_cover_tree()),
						  expected_by_id, "cover_tree_index");
	assert_same_neighbors(mtrc::stats::search::range(space, id0, 5, mtrc::space::storage::using_knn_graph()),
						  expected_by_id, "knn_graph_index");
	assert_same_neighbors(mtrc::space::query::within(space, id0, 5, mtrc::stats::search::cover_tree{}),
						  expected_by_id, "cover_tree_index");

	const auto expected_by_value = mtrc::stats::search::range(space, 4, 2);
	assert_same_neighbors(mtrc::stats::search::range(space, 4, 2, mtrc::stats::search::exact_scan{}),
						  expected_by_value, "exact_scan_index");
	assert_same_neighbors(mtrc::stats::search::range(space, 4, 2, mtrc::stats::search::cover_tree{}),
						  expected_by_value, "cover_tree_index");
	assert_same_neighbors(mtrc::stats::search::range(space, 4, 2, mtrc::stats::search::knn_graph{}),
						  expected_by_value, "knn_graph_index");
	assert_same_neighbors(mtrc::space::query::within(space, 4, 2, mtrc::space::storage::using_cover_tree()),
						  expected_by_value, "cover_tree_index");
}

void empty_and_invalid_inputs_are_handled()
{
	const auto empty = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});

	// Value query on an empty space yields an empty result, not a crash.
	const auto value_empty = mtrc::stats::search::range(empty, 3, 2);
	assert(value_empty.size() == 0);
	assert(value_empty.record_count == 0);

	// RecordId query against an out-of-range id is rejected.
	assert(throws_out_of_range([&] { (void)mtrc::stats::search::range(empty, mtrc::RecordId::from_index(0), 2); }));

	// Negative radius is rejected on both overloads.
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2}, AbsoluteDistance{});
	assert(throws_invalid_argument([&] { (void)mtrc::stats::search::range(space, 1, -1); }));
	assert(throws_invalid_argument([&] { (void)mtrc::stats::search::range(space, space.id(0), -1); }));
	assert(throws_invalid_argument([&] {
		(void)mtrc::stats::search::range(space, space.id(0), 2, mtrc::stats::search::knn_graph{1});
	}));
}

void nan_distances_sort_last_deterministically()
{
	// records[1] is the sentinel; querying value 9.0 makes the pair (9, sentinel) NaN while the
	// other three distances are finite (1, 3, 4). A naive `<` comparator would be an invalid
	// std::sort predicate here; the NaN-safe comparator must place the NaN neighbor strictly
	// last and order the finite neighbors ascending, reproducibly.
	const auto space = mtrc::make_space(std::vector<double>{10.0, 1000.0, 12.0, 13.0}, NaNSentinelDistance{});

	const auto neighbors = mtrc::stats::search::knn(space, 9.0, 4);
	assert(neighbors.size() == 4);
	assert(neighbors[0].id == space.id(0) && neighbors[0].distance == 1.0);
	assert(neighbors[1].id == space.id(2) && neighbors[1].distance == 3.0);
	assert(neighbors[2].id == space.id(3) && neighbors[2].distance == 4.0);
	assert(neighbors[3].id == space.id(1) && std::isnan(neighbors[3].distance));

	// Determinism: the ordering is reproducible across runs (no UB-driven shuffling).
	const auto again = mtrc::stats::search::knn(space, 9.0, 4);
	for (std::size_t index = 0; index < neighbors.size(); ++index) {
		assert(again[index].id == neighbors[index].id);
	}
}

} // namespace

int main()
{
	record_id_range_matches_provider_path();
	duplicate_records_keep_deterministic_ties();
	range_strategy_overloads_match_metric_space_path();
	empty_and_invalid_inputs_are_handled();
	nan_distances_sort_last_deterministically();
	return 0;
}
