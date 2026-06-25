// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_BLOCKED_DISTANCE_TABLE_HPP
#define _METRIC_REPRESENTATIONS_BLOCKED_DISTANCE_TABLE_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <list>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/record/id.hpp>

#include "diagnostics.hpp"
#include "policy.hpp"

namespace mtrc::space::storage {

struct blocked_distance_table_options {
	std::size_t block_size{256};
	std::size_t max_cached_blocks{16};
	runtime_guard runtime;
	bool spill_to_disk{false};
	std::filesystem::path spill_directory{};
	bool cleanup_spill_directory{true};
	std::size_t max_spill_bytes{};
};

namespace blocked_detail {

using record_position_lookup_type = std::unordered_map<RecordId, std::size_t>;

inline auto checked_product(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
{
	if (rhs != 0 && lhs > std::numeric_limits<std::size_t>::max() / rhs) {
		throw RepresentationError(message);
	}
	return lhs * rhs;
}

inline auto checked_sum(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
{
	if (lhs > std::numeric_limits<std::size_t>::max() - rhs) {
		throw RepresentationError(message);
	}
	return lhs + rhs;
}

inline auto make_record_position_lookup(const std::vector<RecordId> &ids) -> record_position_lookup_type
{
	record_position_lookup_type positions;
	positions.reserve(ids.size());
	for (std::size_t position = 0; position < ids.size(); ++position) {
		positions.emplace(ids[position], position);
	}
	return positions;
}

inline auto checked_block_count(std::size_t record_count, std::size_t block_size) -> std::size_t
{
	return record_count == 0 ? 0 : ((record_count - 1) / block_size) + 1;
}

} // namespace blocked_detail

template <typename Space> class BlockedDistanceTable {
  public:
	using space_type = Space;
	using record_type = typename space_type::record_type;
	using distance_type = typename space_type::distance_type;

	explicit BlockedDistanceTable(const space_type &space, std::size_t block_size)
		: BlockedDistanceTable(space, blocked_distance_table_options{block_size})
	{
	}

	explicit BlockedDistanceTable(const space_type &space, blocked_distance_table_options options = {})
		: space_(&space), record_count_(space.size()), version_(space.version()),
		  options_(validate_options(options)),
		  block_count_(blocked_detail::checked_block_count(record_count_, options_.block_size)),
		  metric_key_(core::metric_cache_key(space.metric()))
	{
		ids_ = mtrc::record_ids(space);
		records_ = space.records();
		id_positions_ = blocked_detail::make_record_position_lookup(ids_);
		cache_key_ = representation_cache_key(
			"blocked_distance_table", metric_key_, version_, ids_,
			{{"block_size", std::to_string(options_.block_size)},
			 {"max_cached_blocks", std::to_string(options_.max_cached_blocks)},
			 {"spill", options_.spill_to_disk ? "disk" : "memory"},
			 {"max_spill_bytes", std::to_string(options_.max_spill_bytes)}});
		if (options_.spill_to_disk) {
			initialize_spill_state();
		}
	}

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type
	{
		const auto lhs_position = position_of(lhs);
		const auto rhs_position = position_of(rhs);
		return distance_at_validated_position(lhs_position, rhs_position);
	}

	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position) const -> distance_type
	{
		validate_position(lhs_position);
		validate_position(rhs_position);
		return distance_at_validated_position(lhs_position, rhs_position);
	}

	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position,
							  runtime_guard runtime) const -> distance_type
	{
		runtime.throw_if_cancelled("blocked_distance_table lookup");
		validate_position(lhs_position);
		validate_position(rhs_position);
		return distance_at_validated_position(lhs_position, rhs_position, runtime);
	}

	auto record_count() const -> std::size_t { return record_count_; }

	auto id(std::size_t position) const -> RecordId
	{
		validate_position(position);
		return ids_[position];
	}

	auto position_of(RecordId id) const -> std::size_t
	{
		const auto position = id_positions_.find(id);
		if (position == id_positions_.end()) {
			throw std::out_of_range("record id is outside the blocked distance table");
		}
		return position->second;
	}

	auto contains(RecordId id) const -> bool { return id_positions_.find(id) != id_positions_.end(); }
	auto version() const -> std::size_t { return version_; }
	auto built_for_version() const -> std::size_t { return version_; }
	auto source_version() const -> std::size_t { return space_->version(); }
	auto is_stale() const -> bool { return space_->version() != version_; }
	auto block_size() const -> std::size_t { return options_.block_size; }
	auto max_cached_blocks() const -> std::size_t { return options_.max_cached_blocks; }
	auto block_count() const -> std::size_t { return block_count_; }
	auto cached_blocks() const -> std::size_t { return block_cache_.size(); }
	auto cached_distances() const -> std::size_t { return cached_distance_count_; }
	auto distance_evaluations() const -> std::size_t { return distance_evaluations_; }
	auto dense_distance_slots() const -> std::size_t { return 0; }
	auto spill_enabled() const -> bool { return options_.spill_to_disk; }
	auto spill_directory() const -> std::filesystem::path
	{
		return spill_state_ == nullptr ? std::filesystem::path{} : spill_state_->directory;
	}
	auto spilled_blocks() const -> std::size_t
	{
		return spill_state_ == nullptr ? std::size_t{} : spill_state_->blocks.size();
	}
	auto spill_reads() const -> std::size_t { return spill_reads_; }
	auto spill_writes() const -> std::size_t { return spill_writes_; }
	auto spill_bytes_read() const -> std::size_t { return spill_bytes_read_; }
	auto spill_bytes_written() const -> std::size_t { return spill_bytes_written_; }
	auto max_spill_bytes() const -> std::size_t { return options_.max_spill_bytes; }
	auto memory_bytes_estimate() const -> std::size_t { return current_memory_bytes_estimate(); }
	auto metric_key() const -> const std::string & { return metric_key_; }
	auto cache_key() const -> const std::string & { return cache_key_; }
	auto source_record_ids() const -> const std::vector<RecordId> & { return ids_; }
	auto source_records() const -> const std::vector<record_type> & { return records_; }

	auto stats() const -> distance_table_stats
	{
		distance_table_stats result;
		result.hits = hits_;
		result.misses = misses_;
		result.fill_ratio = fill_ratio();
		result.symmetric_storage = false;
		result.spilled_blocks = spilled_blocks();
		result.spill_reads = spill_reads_;
		result.spill_writes = spill_writes_;
		return result;
	}

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::distance_table, exactness::exact,
										  materialization::lazy, update_mode::snapshot};
		result.space_version = space_->version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.records = record_count_;
		result.distance_evaluations = distance_evaluations_;
		result.cached_distances = cached_distance_count_;
		result.dense_distance_slots = 0;
		result.memory_bytes_estimate = memory_bytes_estimate();
		result.cache_key = cache_key_;
		result.metric_key = metric_key_;
		result.source_record_ids = ids_;
		if (options_.spill_to_disk) {
			result.warnings.push_back("blocked distance table spills evicted blocks to disk");
			if (options_.max_spill_bytes > 0) {
				result.warnings.push_back("blocked distance table enforces a spill byte quota");
			}
		}
		if (result.stale) {
			result.warnings.push_back("blocked distance table was built for an older metric-space version");
		}
		return result;
	}

  private:
	struct block_cache_key {
		std::size_t lhs_block{};
		std::size_t rhs_block{};

		friend auto operator==(block_cache_key lhs, block_cache_key rhs) -> bool
		{
			return lhs.lhs_block == rhs.lhs_block && lhs.rhs_block == rhs.rhs_block;
		}
	};

	struct block_cache_key_hash {
		auto operator()(block_cache_key key) const noexcept -> std::size_t
		{
			const auto lhs = std::hash<std::size_t>{}(key.lhs_block);
			const auto rhs = std::hash<std::size_t>{}(key.rhs_block);
			return lhs ^ (rhs + 0x9e3779b97f4a7c15ULL + (lhs << 6U) + (lhs >> 2U));
		}
	};

	using lru_list_type = std::list<block_cache_key>;

	struct cached_block {
		std::size_t lhs_begin{};
		std::size_t rhs_begin{};
		std::size_t lhs_count{};
		std::size_t rhs_count{};
		std::vector<distance_type> distances;
		typename lru_list_type::iterator lru_position;
		bool loaded_from_spill{};
		bool persisted_to_spill{};
	};

	using block_cache_type = std::unordered_map<block_cache_key, cached_block, block_cache_key_hash>;

	struct spilled_block {
		std::filesystem::path path;
		std::size_t lhs_begin{};
		std::size_t rhs_begin{};
		std::size_t lhs_count{};
		std::size_t rhs_count{};
		std::size_t distance_count{};
		std::size_t byte_count{};
	};

	struct spill_state {
		std::filesystem::path directory;
		bool cleanup_directory{true};
		std::unordered_map<block_cache_key, spilled_block, block_cache_key_hash> blocks;

		~spill_state()
		{
			if (cleanup_directory && !directory.empty()) {
				std::error_code ignored;
				for (const auto &entry : blocks) {
					std::filesystem::remove(entry.second.path, ignored);
				}
				std::filesystem::remove(directory, ignored);
			}
		}
	};

	static auto validate_options(blocked_distance_table_options options) -> blocked_distance_table_options
	{
		if (options.block_size == 0) {
			throw RepresentationError("blocked distance table requires a positive block_size");
		}
		if (options.max_cached_blocks == 0) {
			throw RepresentationError("blocked distance table requires a positive max_cached_blocks");
		}
		if (options.spill_to_disk && !std::is_trivially_copyable_v<distance_type>) {
			throw RepresentationError("blocked distance table disk spill requires a trivially copyable distance type");
		}
		return options;
	}

	static auto default_spill_directory(const void *owner) -> std::filesystem::path
	{
		return std::filesystem::temp_directory_path() /
			   ("metric-blocked-distance-table-" +
				std::to_string(reinterpret_cast<std::uintptr_t>(owner)));
	}

	auto initialize_spill_state() -> void
	{
		spill_state_ = std::make_shared<spill_state>();
		spill_state_->cleanup_directory = options_.cleanup_spill_directory;
		spill_state_->directory =
			options_.spill_directory.empty() ? default_spill_directory(this) : options_.spill_directory;
		std::error_code error;
		std::filesystem::create_directories(spill_state_->directory, error);
		if (error) {
			throw RepresentationError("blocked distance table could not create spill directory: " + error.message());
		}
	}

	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the blocked distance table");
		}
	}

	auto distance_at_validated_position(std::size_t lhs_position, std::size_t rhs_position,
										runtime_guard runtime = {}) const -> distance_type
	{
		const block_cache_key key{lhs_position / options_.block_size, rhs_position / options_.block_size};
		const auto &block = block_at(key, runtime);
		const auto lhs_offset = lhs_position - block.lhs_begin;
		const auto rhs_offset = rhs_position - block.rhs_begin;
		return block.distances[lhs_offset * block.rhs_count + rhs_offset];
	}

	auto block_at(block_cache_key key, runtime_guard runtime = {}) const -> const cached_block &
	{
		const auto cached = block_cache_.find(key);
		if (cached != block_cache_.end()) {
			++hits_;
			lru_blocks_.splice(lru_blocks_.begin(), lru_blocks_, cached->second.lru_position);
			cached->second.lru_position = lru_blocks_.begin();
			return cached->second;
		}

		++misses_;
		while (block_cache_.size() >= options_.max_cached_blocks) {
			evict_lru_block();
		}

		auto block = load_or_materialize_block(key, runtime.active() ? runtime : options_.runtime);
		lru_blocks_.push_front(key);
		block.lru_position = lru_blocks_.begin();
		cached_distance_count_ = blocked_detail::checked_sum(
			cached_distance_count_, block.distances.size(),
			"blocked distance table cached distance count exceeds size_t capacity");
		if (!block.loaded_from_spill) {
			distance_evaluations_ = blocked_detail::checked_sum(
				distance_evaluations_, block.distances.size(),
				"blocked distance table distance evaluation count exceeds size_t capacity");
		}
		const auto inserted = block_cache_.emplace(key, std::move(block));
		return inserted.first->second;
	}

	auto evict_lru_block() const -> void
	{
		const auto key = lru_blocks_.back();
		const auto cached = block_cache_.find(key);
		if (cached != block_cache_.end()) {
			if (options_.spill_to_disk && !cached->second.persisted_to_spill) {
				write_spilled_block(key, cached->second);
			}
			cached_distance_count_ -= cached->second.distances.size();
			block_cache_.erase(cached);
		}
		lru_blocks_.pop_back();
	}

	auto load_or_materialize_block(block_cache_key key, runtime_guard runtime = {}) const -> cached_block
	{
		if (auto spilled = load_spilled_block(key)) {
			return std::move(*spilled);
		}
		return materialize_block(key, runtime);
	}

	auto load_spilled_block(block_cache_key key) const -> std::optional<cached_block>
	{
		if (spill_state_ == nullptr) {
			return std::nullopt;
		}
		const auto spilled = spill_state_->blocks.find(key);
		if (spilled == spill_state_->blocks.end()) {
			return std::nullopt;
		}

		const auto &metadata = spilled->second;
		std::ifstream input(metadata.path, std::ios::binary);
		if (!input) {
			throw RepresentationError("blocked distance table could not open spilled block for reading");
		}

		cached_block block;
		block.lhs_begin = metadata.lhs_begin;
		block.rhs_begin = metadata.rhs_begin;
		block.lhs_count = metadata.lhs_count;
		block.rhs_count = metadata.rhs_count;
		block.distances.resize(metadata.distance_count);
		input.read(reinterpret_cast<char *>(block.distances.data()),
				   static_cast<std::streamsize>(metadata.byte_count));
		if (!input) {
			throw RepresentationError("blocked distance table could not read spilled block");
		}
		block.loaded_from_spill = true;
		block.persisted_to_spill = true;
		++spill_reads_;
		spill_bytes_read_ = blocked_detail::checked_sum(
			spill_bytes_read_, metadata.byte_count,
			"blocked distance table spill byte count exceeds size_t capacity");
		return block;
	}

	auto write_spilled_block(block_cache_key key, const cached_block &block) const -> void
	{
		if (spill_state_ == nullptr) {
			return;
		}
		std::error_code error;
		std::filesystem::create_directories(spill_state_->directory, error);
		if (error) {
			throw RepresentationError("blocked distance table could not create spill directory: " + error.message());
		}

		const auto file_name = "block-" + std::to_string(key.lhs_block) + "-" +
							   std::to_string(key.rhs_block) + ".bin";
		const auto path = spill_state_->directory / file_name;
		const auto byte_count = blocked_detail::checked_product(
			block.distances.size(), sizeof(distance_type),
			"blocked distance table spilled block size exceeds size_t capacity");
		if (options_.max_spill_bytes > 0 &&
			(byte_count > options_.max_spill_bytes ||
			 spill_bytes_written_ > options_.max_spill_bytes - byte_count)) {
			throw RepresentationError(
				"blocked distance table spill byte quota exceeded: current_bytes=" +
				std::to_string(spill_bytes_written_) + " block_bytes=" + std::to_string(byte_count) +
				" max_spill_bytes=" + std::to_string(options_.max_spill_bytes));
		}
		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		if (!output) {
			throw RepresentationError("blocked distance table could not open spilled block for writing");
		}
		output.write(reinterpret_cast<const char *>(block.distances.data()),
					 static_cast<std::streamsize>(byte_count));
		if (!output) {
			std::error_code ignored;
			std::filesystem::remove(path, ignored);
			throw RepresentationError("blocked distance table could not write spilled block");
		}

		spilled_block metadata;
		metadata.path = path;
		metadata.lhs_begin = block.lhs_begin;
		metadata.rhs_begin = block.rhs_begin;
		metadata.lhs_count = block.lhs_count;
		metadata.rhs_count = block.rhs_count;
		metadata.distance_count = block.distances.size();
		metadata.byte_count = byte_count;
		spill_state_->blocks[key] = std::move(metadata);
		++spill_writes_;
		spill_bytes_written_ = blocked_detail::checked_sum(
			spill_bytes_written_, byte_count,
			"blocked distance table spill byte count exceeds size_t capacity");
	}

	auto materialize_block(block_cache_key key, runtime_guard runtime = {}) const -> cached_block
	{
		cached_block block;
		block.lhs_begin = key.lhs_block * options_.block_size;
		block.rhs_begin = key.rhs_block * options_.block_size;
		const auto lhs_end = std::min(record_count_, block.lhs_begin + options_.block_size);
		const auto rhs_end = std::min(record_count_, block.rhs_begin + options_.block_size);
		block.lhs_count = lhs_end - block.lhs_begin;
		block.rhs_count = rhs_end - block.rhs_begin;
		const auto cell_count = blocked_detail::checked_product(
			block.lhs_count, block.rhs_count,
			"blocked distance table block size exceeds size_t capacity");
		block.distances.reserve(cell_count);
		for (std::size_t lhs = block.lhs_begin; lhs < lhs_end; ++lhs) {
			for (std::size_t rhs = block.rhs_begin; rhs < rhs_end; ++rhs) {
				runtime.throw_if_cancelled("blocked_distance_table block materialization");
				block.distances.push_back(space_->metric()(records_.at(lhs), records_.at(rhs)));
			}
		}
		return block;
	}

	auto fill_ratio() const -> double
	{
		if (record_count_ == 0) {
			return 1.0;
		}
		const auto possible_distances = static_cast<double>(record_count_) * static_cast<double>(record_count_);
		return static_cast<double>(cached_distance_count_) / possible_distances;
	}

	auto current_memory_bytes_estimate() const -> std::size_t
	{
		const auto record_bytes = blocked_detail::checked_product(
			records_.size(), sizeof(record_type),
			"blocked distance table memory estimate exceeds size_t capacity");
		const auto id_bytes = blocked_detail::checked_product(
			ids_.size(), sizeof(RecordId),
			"blocked distance table memory estimate exceeds size_t capacity");
		const auto index_bytes = blocked_detail::checked_product(
			id_positions_.size(), sizeof(blocked_detail::record_position_lookup_type::value_type),
			"blocked distance table memory estimate exceeds size_t capacity");
		const auto distance_bytes = blocked_detail::checked_product(
			cached_distance_count_, sizeof(distance_type),
			"blocked distance table memory estimate exceeds size_t capacity");
		const auto cache_entry_bytes = blocked_detail::checked_product(
			block_cache_.size(), sizeof(typename block_cache_type::value_type),
			"blocked distance table memory estimate exceeds size_t capacity");
		const auto lru_bytes = blocked_detail::checked_product(
			lru_blocks_.size(), sizeof(block_cache_key),
			"blocked distance table memory estimate exceeds size_t capacity");

		const auto identity_bytes = blocked_detail::checked_sum(
			id_bytes, index_bytes, "blocked distance table memory estimate exceeds size_t capacity");
		const auto cache_bytes = blocked_detail::checked_sum(
			distance_bytes, cache_entry_bytes,
			"blocked distance table memory estimate exceeds size_t capacity");
		return blocked_detail::checked_sum(
			blocked_detail::checked_sum(
				record_bytes, identity_bytes,
				"blocked distance table memory estimate exceeds size_t capacity"),
			blocked_detail::checked_sum(
				cache_bytes, lru_bytes,
				"blocked distance table memory estimate exceeds size_t capacity"),
			"blocked distance table memory estimate exceeds size_t capacity");
	}

	const space_type *space_;
	std::size_t record_count_;
	std::size_t version_;
	blocked_distance_table_options options_;
	std::size_t block_count_;
	std::string metric_key_;
	std::string cache_key_;
	std::vector<RecordId> ids_;
	std::vector<record_type> records_;
	blocked_detail::record_position_lookup_type id_positions_;
	mutable lru_list_type lru_blocks_;
	mutable block_cache_type block_cache_;
	std::shared_ptr<spill_state> spill_state_;
	mutable std::size_t cached_distance_count_{};
	mutable std::size_t distance_evaluations_{};
	mutable std::size_t hits_{};
	mutable std::size_t misses_{};
	mutable std::size_t spill_reads_{};
	mutable std::size_t spill_writes_{};
	mutable std::size_t spill_bytes_read_{};
	mutable std::size_t spill_bytes_written_{};
};

template <typename Space>
auto blocked_distance_table(const Space &space, blocked_distance_table_options options = {})
	-> BlockedDistanceTable<Space>
{
	return BlockedDistanceTable<Space>(space, options);
}

template <typename Space>
auto blocked_distance_table(const Space &space, std::size_t block_size) -> BlockedDistanceTable<Space>
{
	return BlockedDistanceTable<Space>(space, block_size);
}

} // namespace mtrc::space::storage

#endif
