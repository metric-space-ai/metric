// Coverage for the lineage-preserving partition operations on a finite metric space
// (mtrc::space::split / split_if / merge / select_subspace). These prove that carving a space into
// sub-spaces and recombining spaces preserves both RecordId lineage and metric validity, including after
// the parent space has been mutated so that position_of(id) != id.index().

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/space/partition.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

template <typename Callable> static auto throws(Callable &&call) -> bool
{
	try {
		call();
	} catch (...) {
		return true;
	}
	return false;
}

int main()
{
	auto space = mtrc::make_space(std::vector<int>{0, 10, 20, 30, 40}, AbsoluteDistance{});
	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);
	const auto id3 = space.id(3);
	const auto id4 = space.id(4);

	// Mutate the parent so positions and id indices diverge: after erasing id1, id2..id4 shift down a slot
	// while keeping their original RecordIds.
	assert(space.erase(id1));
	assert(space.size() == 4);
	assert(space.position_of(id2) == 1);
	assert(space.position_of(id4) == 3);

	// select_subspace keeps the requested RecordIds as lineage and rebuilds records in the requested order.
	const auto sub = mtrc::select_subspace(space, std::vector<mtrc::RecordId>{id3, id0});
	assert(sub.space.size() == 2);
	assert(sub.source_ids.size() == 2);
	assert(sub.source_ids[0] == id3);
	assert(sub.source_ids[1] == id0);
	assert(sub.space.record(sub.space.id(0)) == space.record(id3)); // 30
	assert(sub.space.record(sub.space.id(1)) == space.record(id0)); // 0
	// metric validity: a distance inside the sub-space matches the same pair in the parent.
	assert(sub.space.distance(sub.space.id(0), sub.space.id(1)) == space.distance(id3, id0)); // |30-0| = 30

	// A sub-space can never silently reference a record that no longer exists.
	assert(throws([&space, id1]() { (void)mtrc::select_subspace(space, std::vector<mtrc::RecordId>{id1}); }));

	// split into explicit groups; each part preserves lineage and metric validity.
	const auto parts = mtrc::split(space, std::vector<std::vector<mtrc::RecordId>>{{id0, id2}, {id3, id4}});
	assert(parts.size() == 2);
	assert(parts[0].space.size() == 2);
	assert(parts[1].space.size() == 2);
	assert(parts[0].source_ids[0] == id0);
	assert(parts[0].source_ids[1] == id2);
	assert(parts[1].source_ids[0] == id3);
	for (const auto &part : parts) {
		for (std::size_t position = 0; position < part.source_ids.size(); ++position) {
			assert(part.space.record(part.space.id(position)) == space.record(part.source_ids[position]));
		}
	}

	// split_if partitions by a predicate over the record; lineage preserved on both sides.
	const auto partitioned = mtrc::split_if(space, [](int value) { return value >= 20; });
	const auto &large = partitioned.first;
	const auto &small = partitioned.second;
	assert(large.space.size() == 3); // 20, 30, 40
	assert(small.space.size() == 1); // 0
	assert(small.source_ids[0] == id0);
	assert(large.space.size() + small.space.size() == space.size());
	for (std::size_t position = 0; position < large.source_ids.size(); ++position) {
		assert(large.space.record(large.space.id(position)) == space.record(large.source_ids[position]));
		assert(space.record(large.source_ids[position]) >= 20);
	}

	// merge concatenates records in input order and records, per merged record, the origin space and id.
	auto first = mtrc::make_space(std::vector<int>{1, 2}, AbsoluteDistance{});
	auto second = mtrc::make_space(std::vector<int>{100}, AbsoluteDistance{});
	const auto merged = mtrc::merge(first, second);
	assert(merged.space.size() == 3);
	assert(merged.lineage.size() == 3);
	assert(merged.lineage[0].source_index == 0 && merged.lineage[0].source_id == first.id(0));
	assert(merged.lineage[1].source_index == 0 && merged.lineage[1].source_id == first.id(1));
	assert(merged.lineage[2].source_index == 1 && merged.lineage[2].source_id == second.id(0));
	assert(merged.space.record(merged.space.id(0)) == first.record(first.id(0)));  // 1
	assert(merged.space.record(merged.space.id(2)) == second.record(second.id(0))); // 100
	// metric validity across the merge boundary.
	assert(merged.space.distance(merged.space.id(0), merged.space.id(2)) == AbsoluteDistance{}(1, 100)); // 99

	// list merge keeps source_index aligned with argument order.
	const auto merged_three = mtrc::merge(std::vector<const decltype(first) *>{&first, &second, &first});
	assert(merged_three.space.size() == 5);
	assert(merged_three.lineage[4].source_index == 2);
	assert(merged_three.lineage[4].source_id == first.id(1));

	return 0;
}
