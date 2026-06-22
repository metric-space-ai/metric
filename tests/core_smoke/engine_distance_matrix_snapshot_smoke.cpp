#include <cassert>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <vector>

#include "metric/engine.hpp"

struct DirectedDistanceSnapshot {
	auto operator()(int lhs, int rhs) const -> int
	{
		if (lhs <= rhs) {
			return rhs - lhs;
		}
		return 2 * (lhs - rhs);
	}
};

namespace mtrc::core {
template <> struct metric_traits<DirectedDistanceSnapshot> {
	static constexpr auto law = metric_law::distance;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;

	static auto cache_key(const DirectedDistanceSnapshot &) -> std::string { return "directed-distance-snapshot:v1"; }
};
} // namespace mtrc::core

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

template <typename Snapshot>
auto assert_eager_snapshot_contract(const Snapshot &snapshot, const std::vector<mtrc::RecordId> &ids,
									std::size_t expected_version) -> void
{
	assert(snapshot.kind == mtrc::space::storage::representation_kind::distance_table);
	assert(snapshot.exact == mtrc::space::storage::exactness::exact);
	assert(snapshot.materialized == mtrc::space::storage::materialization::materialized);
	assert(snapshot.updates == mtrc::space::storage::update_mode::snapshot);
	assert(snapshot.mode == mtrc::space::storage::distance_table_mode::eager);
	assert(snapshot.built_for_version == expected_version);
	assert(snapshot.record_count == ids.size());
	assert(snapshot.cached_distances == ids.size() * ids.size());
	assert(snapshot.dense_distance_slots == ids.size() * ids.size());
	assert(snapshot.metric_key == "directed-distance-snapshot:v1");
	assert(snapshot.cache_key.find("metric=directed-distance-snapshot:v1") != std::string::npos);
	assert(snapshot.cache_key.find("ids=0,1,2") != std::string::npos);
	assert(snapshot.source_record_ids == ids);
	assert(snapshot.distances.size() == ids.size() * ids.size());
	for (std::size_t position = 0; position < ids.size(); ++position) {
		assert(snapshot.contains(ids[position]));
		assert(snapshot.position_of(ids[position]) == position);
	}
}

int main()
{
	const std::vector<int> records{0, 2, 5};
	auto space = mtrc::make_space(records, DirectedDistanceSnapshot{});
	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);
	const std::vector<mtrc::RecordId> ids{id0, id1, id2};

	const std::vector<mtrc::RecordId> helper_ids{id0, id1};
	std::vector<std::optional<int>> helper_cache(4);
	helper_cache[0] = 0;
	helper_cache[3] = 2;
	const auto helper_cells = mtrc::space::storage::distance_table_snapshot_cells<int>(
		helper_ids, 2, 2,
		[&helper_cache](std::size_t lhs, std::size_t rhs) -> const std::optional<int> & {
			return helper_cache[lhs * 2 + rhs];
		});
	assert(helper_cells.size() == 2);
	assert(helper_cells[0].lhs == id0);
	assert(helper_cells[0].rhs == id0);
	assert(helper_cells[0].distance == 0);
	assert(helper_cells[1].lhs == id1);
	assert(helper_cells[1].rhs == id1);
	assert(helper_cells[1].distance == 2);
	assert(mtrc::space::storage::distance_table_snapshot_cell_position(helper_cells, id1, id1) == 1);
	assert(mtrc::space::storage::distance_table_snapshot_cell_position(helper_cells, id1, id0) ==
		   helper_cells.size());
	assert(mtrc::space::storage::has_distance_table_snapshot_cell(helper_cells, id0, id0));
	assert(!mtrc::space::storage::has_distance_table_snapshot_cell(helper_cells, id1, id0));
	assert(mtrc::space::storage::distance_table_snapshot_cell_distance_or_throw(
			   helper_cells, id1, id1, "missing helper cell") == 2);
	assert_out_of_range([&helper_cells, id1, id0]() {
		(void)mtrc::space::storage::distance_table_snapshot_cell_distance_or_throw(
			helper_cells, id1, id0, "missing helper cell");
	});
	const auto empty_helper_cells = mtrc::space::storage::distance_table_snapshot_cells<int>(
		helper_ids, 2, 0,
		[&helper_cache](std::size_t, std::size_t) -> const std::optional<int> & {
			static const std::optional<int> empty;
			return empty;
		});
	assert(empty_helper_cells.empty());
	bool rejected_incomplete_helper_ids = false;
	try {
		(void)mtrc::space::storage::distance_table_snapshot_cells<int>(
			helper_ids, 3, 0,
			[&helper_cache](std::size_t, std::size_t) -> const std::optional<int> & {
				return helper_cache[0];
			});
	} catch (const mtrc::RepresentationError &) {
		rejected_incomplete_helper_ids = true;
	}
	assert(rejected_incomplete_helper_ids);

	auto eager = mtrc::space::storage::matrix(space);
	const auto eager_snapshot = eager.snapshot();
	assert_eager_snapshot_contract(eager_snapshot, ids, space.version());
	assert(eager_snapshot.distance(id0, id2) == 5);
	assert(eager_snapshot.distance(id2, id0) == 10);
	assert(eager_snapshot.distance(id0, id2) != eager_snapshot.distance(id2, id0));

	const auto free_snapshot = mtrc::space::storage::snapshot(eager);
	assert(free_snapshot.cache_key == eager_snapshot.cache_key);
	assert(free_snapshot.distance(id1, id2) == 3);
	assert(free_snapshot.distance(id2, id1) == 6);

	const auto copied_snapshot = eager_snapshot;
	space.replace(id2, 7);
	const auto inserted_id = space.insert(9);
	assert(copied_snapshot.built_for_version == 0);
	assert(copied_snapshot.source_record_ids == ids);
	assert(copied_snapshot.distance(id0, id2) == 5);
	assert(copied_snapshot.distance(id2, id0) == 10);
	assert(!copied_snapshot.contains(inserted_id));
	assert_out_of_range([&copied_snapshot, inserted_id]() { (void)copied_snapshot.position_of(inserted_id); });
	assert_out_of_range([&copied_snapshot, id0, inserted_id]() { (void)copied_snapshot.distance(id0, inserted_id); });

	auto lazy = mtrc::space::storage::matrix(space, mtrc::space::storage::distance_table_mode::lazy);
	assert(lazy.cached_distances() == 0);
	assert(lazy.distance(id0, id2) == 7);
	assert(lazy.distance(id2, id0) == 14);
	const auto lazy_stats_before = lazy.stats();
	assert(lazy_stats_before.hits == 0);
	assert(lazy_stats_before.misses == 2);
	assert(lazy_stats_before.fill_ratio == 2.0 / 16.0);

	const auto lazy_snapshot = lazy.snapshot();
	assert(lazy.cached_distances() == 2);
	assert(lazy.stats().hits == lazy_stats_before.hits);
	assert(lazy.stats().misses == lazy_stats_before.misses);
	assert(lazy_snapshot.materialized == mtrc::space::storage::materialization::lazy);
	assert(lazy_snapshot.mode == mtrc::space::storage::distance_table_mode::lazy);
	assert(lazy_snapshot.record_count == 4);
	assert(lazy_snapshot.cached_distances == 2);
	assert(lazy_snapshot.dense_distance_slots == 16);
	assert(lazy_snapshot.source_record_ids == std::vector<mtrc::RecordId>({id0, id1, id2, inserted_id}));
	assert(lazy_snapshot.distances.size() == 2);
	assert(lazy_snapshot.has_distance(id0, id2));
	assert(lazy_snapshot.has_distance(id2, id0));
	assert(!lazy_snapshot.has_distance(id0, inserted_id));
	assert(lazy_snapshot.distance(id0, id2) == 7);
	assert(lazy_snapshot.distance(id2, id0) == 14);
	assert_out_of_range([&lazy_snapshot, id0, inserted_id]() { (void)lazy_snapshot.distance(id0, inserted_id); });

	return 0;
}
