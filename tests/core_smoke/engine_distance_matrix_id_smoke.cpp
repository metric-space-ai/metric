#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

#include "metric/engine.hpp"

struct DirectedDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		if (lhs <= rhs) {
			return rhs - lhs;
		}
		return 2 * (lhs - rhs);
	}
};

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

template <typename Callable> auto assert_stale_representation(Callable &&call) -> void
{
	bool rejected = false;
	try {
		call();
	} catch (const mtrc::StaleRepresentationError &) {
		rejected = true;
	}
	assert(rejected);
}

template <typename Matrix, typename Space>
auto assert_matrix_matches_space_order(const Matrix &matrix, const Space &space) -> void
{
	assert(matrix.record_count() == space.size());
	for (std::size_t lhs = 0; lhs < space.size(); ++lhs) {
		const auto lhs_id = space.id(lhs);
		assert(matrix.id(lhs) == lhs_id);
		assert(matrix.position_of(lhs_id) == lhs);
		assert(matrix.contains(lhs_id));
		for (std::size_t rhs = 0; rhs < space.size(); ++rhs) {
			const auto rhs_id = space.id(rhs);
			assert(matrix.distance(lhs_id, rhs_id) == space.distance(lhs_id, rhs_id));
			assert(matrix.distance_at_position(lhs, rhs) == space.distance(lhs_id, rhs_id));
		}
	}
}

template <typename Space>
auto assert_space_order(const Space &space, const std::vector<mtrc::RecordId> &expected_ids) -> void
{
	assert(space.size() == expected_ids.size());
	for (std::size_t position = 0; position < expected_ids.size(); ++position) {
		const auto id = expected_ids[position];
		assert(space.id(position) == id);
		assert(space.contains(id));
		assert(space.position_of(id) == position);
	}
}

int main()
{
	const std::vector<int> records{0, 2, 5, 9};
	auto space = mtrc::make_space(records, DirectedDistance{});
	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);
	const auto id3 = space.id(3);
	const std::vector<mtrc::RecordId> initial_ids{id0, id1, id2, id3};
	const std::unordered_set<mtrc::RecordId> hashed_ids(initial_ids.begin(), initial_ids.end());

	assert(id0.index() == 0);
	assert(id1.index() == 1);
	assert(id2.index() == 2);
	assert(id3.index() == 3);
	assert(hashed_ids.count(id2) == 1);
	assert(hashed_ids.count(mtrc::RecordId::from_index(99)) == 0);
	assert_space_order(space, initial_ids);
	assert(space.position_of(id2) == 2);
	assert(space.record(id3) == 9);
	assert(space.distance(id0, id2) == 5);
	assert(space.distance(id2, id0) == 10);

	const auto live = mtrc::space::storage::implicit(space);
	assert(live.distance_at_position(0, 2) == 5);
	assert(live.distance_at_position(2, 0) == 10);
	assert(live.record_at_position(2) == 5);

	auto eager = mtrc::space::storage::matrix(space);
	assert_matrix_matches_space_order(eager, space);
	assert(eager.distance(id0, id2) == 5);
	assert(eager.distance(id2, id0) == 10);
	assert(eager.distance(id0, id2) != eager.distance(id2, id0));
	assert(eager.source_record_ids() == initial_ids);
	assert(eager.diagnostics().source_record_ids == initial_ids);
	assert(eager.diagnostics().cached_distances == records.size() * records.size());
	assert(eager.diagnostics().dense_distance_slots == records.size() * records.size());
	assert(eager.cache_key().find("ids=0,1,2,3") != std::string::npos);

	auto lazy = mtrc::space::storage::matrix(space, mtrc::space::storage::distance_table_mode::lazy);
	assert(lazy.cached_distances() == 0);
	assert(lazy.dense_distance_slots() == records.size() * records.size());
	assert(lazy.stats().fill_ratio == 0.0);
	const auto lazy_initial_diagnostics = lazy.diagnostics();
	assert(lazy_initial_diagnostics.cached_distances == 0);
	assert(lazy_initial_diagnostics.dense_distance_slots == records.size() * records.size());
	assert(lazy_initial_diagnostics.memory_bytes_estimate < eager.diagnostics().memory_bytes_estimate);
	const auto lazy_initial_snapshot = lazy.snapshot();
	assert(lazy_initial_snapshot.cached_distances == 0);
	assert(lazy_initial_snapshot.dense_distance_slots == records.size() * records.size());
	assert(lazy_initial_snapshot.distances.empty());
	assert(lazy.distance(id0, id2) == 5);
	assert(lazy.distance(id2, id0) == 10);
	assert(lazy.cached_distances() == 2);
	assert(lazy.distance(id0, id2) == 5);
	assert(lazy.stats().hits == 1);
	assert(lazy.stats().misses == 2);
	assert(lazy.stats().fill_ratio == 2.0 / static_cast<double>(records.size() * records.size()));
	assert(lazy.source_record_ids() == initial_ids);
	const auto lazy_directed_snapshot = lazy.snapshot();
	assert(lazy_directed_snapshot.cached_distances == 2);
	assert(lazy_directed_snapshot.dense_distance_slots == records.size() * records.size());
	assert(lazy_directed_snapshot.distances.size() == 2);
	assert(lazy_directed_snapshot.has_distance(id0, id2));
	assert(lazy_directed_snapshot.has_distance(id2, id0));
	assert(lazy_directed_snapshot.distance(id0, id2) == 5);
	assert(lazy_directed_snapshot.distance(id2, id0) == 10);
	assert(lazy_directed_snapshot.distance(id0, id2) != lazy_directed_snapshot.distance(id2, id0));

	constexpr std::size_t large_record_count = 16384;
	std::vector<int> large_records(large_record_count);
	for (std::size_t index = 0; index < large_records.size(); ++index) {
		large_records[index] = static_cast<int>(index);
	}
	auto large_space = mtrc::make_space(large_records, DirectedDistance{});
	const auto large_first_id = large_space.id(0);
	const auto large_middle_id = large_space.id(large_record_count / 2);
	const auto large_last_id = large_space.id(large_record_count - 1);
	const auto expected_middle_to_last = static_cast<int>((large_record_count - 1) - (large_record_count / 2));
	auto large_lazy = mtrc::space::storage::matrix(large_space, mtrc::space::storage::distance_table_mode::lazy);
	assert(large_lazy.record_count() == large_record_count);
	assert(large_lazy.cached_distances() == 0);
	assert(large_lazy.dense_distance_slots() == large_record_count * large_record_count);
	assert(large_lazy.diagnostics().dense_distance_slots == large_record_count * large_record_count);
	assert(large_lazy.memory_bytes_estimate() <
		   mtrc::space::storage::estimate_distance_table_memory_bytes<int>(large_record_count) / 16);
	assert(large_lazy.contains(large_last_id));
	assert(large_lazy.position_of(large_middle_id) == large_record_count / 2);
	assert(large_lazy.distance(large_first_id, large_last_id) == static_cast<int>(large_record_count - 1));
	assert(large_lazy.distance(large_last_id, large_first_id) == static_cast<int>(2 * (large_record_count - 1)));
	assert(large_lazy.distance(large_middle_id, large_last_id) == expected_middle_to_last);
	const auto large_lazy_snapshot = large_lazy.snapshot();
	assert(large_lazy_snapshot.record_count == large_record_count);
	assert(large_lazy_snapshot.cached_distances == 3);
	assert(large_lazy_snapshot.dense_distance_slots == large_record_count * large_record_count);
	assert(large_lazy_snapshot.distances.size() == 3);
	assert(large_lazy_snapshot.contains(large_last_id));
	assert(large_lazy_snapshot.position_of(large_middle_id) == large_record_count / 2);
	assert(large_lazy_snapshot.has_distance(large_first_id, large_last_id));
	assert(large_lazy_snapshot.has_distance(large_last_id, large_first_id));
	assert(large_lazy_snapshot.has_distance(large_middle_id, large_last_id));
	assert(!large_lazy_snapshot.has_distance(large_last_id, large_middle_id));
	assert(large_lazy_snapshot.distance(large_first_id, large_last_id) == static_cast<int>(large_record_count - 1));
	assert(large_lazy_snapshot.distance(large_last_id, large_first_id) ==
		   static_cast<int>(2 * (large_record_count - 1)));
	assert_out_of_range([&large_lazy_snapshot, large_last_id, large_middle_id]() {
		(void)large_lazy_snapshot.distance(large_last_id, large_middle_id);
	});
	auto copied_large_lazy_snapshot = large_lazy_snapshot;
	assert(copied_large_lazy_snapshot.contains(large_last_id));
	assert(copied_large_lazy_snapshot.position_of(large_middle_id) == large_record_count / 2);
	assert(copied_large_lazy_snapshot.distance(large_middle_id, large_last_id) == expected_middle_to_last);
	auto moved_large_lazy_snapshot = std::move(copied_large_lazy_snapshot);
	assert(moved_large_lazy_snapshot.contains(large_first_id));
	assert(moved_large_lazy_snapshot.distance(large_last_id, large_first_id) ==
		   static_cast<int>(2 * (large_record_count - 1)));

	const auto initial_version = space.version();
	space.replace(id2, 7);
	assert(space.version() == initial_version + 1);
	assert(space.contains(id2));
	assert(space.position_of(id2) == 2);
	assert(space.record(id2) == 7);
	assert_space_order(space, initial_ids);
	assert(eager.is_stale());
	assert(lazy.is_stale());
	assert(eager.distance(id0, id2) == 5);
	assert(space.distance(id0, id2) == 7);
	const auto lazy_cached_before_stale_read = lazy.cached_distances();
	const auto lazy_misses_before_stale_read = lazy.stats().misses;
	assert_stale_representation([&lazy, id0, id1]() { (void)lazy.distance(id0, id1); });
	assert(lazy.cached_distances() == lazy_cached_before_stale_read);
	assert(lazy.stats().misses == lazy_misses_before_stale_read);

	auto replaced = mtrc::space::storage::matrix(space);
	assert(replaced.source_record_ids() == initial_ids);
	assert(replaced.cache_key() != eager.cache_key());
	assert(replaced.distance(id0, id2) == 7);
	assert(replaced.distance(id2, id0) == 14);

	assert(space.erase(id1));
	assert(!space.contains(id1));
	assert_space_order(space, std::vector<mtrc::RecordId>{id0, id2, id3});
	assert(space.position_of(id2) == 1);
	assert(space.position_of(id3) == 2);
	const auto inserted_id = space.insert(13);
	assert(inserted_id.index() == 4);
	assert_space_order(space, std::vector<mtrc::RecordId>{id0, id2, id3, inserted_id});
	assert(space.position_of(inserted_id) == 3);
	assert(space.record(inserted_id) == 13);

	const std::vector<mtrc::RecordId> refreshed_ids{id0, id2, id3, inserted_id};
	auto refreshed = mtrc::space::storage::matrix(space);
	assert_matrix_matches_space_order(refreshed, space);
	assert(refreshed.source_record_ids() == refreshed_ids);
	assert(refreshed.diagnostics().source_record_ids == refreshed_ids);
	assert(refreshed.cache_key().find("ids=0,2,3,4") != std::string::npos);
	assert(refreshed.distance(id2, inserted_id) == 6);
	assert(refreshed.distance(inserted_id, id2) == 12);
	assert(eager.source_record_ids() == initial_ids);
	assert(eager.contains(id1));
	assert(!eager.contains(inserted_id));
	assert(eager.is_stale());

	assert_out_of_range([&space, id1]() { (void)space.position_of(id1); });
	assert_out_of_range([&eager, inserted_id]() { (void)eager.position_of(inserted_id); });

	const auto replace_after_insert_version = space.version();
	space.replace(id3, 11);
	assert(space.version() == replace_after_insert_version + 1);
	assert_space_order(space, refreshed_ids);
	assert(space.record(id3) == 11);

	assert(space.erase(id0));
	assert(!space.contains(id0));
	assert_space_order(space, std::vector<mtrc::RecordId>{id2, id3, inserted_id});
	assert(space.position_of(id2) == 0);
	assert(space.position_of(id3) == 1);
	assert(space.position_of(inserted_id) == 2);

	const auto late_id = space.insert(17);
	assert(late_id.index() == 5);
	assert_space_order(space, std::vector<mtrc::RecordId>{id2, id3, inserted_id, late_id});
	assert(space.record(late_id) == 17);

	const auto replace_shifted_version = space.version();
	space.replace(inserted_id, 15);
	assert(space.version() == replace_shifted_version + 1);
	assert_space_order(space, std::vector<mtrc::RecordId>{id2, id3, inserted_id, late_id});
	assert(space.record(inserted_id) == 15);
	assert(space.distance(id2, inserted_id) == 8);
	assert(space.distance(inserted_id, id2) == 16);

	return 0;
}
