// Sub-space / merge lineage workflow: select_subspace and split keep parent RecordId lineage even
// after the parent has been mutated; the lineage lookups (parent_record_id / merge_origin / table
// views) map a derived RecordId back to its source; the alignment guard refuses to return lineage from
// a derived space that was itself structurally mutated; and merge_checked rejects incompatible metrics.

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/space/lineage.hpp"
#include "metric/space/partition.hpp"
#include "metric/space/records.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

// A parameterised metric whose cache key depends on a runtime scale: two instances with different
// scales are incompatible and must not be silently merged.
struct ScaledAbsoluteDistance {
	int scale{1};
	auto operator()(int lhs, int rhs) const -> int { return scale * (lhs > rhs ? lhs - rhs : rhs - lhs); }
};

namespace mtrc::core {
template <> struct metric_traits<ScaledAbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
	static auto cache_key(const ScaledAbsoluteDistance &metric) -> std::string
	{
		return "ScaledAbsoluteDistance:" + std::to_string(metric.scale);
	}
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

static auto test_subspace_lineage_after_parent_mutation() -> void
{
	auto space = mtrc::make_space(std::vector<int>{0, 10, 20, 30, 40}, AbsoluteDistance{});
	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);
	const auto id3 = space.id(3);

	// Mutate the parent so id index and position diverge.
	mtrc::space::records::erase(space, std::vector<mtrc::RecordId>{id1});
	assert(space.position_of(id3) == 2);

	// Sub-space records keep their parent lineage; the lookups recover it.
	auto sub = mtrc::select_subspace(space, std::vector<mtrc::RecordId>{id3, id0, id2});
	assert(mtrc::space::subspace_is_aligned(sub));
	assert(mtrc::parent_record_id(sub, sub.space.id(0)) == id3);
	assert(mtrc::parent_record_id(sub, sub.space.id(1)) == id0);
	assert(mtrc::parent_record_id(sub, sub.space.id(2)) == id2);
	const auto table = mtrc::space::subspace_lineage(sub);
	assert(table.size() == 3);
	assert(table[0].local_id == sub.space.id(0) && table[0].parent_id == id3);
	assert(table[2].parent_id == id2);
	// Records carried over correctly.
	assert(sub.space.record(sub.space.id(0)) == space.record(id3));

	// Mutating the PARENT after the sub-space exists cannot corrupt the sub-space (it is an independent
	// copy): lineage still resolves the same way.
	mtrc::space::records::insert(space, std::vector<int>{99});
	assert(mtrc::parent_record_id(sub, sub.space.id(0)) == id3);

	// Mutating the SUB-SPACE structurally breaks alignment; lineage lookups refuse rather than lie.
	const auto local0 = sub.space.id(0);
	sub.space.erase(sub.space.id(1));
	assert(!mtrc::space::subspace_is_aligned(sub));
	assert(throws([&] { (void)mtrc::parent_record_id(sub, local0); }));
	assert(throws([&] { (void)mtrc::space::subspace_lineage(sub); }));
}

static auto test_split_lineage() -> void
{
	auto space = mtrc::make_space(std::vector<int>{0, 10, 20, 30}, AbsoluteDistance{});
	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);
	const auto id3 = space.id(3);

	const auto parts = mtrc::split(space, std::vector<std::vector<mtrc::RecordId>>{{id0, id2}, {id1, id3}});
	assert(parts.size() == 2);
	assert(mtrc::parent_record_id(parts[0], parts[0].space.id(0)) == id0);
	assert(mtrc::parent_record_id(parts[0], parts[0].space.id(1)) == id2);
	assert(mtrc::parent_record_id(parts[1], parts[1].space.id(1)) == id3);

	const auto partitioned = mtrc::split_if(space, [](int value) { return value >= 20; });
	assert(partitioned.first.space.size() == 2 && partitioned.second.space.size() == 2);
	assert(mtrc::parent_record_id(partitioned.first, partitioned.first.space.id(0)) == id2);
}

static auto test_merge_lineage_and_compatibility() -> void
{
	auto first = mtrc::make_space(std::vector<int>{1, 2}, AbsoluteDistance{});
	auto second = mtrc::make_space(std::vector<int>{100}, AbsoluteDistance{});

	const auto merged = mtrc::merge(first, second);
	assert(merged.space.size() == 3);
	assert(mtrc::space::merged_is_aligned(merged));
	const auto origin0 = mtrc::merge_origin(merged, merged.space.id(0));
	const auto origin2 = mtrc::merge_origin(merged, merged.space.id(2));
	assert(origin0.source_index == 0 && origin0.source_id == first.id(0));
	assert(origin2.source_index == 1 && origin2.source_id == second.id(0));
	const auto table = mtrc::space::merged_lineage(merged);
	assert(table.size() == 3 && table[2].source_index == 1 && table[2].source_id == second.id(0));

	// merge_checked accepts compatible metrics and rejects incompatible ones.
	auto a = mtrc::make_space(std::vector<int>{1}, ScaledAbsoluteDistance{2});
	auto b = mtrc::make_space(std::vector<int>{5}, ScaledAbsoluteDistance{2});
	auto c = mtrc::make_space(std::vector<int>{9}, ScaledAbsoluteDistance{3});
	const auto compatible = mtrc::merge_checked(a, b);
	assert(compatible.space.size() == 2);
	assert(mtrc::space::metrics_compatible(std::vector<const decltype(a) *>{&a, &b}));
	assert(!mtrc::space::metrics_compatible(std::vector<const decltype(a) *>{&a, &c}));
	assert(throws([&] { (void)mtrc::merge_checked(a, c); }));

	// merge_checked refuses a degenerate request with no usable (non-null) input space rather than
	// fabricating an empty space backed by a default-constructed metric.
	using space_type = decltype(a);
	assert(throws([] { (void)mtrc::merge_checked(std::vector<const space_type *>{}); }));
	assert(throws([] { (void)mtrc::merge_checked(std::vector<const space_type *>{nullptr, nullptr}); }));
}

int main()
{
	test_subspace_lineage_after_parent_mutation();
	test_split_lineage();
	test_merge_lineage_and_compatibility();
	return 0;
}
