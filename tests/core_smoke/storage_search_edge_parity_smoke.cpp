#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "metric/core/metric_traits.hpp"
#include "metric/core/neighbor.hpp"
#include "metric/core/result.hpp"
#include "metric/core/metric_space.hpp"
#include "metric/record/id.hpp"
#include "metric/space/storage/cover_tree_index.hpp"
#include "metric/space/storage/distance_table.hpp"
#include "metric/space/storage/exact_scan_index.hpp"
#include "metric/space/storage/implicit.hpp"
#include "metric/space/storage/knn_graph_index.hpp"
#include "metric/space/storage/policy.hpp"
#include "metric/stats/search/neighbors.hpp"
#include "metric/stats/search/nearest.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

namespace mtrc::core {
template <> struct metric_traits<AbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};
} // namespace mtrc::core

namespace {

using expected_neighbor = std::pair<std::size_t, int>;

template <typename Result>
auto assert_neighbors(const Result &actual, const std::vector<expected_neighbor> &expected,
					  std::size_t expected_record_count, std::size_t expected_requested_count,
					  const std::string &expected_representation) -> void
{
	assert(actual.operator_name == "knn");
	assert(actual.exact);
	assert(actual.record_count == expected_record_count);
	assert(actual.requested_count == expected_requested_count);
	assert(actual.representation == expected_representation);
	assert(actual.size() == expected.size());
	for (std::size_t index = 0; index < expected.size(); ++index) {
		assert(actual[index].id.index() == expected[index].first);
		assert(actual[index].distance == expected[index].second);
	}
}

template <typename Callable> auto assert_out_of_range(Callable &&call) -> void
{
	bool rejected = false;
	try {
		call();
	} catch (const std::out_of_range &) {
		rejected = true;
	}
	assert(rejected);
}

auto empty_space_queries_are_consistent() -> void
{
	const auto space = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
	const std::vector<expected_neighbor> none{};

	assert_neighbors(mtrc::find_neighbors(space, 10, 4), none, 0, 4, "metric_space");
	assert_neighbors(mtrc::find_neighbors(space, 10, 4, mtrc::stats::search::exact_scan{}), none, 0, 4,
					 "exact_scan_index");
	assert_neighbors(mtrc::find_neighbors(space, 10, 4, mtrc::stats::search::brute_force{}), none, 0, 4,
					 "exact_scan_index");
	assert_neighbors(mtrc::find_neighbors(space, 10, 4, mtrc::stats::search::cover_tree{}), none, 0, 4,
					 "cover_tree_index");
	assert_neighbors(mtrc::find_neighbors(space, 10, 4, mtrc::stats::search::knn_graph(4)), none, 0, 4,
					 "knn_graph_index");
	assert_neighbors(mtrc::find_neighbors(space, 10, 4, mtrc::space::storage::using_implicit()), none, 0, 4,
					 "implicit");
	assert_neighbors(mtrc::find_neighbors(space, 10, 4, mtrc::space::storage::using_cover_tree()), none, 0, 4,
					 "cover_tree_index");
	assert_neighbors(mtrc::find_neighbors(space, 10, 4, mtrc::space::storage::using_knn_graph(4)), none, 0, 4,
					 "knn_graph_index");

	const auto eager = mtrc::space::storage::matrix(space);
	assert(eager.record_count() == 0);
	assert(eager.dense_distance_slots() == 0);
	assert(eager.cached_distances() == 0);
	assert(eager.stats().fill_ratio == 1.0);
	assert(eager.snapshot().distances.empty());

	const auto lazy = mtrc::space::storage::matrix(space, mtrc::space::storage::distance_table_mode::lazy);
	assert(lazy.record_count() == 0);
	assert(lazy.cached_distances() == 0);
	assert(lazy.snapshot().distances.empty());

	const auto graph = mtrc::space::storage::knn_graph(space, 4);
	assert(graph.record_count() == 0);
	assert(graph.edge_count() == 0);
	assert_neighbors(mtrc::stats::search::knn(graph, 10, 4), none, 0, 4, "neighbor_index");

	const auto scan = mtrc::space::storage::exact_scan(space);
	assert(scan.record_count() == 0);
	assert_neighbors(mtrc::stats::search::knn(scan, 10, 4), none, 0, 4, "neighbor_index");

	const auto tree = mtrc::space::storage::cover_tree(space);
	assert(tree.record_count() == 0);
	assert_neighbors(mtrc::stats::search::knn(tree, 10, 4), none, 0, 4, "neighbor_index");

	const auto invalid_id = mtrc::RecordId::from_index(0);
	assert_out_of_range([&space, invalid_id]() { (void)mtrc::find_neighbors(space, invalid_id, 0); });
	assert_out_of_range([&space, invalid_id]() {
		(void)mtrc::find_neighbors(space, invalid_id, 0, mtrc::stats::search::distance_table{});
	});
	assert_out_of_range([&space, invalid_id]() {
		(void)mtrc::find_neighbors(space, invalid_id, 0, mtrc::stats::search::knn_graph(0));
	});
	assert_out_of_range([&space, invalid_id]() {
		(void)mtrc::find_neighbors(space, invalid_id, 0, mtrc::space::storage::using_cover_tree());
	});
}

auto singleton_id_queries_exclude_self() -> void
{
	const auto space = mtrc::make_space(std::vector<int>{7}, AbsoluteDistance{});
	const auto only_id = space.id(0);
	const std::vector<expected_neighbor> value_expected{{0, 0}};
	const std::vector<expected_neighbor> id_expected{};

	assert_neighbors(mtrc::find_neighbors(space, 7, 5), value_expected, 1, 5, "metric_space");
	assert_neighbors(mtrc::find_neighbors(space, 7, 5, mtrc::stats::search::exact_scan{}), value_expected, 1, 5,
					 "exact_scan_index");
	assert_neighbors(mtrc::find_neighbors(space, 7, 5, mtrc::stats::search::cover_tree{}), value_expected, 1, 5,
					 "cover_tree_index");
	assert_neighbors(mtrc::find_neighbors(space, 7, 5, mtrc::stats::search::knn_graph(0)), value_expected, 1, 5,
					 "knn_graph_index");

	assert_neighbors(mtrc::find_neighbors(space, only_id, 5), id_expected, 1, 5, "metric_space");
	assert_neighbors(mtrc::find_neighbors(space, only_id, 5, mtrc::stats::search::exact_scan{}), id_expected, 1, 5,
					 "exact_scan_index");
	assert_neighbors(mtrc::find_neighbors(space, only_id, 5, mtrc::stats::search::distance_table{}), id_expected, 1,
					 5, "distance_table");
	assert_neighbors(mtrc::find_neighbors(space, only_id, 5, mtrc::stats::search::knn_graph(0)), id_expected, 1, 5,
					 "knn_graph_index");
	assert_neighbors(mtrc::find_neighbors(space, only_id, 5, mtrc::space::storage::using_cover_tree()), id_expected,
					 1, 5, "cover_tree_index");
	assert_neighbors(mtrc::find_neighbors(space, only_id, 5, mtrc::space::storage::using_knn_graph(0)), id_expected,
					 1, 5, "knn_graph_index");

	const auto table = mtrc::space::storage::matrix(space);
	assert(table.record_count() == 1);
	assert(table.cached_distances() == 1);
	assert(table.distance(only_id, only_id) == 0);
	assert(table.snapshot().distances.size() == 1);

	const auto graph = mtrc::space::storage::knn_graph(space, 5);
	assert(graph.record_count() == 1);
	assert(graph.edge_count() == 0);
	assert(graph.neighbors(only_id).empty());
}

auto duplicate_records_keep_zero_distance_and_id_ties() -> void
{
	const auto space = mtrc::make_space(std::vector<int>{0, 0, 2, 2}, AbsoluteDistance{});
	const auto id0 = space.id(0);
	const auto id2 = space.id(2);
	const std::vector<expected_neighbor> value_zero_expected{{0, 0}, {1, 0}, {2, 2}, {3, 2}};
	const std::vector<expected_neighbor> id0_expected{{1, 0}, {2, 2}, {3, 2}};
	const std::vector<expected_neighbor> id2_expected{{3, 0}, {0, 2}, {1, 2}};

	assert_neighbors(mtrc::find_neighbors(space, 0, 99), value_zero_expected, 4, 99, "metric_space");
	assert_neighbors(mtrc::find_neighbors(space, 0, 99, mtrc::stats::search::exact_scan{}), value_zero_expected, 4,
					 99, "exact_scan_index");
	assert_neighbors(mtrc::find_neighbors(space, 0, 99, mtrc::stats::search::cover_tree{}), value_zero_expected, 4,
					 99, "cover_tree_index");
	assert_neighbors(mtrc::find_neighbors(space, 0, 99, mtrc::stats::search::knn_graph(0)), value_zero_expected, 4,
					 99, "knn_graph_index");
	assert_neighbors(mtrc::find_neighbors(space, 0, 99, mtrc::space::storage::using_implicit()),
					 value_zero_expected, 4, 99, "implicit");
	assert_neighbors(mtrc::find_neighbors(space, 0, 99, mtrc::space::storage::using_cover_tree()),
					 value_zero_expected, 4, 99, "cover_tree_index");
	assert_neighbors(mtrc::find_neighbors(space, 0, 99, mtrc::space::storage::using_knn_graph(0)),
					 value_zero_expected, 4, 99, "knn_graph_index");

	assert_neighbors(mtrc::find_neighbors(space, id0, 99), id0_expected, 4, 99, "metric_space");
	assert_neighbors(mtrc::find_neighbors(space, id0, 99, mtrc::stats::search::exact_scan{}), id0_expected, 4, 99,
					 "exact_scan_index");
	assert_neighbors(mtrc::find_neighbors(space, id0, 99, mtrc::stats::search::distance_table{}), id0_expected, 4,
					 99, "distance_table");
	assert_neighbors(mtrc::find_neighbors(space, id0, 99, mtrc::stats::search::knn_graph(0)), id0_expected, 4, 99,
					 "knn_graph_index");
	assert_neighbors(mtrc::find_neighbors(space, id0, 99, mtrc::space::storage::materialized()), id0_expected, 4,
					 99, "distance_table");
	assert_neighbors(mtrc::find_neighbors(space, id0, 99, mtrc::space::storage::using_cover_tree()), id0_expected,
					 4, 99, "cover_tree_index");
	assert_neighbors(mtrc::find_neighbors(space, id0, 99, mtrc::space::storage::using_knn_graph(0)), id0_expected,
					 4, 99, "knn_graph_index");

	assert_neighbors(mtrc::find_neighbors(space, id2, 99), id2_expected, 4, 99, "metric_space");
	assert_neighbors(mtrc::find_neighbors(space, id2, 99, mtrc::stats::search::exact_scan{}), id2_expected, 4, 99,
					 "exact_scan_index");
	assert_neighbors(mtrc::find_neighbors(space, id2, 99, mtrc::stats::search::distance_table{}), id2_expected, 4,
					 99, "distance_table");
	assert_neighbors(mtrc::find_neighbors(space, id2, 99, mtrc::space::storage::using_cover_tree()), id2_expected,
					 4, 99, "cover_tree_index");
	assert_neighbors(mtrc::find_neighbors(space, id2, 99, mtrc::space::storage::using_knn_graph(0)), id2_expected,
					 4, 99, "knn_graph_index");

	const auto eager = mtrc::space::storage::matrix(space);
	const auto lazy = mtrc::space::storage::matrix(space, mtrc::space::storage::distance_table_mode::lazy);
	assert_neighbors(mtrc::stats::search::knn(eager, id2, 99), id2_expected, 4, 99, "pairwise_distances");
	assert_neighbors(mtrc::stats::search::knn(lazy, id2, 99), id2_expected, 4, 99, "pairwise_distances");
	assert(lazy.cached_distances() == 3);
	assert(eager.snapshot().distance(id0, space.id(1)) == 0);
	assert(eager.snapshot().distance(id2, space.id(3)) == 0);

	bool rejected_dense_limit = false;
	try {
		(void)mtrc::space::storage::matrix(
			space, mtrc::space::storage::distance_table_options{mtrc::space::storage::distance_table_mode::eager, 3});
	} catch (const mtrc::RepresentationError &) {
		rejected_dense_limit = true;
	}
	assert(rejected_dense_limit);
}

auto index_value_queries_match_brute_force_scan_contract() -> void
{
	const auto space = mtrc::make_space(std::vector<int>{0, 4, 8, 12}, AbsoluteDistance{});
	const std::vector<expected_neighbor> expected{{1, 2}, {2, 2}, {0, 6}, {3, 6}};

	assert_neighbors(mtrc::find_neighbors(space, 6, 4), expected, 4, 4, "metric_space");
	assert_neighbors(mtrc::find_neighbors(space, 6, 4, mtrc::stats::search::exact_scan{}), expected, 4, 4,
					 "exact_scan_index");
	assert_neighbors(mtrc::find_neighbors(space, 6, 4, mtrc::stats::search::cover_tree{}), expected, 4, 4,
					 "cover_tree_index");
	assert_neighbors(mtrc::find_neighbors(space, 6, 4, mtrc::stats::search::knn_graph(4)), expected, 4, 4,
					 "knn_graph_index");
}

} // namespace

int main()
{
	empty_space_queries_are_consistent();
	singleton_id_queries_exclude_self();
	duplicate_records_keep_zero_distance_and_id_ties();
	index_value_queries_match_brute_force_scan_contract();
	return 0;
}
