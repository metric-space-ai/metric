// User-facing mtrc::space API: builder/factory construction, batch record-set mutation with stable
// RecordIds, pairwise distance access (value / row / symmetric pair iteration), and the query helpers
// (nearest / k_nearest / within) checked for parity against an independent brute-force computation and
// against the underlying stats::search. Error cases are exercised explicitly.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/space/builder.hpp"
#include "metric/space/distances.hpp"
#include "metric/space/query.hpp"
#include "metric/space/records.hpp"
#include "metric/space/storage/knn_graph_index.hpp"
#include "metric/space/streaming.hpp"
#include "metric/stats/search/nearest.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct CountingAbsoluteDistance {
	int *calls{};

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
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

namespace records = mtrc::space::records;
namespace distances = mtrc::space::distances;
namespace query = mtrc::space::query;

static auto test_builder() -> void
{
	// Build via the staged builder with opt-in validation.
	auto space = mtrc::space::SpaceBuilder<int, AbsoluteDistance>(AbsoluteDistance{})
					 .add_all(std::vector<int>{0, 10, 20})
					 .add(30)
					 .require_non_empty()
					 .require_true_metric()
					 .require_unique_records()
					 .build();
	assert(space.size() == 4);
	assert(space.distance(space.id(0), space.id(3)) == 30);

	// Factory deduces the metric type.
	auto via_factory = mtrc::space::space_builder<int>(AbsoluteDistance{}).add(1).add(2).build();
	assert(via_factory.size() == 2);

	// build_described carries a name.
	auto described =
		mtrc::space::SpaceBuilder<int, AbsoluteDistance>(AbsoluteDistance{}).add(7).named("demo").build_described();
	assert(described.space.size() == 1);
	assert(described.metadata.contains("name"));
	assert(described.metadata["name"].get<std::string>() == "demo");

	// Error: empty space when require_non_empty.
	assert(throws([] {
		(void)mtrc::space::SpaceBuilder<int, AbsoluteDistance>(AbsoluteDistance{}).require_non_empty().build();
	}));
	// Error: duplicate records when require_unique_records.
	assert(throws([] {
		(void)mtrc::space::SpaceBuilder<int, AbsoluteDistance>(AbsoluteDistance{})
			.add(5)
			.add(5)
			.require_unique_records()
			.build();
	}));
}

static auto test_records_mutation_and_stable_ids() -> void
{
	auto space = mtrc::make_space(std::vector<int>{0, 10, 20}, AbsoluteDistance{});
	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);

	// Batch insert returns the new ids; old ids stay valid and keep their records (stable identity).
	const auto inserted = records::insert(space, std::vector<int>{30, 40});
	assert(inserted.kind == records::mutation_kind::insert);
	assert(inserted.old_size == 3 && inserted.new_size == 5 && inserted.count() == 2);
	assert(inserted.changed());
	const auto id3 = inserted.affected_ids[0];
	const auto id4 = inserted.affected_ids[1];
	assert(space.record(id0) == 0 && space.record(id2) == 20);
	assert(space.record(id3) == 30 && space.record(id4) == 40);

	// Validation helpers.
	assert(records::contains_all(space, std::vector<mtrc::RecordId>{id0, id4}));
	const auto missing = records::missing_ids(space, std::vector<mtrc::RecordId>{id0, mtrc::RecordId::from_index(99)});
	assert(missing.size() == 1 && missing[0] == mtrc::RecordId::from_index(99));
	const auto positions = records::positions_of(space, std::vector<mtrc::RecordId>{id0, id2});
	assert(positions[0] == 0 && positions[1] == 2);

	// Batch replace preserves identity; missing id is rejected and nothing is replaced.
	const auto replaced = records::replace(space, std::vector<records::edit<int>>{{id0, 1}, {id2, 21}});
	assert(replaced.kind == records::mutation_kind::replace && replaced.count() == 2);
	assert(space.record(id0) == 1 && space.record(id2) == 21);
	assert(throws([&] {
		(void)records::replace(space, std::vector<records::edit<int>>{{mtrc::RecordId::from_index(99), 0}});
	}));
	assert(space.record(id0) == 1); // unchanged by the failed replace

	// Stable RecordId after erase: id2/id3/id4 shift position but keep identity.
	const auto erased = records::erase(space, std::vector<mtrc::RecordId>{id1});
	assert(erased.count() == 1 && erased.new_size == 4);
	assert(!space.contains(id1));
	assert(space.contains(id2) && space.position_of(id2) == 1); // position moved, id preserved
	assert(space.record(id2) == 21);

	// Lenient erase reports missing; strict erase throws on missing.
	const auto lenient = records::erase(space, std::vector<mtrc::RecordId>{id0, mtrc::RecordId::from_index(99)});
	assert(lenient.count() == 1 && lenient.missing_count() == 1);
	assert(throws([&] { (void)records::erase_strict(space, std::vector<mtrc::RecordId>{id1}); }));

	// Strict batch ops reject a duplicate id in the request (the all-or-nothing contract would otherwise
	// report the duplicate as "missing"); nothing is mutated when they throw.
	const auto size_before_dup = space.size();
	assert(throws([&] { (void)records::erase_strict(space, std::vector<mtrc::RecordId>{id2, id2}); }));
	assert(space.size() == size_before_dup && space.contains(id2));
	assert(throws([&] {
		(void)records::replace(space, std::vector<records::edit<int>>{{id2, 100}, {id2, 200}});
	}));
	assert(space.record(id2) == 21); // unchanged by the rejected duplicate-target replace
}

static auto test_streaming_append_batch() -> void
{
	int metric_calls = 0;
	auto space = mtrc::make_space(std::vector<int>{0, 10, 20}, CountingAbsoluteDistance{&metric_calls});
	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);

	assert(space.erase(id1));
	const auto version_before = space.version();

	std::vector<mtrc::space::ingest_progress> progress;
	const auto report = mtrc::space::append_batch(
		space, std::vector<int>{30, 40},
		[&](const mtrc::space::ingest_progress &item) { progress.push_back(item); },
		[](const mtrc::space::ingest_progress &) { return false; });

	assert(report.requested == 2 && report.appended == 2 && report.complete());
	assert(report.old_size == 2 && report.new_size == 4);
	assert(report.version_before == version_before);
	assert(report.version_after == version_before + 2);
	assert(report.distance_evaluations == 0);
	assert(!report.materialized_dense_all_pairs);
	assert(metric_calls == 0);

	const auto id3 = report.appended_ids[0];
	const auto id4 = report.appended_ids[1];
	assert(id3 == mtrc::RecordId::from_index(3));
	assert(id4 == mtrc::RecordId::from_index(4));
	assert(space.record(id0) == 0 && space.record(id2) == 20);
	assert(space.record(id3) == 30 && space.record(id4) == 40);
	assert(!space.contains(id1)); // erased id was not reused by streaming append

	assert(progress.size() == 2);
	assert(progress[0].accepted == 1 && !progress[0].cancelled);
	assert(progress[1].accepted == 2 && progress[1].complete());
	assert(progress[1].version_current == report.version_after);
	assert(progress[1].last_id == id4);

	progress.clear();
	const auto cancelled = mtrc::space::append_batch(
		space, std::vector<int>{50, 60, 70},
		[&](const mtrc::space::ingest_progress &item) { progress.push_back(item); },
		[](const mtrc::space::ingest_progress &item) { return item.accepted == 2; });

	assert(cancelled.requested == 3 && cancelled.appended == 2);
	assert(cancelled.cancelled && !cancelled.complete());
	assert(cancelled.distance_evaluations == 0);
	assert(!cancelled.materialized_dense_all_pairs);
	assert(metric_calls == 0);
	assert(cancelled.appended_ids[0] == mtrc::RecordId::from_index(5));
	assert(cancelled.appended_ids[1] == mtrc::RecordId::from_index(6));
	assert(!space.contains(mtrc::RecordId::from_index(7)));
	assert(space.next_record_id() == 7);
	assert(cancelled.version_after == cancelled.version_before + 2);
	assert(!progress.empty() && progress.back().cancelled && progress.back().accepted == 2);
}

static auto test_knn_graph_refresh_after_streaming_append_is_bounded() -> void
{
	int metric_calls = 0;
	std::vector<int> values;
	values.reserve(50);
	for (int value = 0; value < 500; value += 10) {
		values.push_back(value);
	}
	auto space = mtrc::make_space(values, CountingAbsoluteDistance{&metric_calls});
	auto index = mtrc::space::storage::knn_graph(space, 2);
	const auto initial_size = space.size();
	assert(index.record_count() == initial_size);
	assert(index.build_distance_evaluations() == initial_size * (initial_size - 1));
	assert(metric_calls == static_cast<int>(index.build_distance_evaluations()));

	metric_calls = 0;
	const auto id0 = space.id(0);
	const auto append = mtrc::space::append_batch(space, std::vector<int>{1, 2, 498});
	assert(append.appended == 3);
	assert(append.distance_evaluations == 0);
	assert(metric_calls == 0);
	assert(index.is_stale());

	const auto refresh = mtrc::space::refresh_after_append(index, append);
	const auto expected_old_row_updates = initial_size * append.appended;
	const auto expected_new_rows = append.appended * (space.size() - 1);
	assert(refresh.refreshed);
	assert(!refresh.rebuild_required);
	assert(refresh.exact_rows_updated);
	assert(refresh.appended == append.appended);
	assert(refresh.old_row_update_distance_evaluations == expected_old_row_updates);
	assert(refresh.appended_row_distance_evaluations == expected_new_rows);
	assert(refresh.distance_evaluations == expected_old_row_updates + expected_new_rows);
	assert(metric_calls == static_cast<int>(refresh.distance_evaluations));
	assert(!index.is_stale());
	assert(index.record_count() == space.size());
	assert(index.version() == space.version());
	assert(index.maintenance_distance_evaluations() == refresh.distance_evaluations);
	assert(index.total_distance_evaluations() == index.build_distance_evaluations() + refresh.distance_evaluations);

	const auto &updated_old_row = index.neighbors(id0);
	assert(updated_old_row.size() == 2);
	assert(updated_old_row[0].id == append.appended_ids[0]);
	assert(updated_old_row[0].distance == 1);
	assert(updated_old_row[1].id == append.appended_ids[1]);
	assert(updated_old_row[1].distance == 2);

	const auto &new_row = index.neighbors(append.appended_ids[0]);
	assert(new_row.size() == 2);
	assert(new_row[0].id == id0);
	assert(new_row[0].distance == 1);
	assert(new_row[1].id == append.appended_ids[1]);
	assert(new_row[1].distance == 1);

	metric_calls = 0;
	space.replace(id0, -100);
	const auto refused = index.refresh_after_append();
	assert(!refused.refreshed);
	assert(refused.rebuild_required);
	assert(refused.reason.find("rebuild required") != std::string::npos);
	assert(metric_calls == 0);
}

static auto test_distances_access() -> void
{
	auto space = mtrc::make_space(std::vector<int>{0, 10, 20, 30}, AbsoluteDistance{});
	const auto id0 = space.id(0);
	const auto id2 = space.id(2);

	// value() over the live space and over a materialized table agree with the metric.
	auto table = distances::materialize(space);
	assert(distances::value(space, id0, id2) == 20);
	assert(distances::value(table, id0, id2) == 20);
	assert(distances::checked_value(table, id0, id2) == 20);

	// row() from one record, over live space and provider, matches the metric exactly.
	const AbsoluteDistance metric;
	const auto live_row = distances::row(space, id0);
	const auto table_row = distances::row(table, id0);
	assert(live_row.size() == 4 && table_row.size() == 4);
	for (std::size_t index = 0; index < space.size(); ++index) {
		const auto id = space.id(index);
		assert(live_row[index].id == id);
		assert(live_row[index].distance == metric(space.record(id0), space.record(id)));
		assert(table_row[index].distance == live_row[index].distance);
	}

	// Symmetric pair iteration visits each unordered pair once: C(4,2) = 6.
	const auto pairs = distances::pairs(space);
	assert(pairs.size() == 6);
	int pair_sum = 0;
	for (const auto &pair : pairs) {
		assert(pair.lhs.index() < pair.rhs.index());
		assert(pair.distance == metric(space.record(pair.lhs), space.record(pair.rhs)));
		pair_sum += pair.distance;
	}
	int for_each_sum = 0;
	distances::for_each_pair(space, [&](mtrc::RecordId, mtrc::RecordId, int distance) { for_each_sum += distance; });
	assert(pair_sum == for_each_sum);
	assert(pair_sum == 100); // 10+20+30+10+20+10

	// Pair iteration over a materialized provider matches the live-space result exactly.
	const auto table_pairs = distances::pairs(table);
	assert(table_pairs.size() == pairs.size());
	int table_pair_sum = 0;
	for (std::size_t index = 0; index < table_pairs.size(); ++index) {
		assert(table_pairs[index].lhs == pairs[index].lhs && table_pairs[index].rhs == pairs[index].rhs);
		assert(table_pairs[index].distance == pairs[index].distance);
		table_pair_sum += table_pairs[index].distance;
	}
	int table_for_each_sum = 0;
	distances::for_each_pair(table, [&](mtrc::RecordId, mtrc::RecordId, int distance) { table_for_each_sum += distance; });
	assert(table_pair_sum == pair_sum && table_for_each_sum == pair_sum);

	// Explicit pair collection is guarded before reserve() and before metric calls.
	assert(throws([&] { (void)distances::pairs(space, distances::pair_collection_options{5}); }));
	const auto unbounded_pairs = distances::pairs(space, distances::pair_collection_options{0});
	assert(unbounded_pairs.size() == pairs.size());
	int guarded_calls = 0;
	auto large_records = std::vector<int>(1500);
	auto large_space = mtrc::make_space(large_records, CountingAbsoluteDistance{&guarded_calls});
	assert(throws([&] { (void)distances::pairs(large_space); }));
	assert(guarded_calls == 0);

	// status(): live is non-materialized + fresh; eager table is materialized + fresh.
	const auto live_status = distances::status(space);
	assert(!live_status.materialized && live_status.exact && !live_status.stale && live_status.record_count == 4);
	const auto table_status = distances::status(table);
	assert(table_status.materialized && table_status.exact && !table_status.stale);

	// Error: row from a record id that is not in the space.
	assert(throws([&] { (void)distances::row(space, mtrc::RecordId::from_index(99)); }));
}

// Independent brute-force k nearest (excluding the query record itself) for parity checks.
static auto brute_force_knn(const std::vector<int> &records_values, std::size_t query_position, std::size_t k)
	-> std::vector<std::size_t>
{
	const AbsoluteDistance metric;
	std::vector<std::pair<int, std::size_t>> scored;
	for (std::size_t index = 0; index < records_values.size(); ++index) {
		if (index == query_position) {
			continue;
		}
		scored.emplace_back(metric(records_values[query_position], records_values[index]), index);
	}
	std::stable_sort(scored.begin(), scored.end(),
					 [](const auto &lhs, const auto &rhs) { return lhs.first < rhs.first; });
	std::vector<std::size_t> result;
	for (std::size_t index = 0; index < scored.size() && index < k; ++index) {
		result.push_back(scored[index].second);
	}
	return result;
}

static auto test_query_helpers_parity() -> void
{
	const std::vector<int> values = {0, 2, 5, 9, 14};
	auto space = mtrc::make_space(values, AbsoluteDistance{});
	const auto id0 = space.id(0);

	// nearest by id: parity with brute force and with stats::search::knn.
	const auto bf_nearest = brute_force_knn(values, 0, 1);
	const auto nearest = query::nearest(space, id0);
	assert(nearest.id == space.id(bf_nearest[0]));
	assert(nearest.distance == AbsoluteDistance{}(values[0], values[bf_nearest[0]]));

	// k_nearest by id parity with brute force.
	const auto k_nearest = query::k_nearest(space, id0, 3);
	const auto bf_knn = brute_force_knn(values, 0, 3);
	assert(k_nearest.size() == bf_knn.size());
	for (std::size_t index = 0; index < bf_knn.size(); ++index) {
		assert(k_nearest[index].id == space.id(bf_knn[index]));
	}

	// k_nearest delegates to stats::search (identical result).
	const auto direct = mtrc::stats::search::knn(space, id0, 3);
	assert(direct.size() == k_nearest.size());
	for (std::size_t index = 0; index < direct.size(); ++index) {
		assert(direct[index].id == k_nearest[index].id);
		assert(direct[index].distance == k_nearest[index].distance);
	}

	// External-record query, plus representation choice (cover tree / exact scan / distance table).
	const auto by_record = query::k_nearest(space, 4, 2);
	const auto by_tree = query::k_nearest(space, 4, 2, mtrc::stats::search::cover_tree{});
	const auto by_scan = query::k_nearest(space, 4, 2, mtrc::stats::search::exact_scan{});
	assert(by_record.size() == 2 && by_tree.size() == 2 && by_scan.size() == 2);
	for (std::size_t index = 0; index < 2; ++index) {
		assert(by_record[index].id == by_tree[index].id);
		assert(by_record[index].id == by_scan[index].id);
	}
	const auto by_table = query::k_nearest(space, id0, 2, mtrc::stats::search::distance_table{});
	assert(by_table.size() == 2 && by_table[0].id == k_nearest[0].id);

	// within radius parity: count and membership match brute force (radius 5 around id0).
	const auto within = query::within(space, id0, 5);
	const AbsoluteDistance metric;
	std::size_t expected_within = 0;
	for (std::size_t index = 1; index < values.size(); ++index) {
		if (metric(values[0], values[index]) <= 5) {
			++expected_within;
		}
	}
	assert(within.size() == expected_within);
	for (const auto &neighbor : within) {
		assert(metric(values[0], space.record(neighbor.id)) <= 5);
	}

	// Error cases: nearest on a single-record space, negative radius, missing query id.
	auto single = mtrc::make_space(std::vector<int>{42}, AbsoluteDistance{});
	assert(!query::try_nearest(single, single.id(0)).has_value());
	assert(throws([&] { (void)query::nearest(single, single.id(0)); }));
	assert(throws([&] { (void)query::within(space, id0, -1); }));
	assert(throws([&] { (void)query::k_nearest(space, mtrc::RecordId::from_index(99), 2); }));
	// k == 0 is a valid empty request, not an error.
	assert(query::k_nearest(space, id0, 0).empty());
}

int main()
{
	test_builder();
	test_records_mutation_and_stable_ids();
	test_streaming_append_batch();
	test_knn_graph_refresh_after_streaming_append_is_bounded();
	test_distances_access();
	test_query_helpers_parity();
	return 0;
}
