#include <cassert>
#include <cstddef>
#include <stdexcept>
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
		}
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

	assert(id0.index() == 0);
	assert(id1.index() == 1);
	assert(id2.index() == 2);
	assert(id3.index() == 3);
	assert(space.position_of(id2) == 2);
	assert(space.record(id3) == 9);
	assert(space.distance(id0, id2) == 5);
	assert(space.distance(id2, id0) == 10);

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
	assert(lazy.distance(id0, id2) == 5);
	assert(lazy.distance(id2, id0) == 10);
	assert(lazy.cached_distances() == 2);
	assert(lazy.distance(id0, id2) == 5);
	assert(lazy.stats().hits == 1);
	assert(lazy.stats().misses == 2);
	assert(lazy.source_record_ids() == initial_ids);

	const auto initial_version = space.version();
	space.replace(id2, 7);
	assert(space.version() == initial_version + 1);
	assert(space.contains(id2));
	assert(space.position_of(id2) == 2);
	assert(space.record(id2) == 7);
	assert(eager.is_stale());
	assert(lazy.is_stale());
	assert(eager.distance(id0, id2) == 5);
	assert(space.distance(id0, id2) == 7);

	auto replaced = mtrc::space::storage::matrix(space);
	assert(replaced.source_record_ids() == initial_ids);
	assert(replaced.cache_key() != eager.cache_key());
	assert(replaced.distance(id0, id2) == 7);
	assert(replaced.distance(id2, id0) == 14);

	assert(space.erase(id1));
	assert(!space.contains(id1));
	assert(space.position_of(id2) == 1);
	assert(space.position_of(id3) == 2);
	const auto inserted_id = space.insert(13);
	assert(inserted_id.index() == 4);
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

	return 0;
}
