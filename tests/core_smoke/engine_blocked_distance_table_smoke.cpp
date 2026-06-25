#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/engine.hpp"

struct DirectedDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		if (lhs <= rhs) {
			return rhs - lhs;
		}
		return 3 * (lhs - rhs);
	}
};

struct CountingDirectedDistance {
	std::shared_ptr<std::size_t> evaluations;

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*evaluations);
		if (lhs <= rhs) {
			return rhs - lhs;
		}
		return 3 * (lhs - rhs);
	}
};

template <typename Callable> auto assert_representation_error(Callable &&call) -> void
{
	bool rejected = false;
	try {
		call();
	} catch (const mtrc::RepresentationError &) {
		rejected = true;
	}
	assert(rejected);
}

static auto count_regular_files(const std::filesystem::path &directory) -> std::size_t
{
	if (!std::filesystem::exists(directory)) {
		return 0;
	}

	std::size_t count = 0;
	for (const auto &entry : std::filesystem::directory_iterator(directory)) {
		if (entry.is_regular_file()) {
			++count;
		}
	}
	return count;
}

static auto test_blocked_table_contract_and_cache() -> void
{
	const std::vector<int> records{0, 2, 5, 9, 14, 20};
	auto space = mtrc::make_space(records, DirectedDistance{});
	using table_type = mtrc::space::storage::BlockedDistanceTable<decltype(space)>;
	static_assert(mtrc::PairwiseDistances_v<table_type>);

	table_type table(space, mtrc::space::storage::blocked_distance_table_options{4, 2});
	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);
	const auto id3 = space.id(3);
	const auto id4 = space.id(4);
	const auto id5 = space.id(5);

	assert(table.record_count() == records.size());
	assert(table.block_size() == 4);
	assert(table.max_cached_blocks() == 2);
	assert(table.block_count() == 2);
	assert(table.cached_blocks() == 0);
	assert(table.cached_distances() == 0);
	assert(table.distance_evaluations() == 0);
	assert(table.dense_distance_slots() == 0);
	assert(table.source_records() == records);
	assert(table.source_record_ids() == mtrc::record_ids(space));
	assert(table.id(2) == id2);
	assert(table.position_of(id3) == 3);
	assert(table.contains(id4));
	assert(!table.contains(mtrc::RecordId::from_index(99)));
	assert(table.cache_key().find("blocked_distance_table") != std::string::npos);
	assert(table.cache_key().find("block_size=4") != std::string::npos);
	assert(table.cache_key().find("max_cached_blocks=2") != std::string::npos);

	assert(table.distance(id0, id2) == 5);
	assert(table.cached_blocks() == 1);
	assert(table.cached_distances() == 16);
	assert(table.distance_evaluations() == 16);
	assert(table.stats().misses == 1);
	assert(table.stats().hits == 0);

	assert(table.distance_at_position(1, 3) == 7);
	assert(table.distance(id2, id0) == 15);
	assert(table.distance(id0, id0) == 0);
	assert(table.cached_blocks() == 1);
	assert(table.distance_evaluations() == 16);
	assert(table.stats().hits == 3);
	assert(table.stats().misses == 1);

	assert(table.distance(id4, id5) == 6);
	assert(table.cached_blocks() == 2);
	assert(table.cached_distances() == 20);
	assert(table.distance_evaluations() == 20);
	assert(table.stats().misses == 2);

	assert(table.distance(id0, id5) == 20);
	assert(table.cached_blocks() == 2);
	assert(table.cached_distances() <= table.max_cached_blocks() * table.block_size() * table.block_size());
	assert(table.distance_evaluations() == 28);
	assert(table.stats().misses == 3);

	const auto evaluations_before_reloaded_block = table.distance_evaluations();
	assert(table.distance(id1, id3) == 7);
	assert(table.cached_blocks() == 2);
	assert(table.distance_evaluations() == evaluations_before_reloaded_block + 16);

	const auto diagnostics = table.diagnostics();
	assert(diagnostics.records == records.size());
	assert(diagnostics.cached_distances == table.cached_distances());
	assert(diagnostics.distance_evaluations == table.distance_evaluations());
	assert(diagnostics.dense_distance_slots == 0);
	assert(diagnostics.cache_key == table.cache_key());
	assert(diagnostics.metric_key == table.metric_key());
	assert(diagnostics.source_record_ids == table.source_record_ids());
	assert(!diagnostics.stale);
}

static auto test_large_construction_is_not_quadratic() -> void
{
	constexpr std::size_t large_record_count = 32768;
	std::vector<int> records(large_record_count);
	for (std::size_t index = 0; index < records.size(); ++index) {
		records[index] = static_cast<int>(index);
	}

	auto evaluations = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(records, CountingDirectedDistance{evaluations});
	mtrc::space::storage::BlockedDistanceTable<decltype(space)> table(
		space, mtrc::space::storage::blocked_distance_table_options{64, 2});

	assert(*evaluations == 0);
	assert(table.record_count() == large_record_count);
	assert(table.block_count() == 512);
	assert(table.cached_blocks() == 0);
	assert(table.cached_distances() == 0);
	assert(table.distance_evaluations() == 0);
	assert(table.diagnostics().distance_evaluations == 0);
	assert(table.dense_distance_slots() == 0);
	assert(table.memory_bytes_estimate() <
		   (large_record_count * large_record_count * sizeof(int)) / static_cast<std::size_t>(1024));

	const auto first_id = space.id(0);
	const auto last_id = space.id(large_record_count - 1);
	assert(table.distance(first_id, last_id) == static_cast<int>(large_record_count - 1));
	assert(*evaluations == 64 * 64);
	assert(table.distance_evaluations() == 64 * 64);
	assert(table.cached_blocks() == 1);
	assert(table.cached_distances() == 64 * 64);

	assert(table.distance(first_id, last_id) == static_cast<int>(large_record_count - 1));
	assert(*evaluations == 64 * 64);
	assert(table.stats().hits == 1);
	assert(table.stats().misses == 1);
}

static auto test_evicted_blocks_can_spill_to_disk_without_recomputing() -> void
{
	const std::vector<int> records{0, 10, 20, 30};
	auto evaluations = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(records, CountingDirectedDistance{evaluations});
	const auto spill_directory =
		std::filesystem::temp_directory_path() /
		("metric-blocked-distance-table-spill-smoke-" +
		 std::to_string(reinterpret_cast<std::uintptr_t>(evaluations.get())));
	std::filesystem::remove_all(spill_directory);

	{
		mtrc::space::storage::blocked_distance_table_options options;
		options.block_size = 2;
		options.max_cached_blocks = 1;
		options.spill_to_disk = true;
		options.spill_directory = spill_directory;
		options.cleanup_spill_directory = true;

		mtrc::space::storage::BlockedDistanceTable<decltype(space)> table(space, options);
		assert(table.spill_enabled());
		assert(table.spill_directory() == spill_directory);
		assert(table.max_spill_bytes() == 0);
		assert(std::filesystem::exists(spill_directory));
		assert(table.cached_blocks() == 0);
		assert(table.spilled_blocks() == 0);
		assert(table.spill_reads() == 0);
		assert(table.spill_writes() == 0);

		assert(table.distance(space.id(0), space.id(1)) == 10);
		assert(*evaluations == 4);
		assert(table.cached_blocks() == 1);
		assert(table.cached_distances() == 4);
		assert(table.spilled_blocks() == 0);

		assert(table.distance(space.id(2), space.id(3)) == 10);
		assert(*evaluations == 8);
		assert(table.cached_blocks() == 1);
		assert(table.cached_distances() == 4);
		assert(table.spilled_blocks() == 1);
		assert(table.spill_writes() == 1);
		assert(table.spill_bytes_written() == 4 * sizeof(int));

		assert(table.distance(space.id(0), space.id(1)) == 10);
		assert(*evaluations == 8);
		assert(table.distance_evaluations() == 8);
		assert(table.cached_blocks() == 1);
		assert(table.cached_distances() == 4);
		assert(table.spilled_blocks() == 2);
		assert(table.spill_reads() == 1);
		assert(table.spill_writes() == 2);
		assert(table.spill_bytes_read() == 4 * sizeof(int));
		assert(table.stats().spilled_blocks == table.spilled_blocks());
		assert(table.stats().spill_reads == table.spill_reads());
		assert(table.stats().spill_writes == table.spill_writes());
		assert(!table.diagnostics().warnings.empty());
	}

	assert(!std::filesystem::exists(spill_directory));
}

static auto test_spill_byte_quota_rejects_before_second_write() -> void
{
	const std::vector<int> records{0, 10, 20, 30, 40, 50};
	auto evaluations = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(records, CountingDirectedDistance{evaluations});
	const auto spill_directory =
		std::filesystem::temp_directory_path() /
		("metric-blocked-distance-table-spill-quota-smoke-" +
		 std::to_string(reinterpret_cast<std::uintptr_t>(evaluations.get())));
	std::filesystem::remove_all(spill_directory);

	{
		const auto block_bytes = std::size_t{4} * sizeof(int);
		mtrc::space::storage::blocked_distance_table_options options;
		options.block_size = 2;
		options.max_cached_blocks = 1;
		options.spill_to_disk = true;
		options.spill_directory = spill_directory;
		options.cleanup_spill_directory = true;
		options.max_spill_bytes = block_bytes;

		mtrc::space::storage::BlockedDistanceTable<decltype(space)> table(space, options);
		assert(table.spill_enabled());
		assert(table.max_spill_bytes() == block_bytes);

		assert(table.distance(space.id(0), space.id(1)) == 10);
		assert(*evaluations == 4);
		assert(table.spilled_blocks() == 0);
		assert(table.spill_writes() == 0);

		assert(table.distance(space.id(2), space.id(3)) == 10);
		assert(*evaluations == 8);
		assert(table.cached_blocks() == 1);
		assert(table.cached_distances() == 4);
		assert(table.spilled_blocks() == 1);
		assert(table.spill_writes() == 1);
		assert(table.spill_bytes_written() == block_bytes);
		assert(count_regular_files(spill_directory) == 1);

		assert_representation_error([&table, &space]() {
			(void)table.distance(space.id(4), space.id(5));
		});
		assert(*evaluations == 8);
		assert(table.distance_evaluations() == 8);
		assert(table.cached_blocks() == 1);
		assert(table.cached_distances() == 4);
		assert(table.spilled_blocks() == 1);
		assert(table.spill_writes() == 1);
		assert(table.spill_bytes_written() == block_bytes);
		assert(count_regular_files(spill_directory) == 1);
		assert(table.stats().spilled_blocks == 1);
		assert(table.stats().spill_writes == 1);

		assert(table.distance(space.id(2), space.id(3)) == 10);
		assert(*evaluations == 8);
		assert(table.spilled_blocks() == 1);
		assert(table.spill_writes() == 1);
	}

	assert(!std::filesystem::exists(spill_directory));
}

static auto test_spill_byte_quota_flows_from_resource_budget() -> void
{
	const std::vector<int> records{0, 1, 2};
	auto space = mtrc::make_space(records, DirectedDistance{});
	constexpr auto max_spill_bytes = std::size_t{1234};

	mtrc::space::storage::resource_budget budget;
	budget.allow_out_of_core_spill = true;
	budget.max_spill_bytes = max_spill_bytes;
	const auto runtime_policy = mtrc::space::storage::with_resource_budget(
		mtrc::space::storage::using_distance_table(), budget);
	assert(runtime_policy.max_spill_bytes() == max_spill_bytes);

	const auto options = mtrc::space::storage::blocked_distance_table_options_for<int>(runtime_policy);
	assert(options.spill_to_disk);
	assert(options.max_spill_bytes == max_spill_bytes);

	const auto table = mtrc::space::storage::make_blocked_distance_table(space, runtime_policy);
	assert(table.spill_enabled());
	assert(table.max_spill_bytes() == max_spill_bytes);

	const auto diagnostics = mtrc::space::storage::diagnostics_for_space(space, runtime_policy, {}, "neighbors");
	assert(diagnostics.max_spill_bytes == max_spill_bytes);
	assert(diagnostics.budget.max_spill_bytes == max_spill_bytes);
}

static auto test_stale_keeps_snapshot_records() -> void
{
	const std::vector<int> records{0, 2, 5, 9, 14, 20};
	auto space = mtrc::make_space(records, DirectedDistance{});
	const auto id0 = space.id(0);
	const auto id5 = space.id(5);
	const auto version = space.version();

	mtrc::space::storage::BlockedDistanceTable<decltype(space)> table(
		space, mtrc::space::storage::blocked_distance_table_options{4, 2});
	space.replace(id5, 1000);

	assert(space.version() == version + 1);
	assert(table.version() == version);
	assert(table.built_for_version() == version);
	assert(table.source_version() == space.version());
	assert(table.is_stale());
	assert(table.diagnostics().stale);
	assert(!table.diagnostics().warnings.empty());
	assert(table.source_records().back() == 20);
	assert(table.distance(id0, id5) == 20);
	assert(space.distance(id0, id5) == 1000);

	assert(space.erase(id0));
	const auto inserted_id = space.insert(30);
	assert(table.contains(id0));
	assert(table.contains(id5));
	assert(!table.contains(inserted_id));
	assert(table.position_of(id5) == 5);
	assert(table.source_record_ids().front() == id0);
	assert(table.distance(id0, id5) == 20);
}

static auto test_invalid_options_are_rejected() -> void
{
	const std::vector<int> records{0, 1};
	auto space = mtrc::make_space(records, DirectedDistance{});

	assert_representation_error([&space]() {
		(void)mtrc::space::storage::BlockedDistanceTable<decltype(space)>(
			space, mtrc::space::storage::blocked_distance_table_options{0, 1});
	});
	assert_representation_error([&space]() {
		(void)mtrc::space::storage::BlockedDistanceTable<decltype(space)>(
			space, mtrc::space::storage::blocked_distance_table_options{1, 0});
	});
}

int main()
{
	test_blocked_table_contract_and_cache();
	test_large_construction_is_not_quadratic();
	test_evicted_blocks_can_spill_to_disk_without_recomputing();
	test_spill_byte_quota_rejects_before_second_write();
	test_spill_byte_quota_flows_from_resource_budget();
	test_stale_keeps_snapshot_records();
	test_invalid_options_are_rejected();
	return 0;
}
