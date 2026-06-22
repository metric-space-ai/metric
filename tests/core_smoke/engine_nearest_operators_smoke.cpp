#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "metric/engine.hpp"

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

int main()
{
	auto space = mtrc::make_space(std::vector<int>{0, 2, 5, 9}, AbsoluteDistance{});

	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);
	const auto id3 = space.id(3);

	std::vector<mtrc::Neighbor<int>> tied_neighbors{{id3, 4}, {id1, 2}, {id0, 4}, {id2, 1}};
	mtrc::core::sort_neighbors(tied_neighbors);
	assert(tied_neighbors[0].id == id2);
	assert(tied_neighbors[1].id == id1);
	assert(tied_neighbors[2].id == id0);
	assert(tied_neighbors[3].id == id3);
	const auto nearest_two = mtrc::core::take_nearest_neighbors(tied_neighbors, 2);
	assert(nearest_two.size() == 2);
	assert(nearest_two[0].id == id2);
	assert(nearest_two[1].id == id1);
	const auto without_id0 = mtrc::core::take_neighbors_excluding_id(tied_neighbors, id0, 2);
	assert(without_id0.size() == 2);
	assert(without_id0[0].id == id2);
	assert(without_id0[1].id == id1);
	assert(mtrc::core::take_neighbors_excluding_id(tied_neighbors, id0, 0).empty());
	const auto nearest_ids = mtrc::core::take_nearest_neighbor_ids(tied_neighbors, 3);
	assert(nearest_ids.size() == 3);
	assert(nearest_ids[0] == id2);
	assert(nearest_ids[1] == id1);
	assert(nearest_ids[2] == id0);
	const auto sorted_ids = mtrc::core::neighbor_ids(tied_neighbors, 4);
	assert(sorted_ids[0] == id2);
	assert(sorted_ids[1] == id1);
	assert(sorted_ids[2] == id0);
	assert(sorted_ids[3] == id3);
	const auto nearest_set = mtrc::core::nearest_neighbor_set(
		std::vector<mtrc::Neighbor<int>>{{id3, 4}, {id1, 2}, {id0, 4}, {id2, 1}}, 2, space.size(),
		"helper_metric_space");
	assert(nearest_set.operator_name == "knn");
	assert(nearest_set.representation == "helper_metric_space");
	assert(nearest_set.record_count == space.size());
	assert(nearest_set.requested_count == 2);
	assert(nearest_set.exact);
	assert(nearest_set.size() == 2);
	assert(nearest_set[0].id == id2);
	assert(nearest_set[1].id == id1);
	const auto range_set = mtrc::core::range_neighbor_set(
		std::vector<mtrc::Neighbor<int>>{{id1, 2}, {id3, 4}, {id0, 4}}, space.size(), "helper_range");
	assert(range_set.operator_name == "range");
	assert(range_set.representation == "helper_range");
	assert(range_set.record_count == space.size());
	assert(range_set.requested_count == range_set.size());
	assert(range_set.exact);
	assert(range_set.size() == 3);
	assert(range_set[0].id == id1);
	assert(range_set[1].id == id0);
	assert(range_set[2].id == id3);
	const auto approximate_set = mtrc::core::make_neighbor_set(
		std::vector<mtrc::Neighbor<int>>{{id0, 4}}, space.size(), 5, "custom", "custom_representation", false);
	assert(approximate_set.operator_name == "custom");
	assert(approximate_set.representation == "custom_representation");
	assert(approximate_set.record_count == space.size());
	assert(approximate_set.requested_count == 5);
	assert(!approximate_set.exact);
	assert(approximate_set.size() == 1);
	assert(approximate_set[0].id == id0);
	const auto remapped_source_ids = std::vector<mtrc::RecordId>{id3, id2, id1, id0};
	const auto projected_source_ids = mtrc::core::source_ids_for_neighbors(
		remapped_source_ids, nearest_two, [&space](mtrc::RecordId id) { return space.position_of(id); },
		"neighbor source position is outside lineage");
	assert((projected_source_ids == std::vector<mtrc::RecordId>{id1, id2}));
	bool rejected_bad_neighbor_source_position = false;
	try {
		(void)mtrc::core::source_ids_for_neighbors(
			std::vector<mtrc::RecordId>{id0}, nearest_two, [&space](mtrc::RecordId id) { return space.position_of(id); },
			"neighbor source position is outside lineage");
	} catch (const std::invalid_argument &) {
		rejected_bad_neighbor_source_position = true;
	}
	assert(rejected_bad_neighbor_source_position);
	assert(mtrc::core::neighbor_id_position(tied_neighbors, id0) == 2);
	assert(mtrc::core::neighbor_rank(tied_neighbors, id0) == 3);
	assert(mtrc::core::contains_neighbor_id(tied_neighbors, id3));
	assert(mtrc::core::neighbor_distance_or_throw(tied_neighbors, id3, "missing neighbor") == 4);
	const auto missing_id = mtrc::RecordId::from_index(99);
	assert(mtrc::core::neighbor_id_position(tied_neighbors, missing_id) == tied_neighbors.size());
	assert(mtrc::core::neighbor_rank(tied_neighbors, missing_id) == tied_neighbors.size() + 1);
	assert(!mtrc::core::contains_neighbor_id(tied_neighbors, missing_id));
	const auto without_missing = mtrc::core::take_neighbors_excluding_id(tied_neighbors, missing_id, 2);
	assert(without_missing.size() == 2);
	assert(without_missing[0].id == id2);
	assert(without_missing[1].id == id1);
	bool missing_neighbor_thrown = false;
	try {
		(void)mtrc::core::neighbor_distance_or_throw(tied_neighbors, missing_id, "missing neighbor");
	} catch (const std::invalid_argument &) {
		missing_neighbor_thrown = true;
	}
	assert(missing_neighbor_thrown);
	const auto overlapping_neighbors = std::vector<mtrc::Neighbor<int>>{{id0, 7}, {id3, 1}, {missing_id, 0}};
	assert(mtrc::core::neighbor_id_overlap_count(tied_neighbors, overlapping_neighbors) == 2);
	const auto built_candidates = mtrc::core::neighbor_candidates<int>(
		space.size(), [&space](std::size_t position) { return space.id(position); },
		[&space, id0](mtrc::RecordId id, std::size_t) { return space.distance(id0, id); });
	assert(built_candidates.size() == space.size());
	assert(built_candidates[0].id == id0);
	assert(built_candidates[0].distance == 0);
	assert(built_candidates[3].id == id3);
	assert(built_candidates[3].distance == 9);
	const auto filtered_candidates = mtrc::core::neighbor_candidates_if<int>(
		space.size(), [&space](std::size_t position) { return space.id(position); },
		[&space, id0](mtrc::RecordId id, std::size_t) { return space.distance(id0, id); },
		[id0](mtrc::RecordId id, std::size_t position) { return id != id0 && position != 3; });
	assert(filtered_candidates.size() == 2);
	assert(filtered_candidates[0].id == id1);
	assert(filtered_candidates[1].id == id2);
	const auto radius_candidates = mtrc::core::neighbor_candidates_within<int>(
		space.size(), [&space](std::size_t position) { return space.id(position); },
		[&space, id0](mtrc::RecordId id, std::size_t) { return space.distance(id0, id); }, 5);
	assert(radius_candidates.size() == 3);
	assert(radius_candidates[0].id == id0);
	assert(radius_candidates[1].id == id1);
	assert(radius_candidates[2].id == id2);
	const auto filtered_radius_candidates = mtrc::core::neighbor_candidates_within_if<int>(
		space.size(), [&space](std::size_t position) { return space.id(position); },
		[&space, id0](mtrc::RecordId id, std::size_t) { return space.distance(id0, id); }, 5,
		[id0](mtrc::RecordId id, std::size_t) { return id != id0; });
	assert(filtered_radius_candidates.size() == 2);
	assert(filtered_radius_candidates[0].id == id1);
	assert(filtered_radius_candidates[1].id == id2);

	const auto exact = mtrc::stats::search::knn(space, 4, 3);
	static_assert(std::is_same<decltype(exact)::distance_type, int>::value);
	assert(exact.operator_name == "knn");
	assert(exact.representation == "metric_space");
	assert(exact.record_count == space.size());
	assert(exact.requested_count == 3);
	assert(exact.size() == 3);
	assert(exact[0].id == id2);
	assert(exact[0].distance == 1);
	assert(exact[1].id == id1);
	assert(exact[1].distance == 2);
	assert(exact[2].id == id0);
	assert(exact[2].distance == 4);

	const auto record_neighbors = mtrc::stats::search::knn(space, id0, 2);
	assert(record_neighbors.size() == 2);
	assert(record_neighbors[0].id == id1);
	assert(record_neighbors[0].distance == 2);
	assert(record_neighbors[1].id == id2);
	assert(record_neighbors[1].distance == 5);

	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);
	const auto cached = mtrc::stats::search::knn(matrix, id0, 2);
	assert(cached.representation == "pairwise_distances");
	assert(cached.size() == 2);
	assert(cached[0].id == id1);
	assert(cached[0].distance == record_neighbors[0].distance);
	assert(cached[1].id == id2);
	assert(cached[1].distance == record_neighbors[1].distance);

	const auto close_records = mtrc::stats::search::range(space, 4, 2);
	assert(close_records.operator_name == "range");
	assert(close_records.representation == "metric_space");
	assert(close_records.size() == 2);
	assert(close_records[0].id == id2);
	assert(close_records[0].distance == 1);
	assert(close_records[1].id == id1);
	assert(close_records[1].distance == 2);

	const auto close_cached = mtrc::stats::search::range(matrix, id0, 5);
	assert(close_cached.operator_name == "range");
	assert(close_cached.size() == 2);
	assert(close_cached[0].id == id1);
	assert(close_cached[1].id == id2);

	mtrc::space::storage::CoverTreeIndex<decltype(space)> tree(space);
	const auto indexed = mtrc::stats::search::knn(tree, 4, 2);
	assert(indexed.representation == "neighbor_index");
	assert(indexed.size() == 2);
	assert(indexed[0].id == id2);
	assert(indexed[0].distance == 1);
	assert(indexed[1].id == id1);
	assert(indexed[1].distance == 2);

	return 0;
}
