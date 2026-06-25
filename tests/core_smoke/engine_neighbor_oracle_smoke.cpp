#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct PositionCountingProvider {
	using distance_type = int;

	std::vector<mtrc::RecordId> ids;
	std::vector<int> records;
	mutable std::size_t position_lookups{};
	mutable std::size_t id_distances{};
	mutable std::size_t position_distances{};

	auto distance(mtrc::RecordId, mtrc::RecordId) const -> int
	{
		++id_distances;
		return -1;
	}
	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position) const -> int
	{
		++position_distances;
		const auto lhs = records[lhs_position];
		const auto rhs = records[rhs_position];
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
	auto record_count() const -> std::size_t { return ids.size(); }
	auto id(std::size_t position) const -> mtrc::RecordId { return ids[position]; }
	auto position_of(mtrc::RecordId id) const -> std::size_t
	{
		++position_lookups;
		for (std::size_t position = 0; position < ids.size(); ++position) {
			if (ids[position] == id) {
				return position;
			}
		}
		throw std::out_of_range("record id is outside the counting provider");
	}
	auto contains(mtrc::RecordId id) const -> bool
	{
		for (const auto candidate : ids) {
			if (candidate == id) {
				return true;
			}
		}
		return false;
	}
	auto version() const -> std::size_t { return 1; }
	auto is_stale() const -> bool { return false; }
};

namespace mtrc::core {
template <> struct metric_traits<AbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};
} // namespace mtrc::core

namespace {

using neighbor_type = mtrc::Neighbor<int>;

template <typename Space>
auto oracle_for_record_query(const Space &space, const typename Space::record_type &query, std::size_t count)
	-> std::vector<neighbor_type>
{
	std::vector<neighbor_type> neighbors;
	neighbors.reserve(space.size());
	for (std::size_t position = 0; position < space.size(); ++position) {
		const auto id = space.id(position);
		neighbors.push_back(neighbor_type{id, space.metric()(query, space.record(id))});
	}
	return mtrc::core::take_nearest_neighbors(std::move(neighbors), count);
}

template <typename Space>
auto oracle_for_id_query(const Space &space, mtrc::RecordId query_id, std::size_t count) -> std::vector<neighbor_type>
{
	std::vector<neighbor_type> neighbors;
	neighbors.reserve(space.size() > 0 ? space.size() - 1 : 0);
	for (std::size_t position = 0; position < space.size(); ++position) {
		const auto id = space.id(position);
		if (id == query_id) {
			continue;
		}
		neighbors.push_back(neighbor_type{id, space.distance(query_id, id)});
	}
	return mtrc::core::take_nearest_neighbors(std::move(neighbors), count);
}

template <typename Result>
auto assert_matches_oracle(const Result &actual, const std::vector<neighbor_type> &expected,
						   const char *expected_representation) -> void
{
	assert(actual.operator_name == "knn");
	assert(actual.exact);
	assert(actual.representation == expected_representation);
	assert(actual.provenance.route_kind ==
		   mtrc::default_search_route_kind_for_representation(expected_representation));
	assert(actual.provenance.domain_kind ==
		   mtrc::default_result_domain_kind_for_representation(expected_representation));
	assert(actual.size() == expected.size());
	assert(actual.requested_count >= actual.size());
	for (std::size_t index = 0; index < expected.size(); ++index) {
		assert(actual[index].id == expected[index].id);
		assert(actual[index].distance == expected[index].distance);
	}
}

} // namespace

int main()
{
	const auto records = std::vector<int>{0, 4, 8, 10, 12};
	const auto space = mtrc::make_space(records, AbsoluteDistance{});
	const int query = 6;
	const std::size_t count = 4;
	const auto expected_query = oracle_for_record_query(space, query, count);
	assert(expected_query[0].id == space.id(1));
	assert(expected_query[1].id == space.id(2));
	assert(expected_query[0].distance == expected_query[1].distance);

	assert_matches_oracle(mtrc::find_neighbors(space, query, count), expected_query, "metric_space");
	assert_matches_oracle(mtrc::find_neighbors(space, query, mtrc::count{count}), expected_query, "metric_space");
	assert_matches_oracle(mtrc::find_neighbors(space, query, count, mtrc::stats::search::exact_scan{}),
						  expected_query, "exact_scan_index");
	assert_matches_oracle(mtrc::find_neighbors(space, query, count, mtrc::stats::search::brute_force{}),
						  expected_query, "exact_scan_index");
	assert_matches_oracle(mtrc::find_neighbors(space, query, count, mtrc::stats::search::cover_tree{}), expected_query,
						  "cover_tree_index");
	assert_matches_oracle(mtrc::find_neighbors(space, query, mtrc::count{count}, mtrc::stats::search::cover_tree{}),
						  expected_query, "cover_tree_index");
	assert_matches_oracle(mtrc::find_neighbors(space, query, count, mtrc::stats::search::knn_graph(count)),
						  expected_query, "knn_graph_index");
	assert_matches_oracle(mtrc::find_neighbors(space, query, count, mtrc::space::storage::exact()), expected_query,
						  "metric_space");
	assert_matches_oracle(mtrc::find_neighbors(space, query, count, mtrc::space::storage::using_implicit()),
						  expected_query, "implicit");
	assert_matches_oracle(mtrc::find_neighbors(space, query, count, mtrc::space::storage::using_cover_tree()),
						  expected_query, "cover_tree_index");
	assert_matches_oracle(mtrc::find_neighbors(space, query, count, mtrc::space::storage::using_knn_graph(count)),
						  expected_query, "knn_graph_index");

	const auto query_id = space.id(2);
	const std::size_t id_count = 3;
	const auto expected_id = oracle_for_id_query(space, query_id, id_count);
	assert(expected_id[0].id == space.id(3));
	assert(expected_id[1].id == space.id(1));
	assert(expected_id[2].id == space.id(4));
	assert(expected_id[1].distance == expected_id[2].distance);

	assert_matches_oracle(mtrc::find_neighbors(space, query_id, id_count), expected_id, "metric_space");
	assert_matches_oracle(mtrc::find_neighbors(space, query_id, mtrc::count{id_count}), expected_id,
						  "metric_space");
	assert_matches_oracle(mtrc::find_neighbors(space, query_id, id_count, mtrc::stats::search::exact_scan{}),
						  expected_id, "exact_scan_index");
	assert_matches_oracle(mtrc::find_neighbors(space, query_id, id_count, mtrc::stats::search::brute_force{}),
						  expected_id, "exact_scan_index");
	assert_matches_oracle(mtrc::find_neighbors(space, query_id, id_count, mtrc::stats::search::distance_table{}),
						  expected_id, "distance_table");
	assert_matches_oracle(
		mtrc::find_neighbors(space, query_id, mtrc::count{id_count}, mtrc::stats::search::distance_table{}),
		expected_id, "distance_table");
	assert_matches_oracle(mtrc::find_neighbors(space, query_id, id_count, mtrc::stats::search::knn_graph(id_count)),
						  expected_id, "knn_graph_index");
	assert_matches_oracle(mtrc::find_neighbors(space, query_id, id_count, mtrc::space::storage::exact()), expected_id,
						  "metric_space");
	assert_matches_oracle(mtrc::find_neighbors(space, query_id, id_count, mtrc::space::storage::materialized()),
						  expected_id, "distance_table");
	assert_matches_oracle(mtrc::find_neighbors(space, query_id, id_count, mtrc::space::storage::using_distance_table()),
						  expected_id, "distance_table");
	assert_matches_oracle(mtrc::find_neighbors(space, query_id, id_count, mtrc::space::storage::using_cover_tree()),
						  expected_id, "cover_tree_index");
	assert_matches_oracle(mtrc::find_neighbors(space, query_id, id_count, mtrc::space::storage::using_knn_graph(id_count)),
						  expected_id, "knn_graph_index");

	const auto zero_neighbors = mtrc::find_neighbors(space, query_id, 0, mtrc::space::storage::using_cover_tree());
	assert(zero_neighbors.empty());
	assert(zero_neighbors.requested_count == 0);
	assert(zero_neighbors.representation == "cover_tree_index");

	PositionCountingProvider provider{{space.id(0), space.id(1), space.id(2), space.id(3), space.id(4)}, records};
	static_assert(mtrc::PairwiseDistances_v<PositionCountingProvider>);
	const auto provider_neighbors = mtrc::stats::search::knn(provider, query_id, id_count);
	assert_matches_oracle(provider_neighbors, expected_id, "pairwise_distances");
	assert(provider.position_lookups == 1);
	assert(provider.position_distances == records.size() - 1);
	assert(provider.id_distances == 0);

	return 0;
}
