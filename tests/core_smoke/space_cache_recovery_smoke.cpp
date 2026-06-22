// Stale cache/index detection and recovery for finite-space representations. After a record-set
// mutation, every derived representation (DistanceTable, CoverTreeIndex, KnnGraphIndex, ExactScanIndex)
// must report itself stale, refuse to silently serve stale values where a checked path is used, and
// rebuild to the current space version while preserving its defining parameters.

#include <cassert>
#include <cstddef>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/space/cache.hpp"
#include "metric/space/distances.hpp"
#include "metric/space/records.hpp"
#include "metric/space/storage/cover_tree_index.hpp"
#include "metric/space/storage/distance_table.hpp"
#include "metric/space/storage/exact_scan_index.hpp"
#include "metric/space/storage/knn_graph_index.hpp"

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

template <typename Callable> static auto throws(Callable &&call) -> bool
{
	try {
		call();
	} catch (...) {
		return true;
	}
	return false;
}

namespace cache = mtrc::space::cache;
namespace distances = mtrc::space::distances;
namespace records = mtrc::space::records;

int main()
{
	auto space = mtrc::make_space(std::vector<int>{0, 10, 20, 30}, AbsoluteDistance{});
	const auto id0 = space.id(0);
	const auto id2 = space.id(2);

	mtrc::space::storage::DistanceTable<decltype(space)> table(space);
	mtrc::space::storage::CoverTreeIndex<decltype(space)> tree(space);
	mtrc::space::storage::KnnGraphIndex<decltype(space)> graph(space, 2);
	mtrc::space::storage::ExactScanIndex<decltype(space)> scan(space);

	// All fresh against the version they were built for.
	assert(!cache::is_stale(table) && !cache::is_stale(tree) && !cache::is_stale(graph) && !cache::is_stale(scan));
	assert(!cache::status(table).stale);
	cache::require_fresh(table); // no throw while fresh
	distances::require_fresh(table);
	assert(distances::checked_value(table, id0, id2) == 20);

	// Mutate the space: every representation built for the old version is now stale.
	const auto mutation = records::insert(space, std::vector<int>{40});
	assert(mutation.changed());
	assert(cache::is_stale(table) && cache::is_stale(tree) && cache::is_stale(graph) && cache::is_stale(scan));
	assert(cache::status(table).stale && distances::is_stale(table));
	assert(distances::status(table).stale);

	// Checked read paths refuse to serve stale values.
	assert(throws([&] { cache::require_fresh(table); }));
	assert(throws([&] { distances::require_fresh(table); }));
	assert(throws([&] { (void)distances::checked_value(table, id0, id2); }));

	// Rebuild in place; refresh() reports that a rebuild happened, then the table is fresh and sees the
	// new record. A second refresh is a no-op.
	assert(cache::refresh(table, space));
	assert(!cache::is_stale(table) && table.record_count() == 5);
	assert(distances::checked_value(table, id0, id2) == 20);
	assert(!cache::refresh(table, space));

	// kNN graph rebuild preserves k; cover tree and exact scan rebuild to the new size.
	assert(cache::refresh(graph, space) && graph.k() == 2 && graph.record_count() == 5);
	assert(cache::refresh(tree, space) && !cache::is_stale(tree) && tree.record_count() == 5);
	assert(cache::refresh(scan, space) && scan.record_count() == 5);

	// rebuild() returns a fresh copy and leaves the original untouched (still stale after a 2nd mutation).
	records::insert(space, std::vector<int>{50});
	auto rebuilt = cache::rebuild(table, space);
	assert(!rebuilt.is_stale() && rebuilt.record_count() == 6);
	assert(cache::is_stale(table)); // original not mutated by rebuild

	// Rebuild preserves the table's lazy/eager mode: a lazy table the caller built on purpose stays
	// lazy after a refresh (no surprise all-pairs materialization).
	auto lazy = distances::lazy_table(space);
	assert(lazy.mode() == mtrc::space::storage::distance_table_mode::lazy);
	assert(lazy.cached_distances() == 0); // nothing materialized up front
	records::insert(space, std::vector<int>{60});
	assert(cache::refresh(lazy, space));
	assert(lazy.mode() == mtrc::space::storage::distance_table_mode::lazy); // mode preserved
	assert(lazy.cached_distances() == 0);                                   // still not materialized
	assert(lazy.record_count() == space.size());

	return 0;
}
