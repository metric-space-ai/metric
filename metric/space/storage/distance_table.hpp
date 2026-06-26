// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_MATRIX_CACHE_HPP
#define _METRIC_REPRESENTATIONS_MATRIX_CACHE_HPP

#include <cstddef>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <metric/core/errors.hpp>
#include <metric/core/concepts.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/record/id.hpp>
#include "diagnostics.hpp"
#include "policy.hpp"

namespace mtrc::space::storage {

enum class distance_table_mode {
	eager,
	lazy,
};

inline constexpr std::size_t default_distance_table_max_dense_records = 4096;
inline constexpr std::size_t default_distance_table_max_memory_bytes = 512ULL * 1024ULL * 1024ULL;

struct distance_table_options {
	distance_table_mode mode{distance_table_mode::eager};
	// Set either budget to 0 only when the caller intentionally opts into unbounded table growth.
	std::size_t max_dense_records{default_distance_table_max_dense_records};
	std::size_t max_memory_bytes{default_distance_table_max_memory_bytes};
	runtime_guard runtime;
};

namespace detail {

using record_position_lookup_type = std::unordered_map<RecordId, std::size_t>;

inline auto make_record_position_lookup(const std::vector<RecordId> &ids) -> record_position_lookup_type
{
	record_position_lookup_type positions;
	positions.reserve(ids.size());
	for (std::size_t position = 0; position < ids.size(); ++position) {
		positions.emplace(ids[position], position);
	}
	return positions;
}

inline auto checked_distance_table_size_product(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
{
	if (rhs != 0 && lhs > std::numeric_limits<std::size_t>::max() / rhs) {
		throw RepresentationError(message);
	}
	return lhs * rhs;
}

inline auto checked_distance_table_size_sum(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
{
	if (lhs > std::numeric_limits<std::size_t>::max() - rhs) {
		throw RepresentationError(message);
	}
	return lhs + rhs;
}

inline auto distance_table_dense_slot_count(std::size_t record_count) -> std::size_t
{
	return checked_distance_table_size_product(
		record_count, record_count, "distance table dense storage exceeds size_t capacity");
}

inline auto distance_table_record_id_storage_bytes(std::size_t record_count) -> std::size_t
{
	return checked_distance_table_size_product(
		record_count, sizeof(RecordId), "distance table memory estimate exceeds size_t capacity");
}

inline auto distance_table_position_index_storage_bytes(std::size_t record_count) -> std::size_t
{
	return checked_distance_table_size_product(
		record_count, sizeof(record_position_lookup_type::value_type),
		"distance table memory estimate exceeds size_t capacity");
}

} // namespace detail

template <typename Distance> auto estimate_distance_table_memory_bytes(std::size_t record_count) -> std::size_t
{
	const auto dense_slots = detail::distance_table_dense_slot_count(record_count);
	const auto matrix_bytes = detail::checked_distance_table_size_product(
		dense_slots, sizeof(std::optional<Distance>), "distance table memory estimate exceeds size_t capacity");
	const auto id_bytes = detail::distance_table_record_id_storage_bytes(record_count);
	const auto index_bytes = detail::distance_table_position_index_storage_bytes(record_count);
	return detail::checked_distance_table_size_sum(
		detail::checked_distance_table_size_sum(
			matrix_bytes, id_bytes, "distance table memory estimate exceeds size_t capacity"),
		index_bytes, "distance table memory estimate exceeds size_t capacity");
}

template <typename Distance> struct distance_table_snapshot_cell {
	RecordId lhs;
	RecordId rhs;
	Distance distance;
};

struct distance_table_snapshot_cell_key {
	RecordId lhs;
	RecordId rhs;

	friend auto operator==(distance_table_snapshot_cell_key lhs, distance_table_snapshot_cell_key rhs) -> bool
	{
		return lhs.lhs == rhs.lhs && lhs.rhs == rhs.rhs;
	}
};

struct distance_table_snapshot_cell_key_hash {
	auto operator()(distance_table_snapshot_cell_key key) const noexcept -> std::size_t
	{
		const auto lhs = std::hash<RecordId>{}(key.lhs);
		const auto rhs = std::hash<RecordId>{}(key.rhs);
		return lhs ^ (rhs + 0x9e3779b97f4a7c15ULL + (lhs << 6U) + (lhs >> 2U));
	}
};

using distance_table_snapshot_cell_index =
	std::unordered_map<distance_table_snapshot_cell_key, std::size_t, distance_table_snapshot_cell_key_hash>;

template <typename Distance>
auto distance_table_snapshot_cell_position(const std::vector<distance_table_snapshot_cell<Distance>> &cells, RecordId lhs,
										 RecordId rhs) -> std::size_t
{
	for (std::size_t index = 0; index < cells.size(); ++index) {
		if (cells[index].lhs == lhs && cells[index].rhs == rhs) {
			return index;
		}
	}
	return cells.size();
}

template <typename Distance>
auto has_distance_table_snapshot_cell(const std::vector<distance_table_snapshot_cell<Distance>> &cells, RecordId lhs,
									RecordId rhs) -> bool
{
	return distance_table_snapshot_cell_position(cells, lhs, rhs) != cells.size();
}

template <typename Distance>
auto distance_table_snapshot_cell_distance_or_throw(const std::vector<distance_table_snapshot_cell<Distance>> &cells,
												  RecordId lhs, RecordId rhs, const char *message) -> Distance
{
	const auto position = distance_table_snapshot_cell_position(cells, lhs, rhs);
	if (position == cells.size()) {
		throw std::out_of_range(message);
	}
	return cells[position].distance;
}

template <typename Distance> struct distance_table_snapshot {
	using distance_type = Distance;
	using cell_type = distance_table_snapshot_cell<distance_type>;

	representation_kind kind{representation_kind::distance_table};
	exactness exact{exactness::exact};
	materialization materialized{materialization::materialized};
	update_mode updates{update_mode::snapshot};
	distance_table_mode mode{distance_table_mode::eager};
	std::size_t built_for_version{};
	std::size_t record_count{};
	std::size_t cached_distances{};
	std::size_t dense_distance_slots{};
	std::size_t max_dense_records{};
	std::size_t max_memory_bytes{};
	std::string metric_key;
	std::string cache_key;
	std::vector<RecordId> source_record_ids;
	std::vector<cell_type> distances;
	mutable detail::record_position_lookup_type record_positions_;
	mutable distance_table_snapshot_cell_index cell_positions_;
	mutable std::size_t indexed_record_count_{};
	mutable std::size_t indexed_distance_count_{};

	auto rebuild_indexes() -> void
	{
		rebuild_indexes_();
	}

	auto rebuild_indexes() const -> void
	{
		rebuild_indexes_();
	}

  private:
	auto ensure_record_index() const -> void
	{
		if (indexed_record_count_ != source_record_ids.size()) {
			rebuild_record_index_();
		}
	}

	auto ensure_cell_index() const -> void
	{
		if (indexed_distance_count_ != distances.size()) {
			rebuild_cell_index_();
		}
	}

  public:
	auto contains(RecordId id) const -> bool
	{
		ensure_record_index();
		return record_positions_.find(id) != record_positions_.end();
	}

	auto position_of(RecordId id) const -> std::size_t
	{
		ensure_record_index();
		const auto position = record_positions_.find(id);
		if (position == record_positions_.end()) {
			throw std::out_of_range("record id is outside the distance table snapshot");
		}
		return position->second;
	}

	auto has_distance(RecordId lhs, RecordId rhs) const -> bool
	{
		ensure_cell_index();
		return cell_positions_.find(distance_table_snapshot_cell_key{lhs, rhs}) != cell_positions_.end();
	}

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type
	{
		ensure_cell_index();
		const auto position = cell_positions_.find(distance_table_snapshot_cell_key{lhs, rhs});
		if (position == cell_positions_.end()) {
			throw std::out_of_range("distance cell is not present in the distance table snapshot");
		}
		return distances[position->second].distance;
	}

  private:
	auto rebuild_indexes_() const -> void
	{
		rebuild_record_index_();
		rebuild_cell_index_();
	}

	auto rebuild_record_index_() const -> void
	{
		record_positions_ = detail::make_record_position_lookup(source_record_ids);
		indexed_record_count_ = source_record_ids.size();
	}

	auto rebuild_cell_index_() const -> void
	{
		cell_positions_.clear();
		cell_positions_.reserve(distances.size());
		for (std::size_t position = 0; position < distances.size(); ++position) {
			cell_positions_.emplace(
				distance_table_snapshot_cell_key{distances[position].lhs, distances[position].rhs}, position);
		}
		indexed_distance_count_ = distances.size();
	}
};

template <typename Distance, typename CachedDistanceAt>
auto distance_table_snapshot_cells(const std::vector<RecordId> &ids, std::size_t record_count,
								 std::size_t expected_cached_count, CachedDistanceAt cached_distance_at)
	-> std::vector<distance_table_snapshot_cell<Distance>>
{
	if (record_count > ids.size()) {
		throw RepresentationError("distance table snapshot cell IDs are incomplete");
	}
	std::vector<distance_table_snapshot_cell<Distance>> cells;
	cells.reserve(expected_cached_count);
	for (std::size_t lhs = 0; lhs < record_count; ++lhs) {
		for (std::size_t rhs = 0; rhs < record_count; ++rhs) {
			auto &&cached = cached_distance_at(lhs, rhs);
			if (cached.has_value()) {
				cells.push_back(distance_table_snapshot_cell<Distance>{ids[lhs], ids[rhs], *cached});
			}
		}
	}
	return cells;
}

template <typename Space> class DistanceTable {
  public:
	using space_type = Space;
	using distance_type = typename space_type::distance_type;
	using snapshot_type = distance_table_snapshot<distance_type>;

	explicit DistanceTable(const space_type &space, distance_table_mode mode = distance_table_mode::eager)
		: DistanceTable(space, distance_table_options{mode})
	{
	}

	explicit DistanceTable(const space_type &space, distance_table_options options)
		: space_(&space), record_count_(space.size()), version_(space.version()), options_(options),
		  dense_distance_slots_(dense_slot_count(record_count_)),
		  memory_bytes_estimate_(initial_memory_bytes_estimate(options_.mode, record_count_)),
		  metric_key_(core::metric_cache_key(space.metric()))
	{
		enforce_budget();
		ids_ = mtrc::record_ids(space);
		id_positions_ = detail::make_record_position_lookup(ids_);
		cache_key_ = representation_cache_key("distance_table", metric_key_, version_, ids_);

		if (options_.mode == distance_table_mode::eager) {
			matrix_.resize(dense_distance_slots_);
			for (std::size_t lhs = 0; lhs < record_count_; ++lhs) {
				for (std::size_t rhs = 0; rhs < record_count_; ++rhs) {
					options_.runtime.throw_if_cancelled("distance_table materialization");
					matrix_[offset(lhs, rhs)] = space.metric()(space.records().at(lhs), space.records().at(rhs));
					++cached_count_;
				}
			}
		}
	}

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type
	{
		if (options_.mode == distance_table_mode::lazy && is_stale()) {
			throw StaleRepresentationError(
				"lazy distance table is stale: source metric space changed since construction; rebuild the table");
		}
		const auto lhs_position = position_of(lhs);
		const auto rhs_position = position_of(rhs);
		return distance_at_validated_position(lhs_position, rhs_position);
	}

	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position) const -> distance_type
	{
		if (options_.mode == distance_table_mode::lazy && is_stale()) {
			throw StaleRepresentationError(
				"lazy distance table is stale: source metric space changed since construction; rebuild the table");
		}
		validate_position(lhs_position);
		validate_position(rhs_position);
		return distance_at_validated_position(lhs_position, rhs_position);
	}

	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position,
							  runtime_guard runtime) const -> distance_type
	{
		runtime.throw_if_cancelled("distance_table lookup");
		if (options_.mode == distance_table_mode::lazy && is_stale()) {
			throw StaleRepresentationError(
				"lazy distance table is stale: source metric space changed since construction; rebuild the table");
		}
		validate_position(lhs_position);
		validate_position(rhs_position);
		return distance_at_validated_position(lhs_position, rhs_position, runtime);
	}

  private:
	auto distance_at_validated_position(std::size_t lhs_position, std::size_t rhs_position,
										runtime_guard runtime = {}) const -> distance_type
	{
		if (options_.mode == distance_table_mode::eager) {
			auto &cached = matrix_[offset(lhs_position, rhs_position)];
			if (cached.has_value()) {
				++hits_;
				return *cached;
			}
			++misses_;
			cached = compute_source_distance_at_position(lhs_position, rhs_position);
			++cached_count_;
			return *cached;
		}

		const sparse_cache_key key{lhs_position, rhs_position};
		const auto cached = sparse_cache_.find(key);
		if (cached != sparse_cache_.end()) {
			++hits_;
			return cached->second;
		}

		enforce_sparse_cache_budget(sparse_cache_.size() + 1);
		++misses_;
		runtime.throw_if_cancelled("distance_table distance evaluation");
		const auto distance = compute_source_distance_at_position(lhs_position, rhs_position);
		sparse_cache_.emplace(key, distance);
		++cached_count_;
		return distance;
	}

  public:
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
			throw std::out_of_range("record id is outside the distance table");
		}
		return position->second;
	}
	auto contains(RecordId id) const -> bool
	{
		return id_positions_.find(id) != id_positions_.end();
	}
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }
	auto cached_distances() const -> std::size_t { return cached_count_; }
	auto dense_distance_slots() const -> std::size_t { return dense_distance_slots_; }
	auto mode() const -> distance_table_mode { return options_.mode; }
	auto max_dense_records() const -> std::size_t { return options_.max_dense_records; }
	auto max_memory_bytes() const -> std::size_t { return options_.max_memory_bytes; }
	auto memory_bytes_estimate() const -> std::size_t { return current_memory_bytes_estimate(); }
	auto metric_key() const -> const std::string & { return metric_key_; }
	auto cache_key() const -> const std::string & { return cache_key_; }
	auto source_record_ids() const -> const std::vector<RecordId> & { return ids_; }

	auto snapshot() const -> snapshot_type
	{
		snapshot_type result;
		result.materialized =
			options_.mode == distance_table_mode::eager ? materialization::materialized : materialization::lazy;
		result.mode = options_.mode;
		result.built_for_version = version_;
		result.record_count = record_count_;
		result.cached_distances = cached_count_;
		result.dense_distance_slots = dense_distance_slots_;
		result.max_dense_records = options_.max_dense_records;
		result.max_memory_bytes = options_.max_memory_bytes;
		result.metric_key = metric_key_;
		result.cache_key = cache_key_;
		result.source_record_ids = ids_;
		if (options_.mode == distance_table_mode::eager) {
			result.distances = distance_table_snapshot_cells<distance_type>(
				ids_, record_count_, cached_count_,
				[this](std::size_t lhs, std::size_t rhs) -> const std::optional<distance_type> & {
					return matrix_[offset(lhs, rhs)];
				});
		} else {
			result.distances.reserve(sparse_cache_.size());
			for (const auto &cached : sparse_cache_) {
				result.distances.push_back(distance_table_snapshot_cell<distance_type>{
					ids_[cached.first.first], ids_[cached.first.second], cached.second});
			}
		}
		result.rebuild_indexes();
		return result;
	}

	auto stats() const -> distance_table_stats
	{
		distance_table_stats result;
		result.hits = hits_;
		result.misses = misses_;
		result.fill_ratio = fill_ratio();
		result.symmetric_storage = false;
		return result;
	}

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::distance_table, exactness::exact,
										  options_.mode == distance_table_mode::eager ? materialization::materialized
																					: materialization::lazy,
										  update_mode::snapshot};
		result.space_version = space_->version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.records = record_count_;
		result.distance_evaluations = cached_count_;
		result.cached_distances = cached_count_;
		result.dense_distance_slots = dense_distance_slots_;
		result.max_dense_records = options_.max_dense_records;
		result.max_memory_bytes = options_.max_memory_bytes;
		result.memory_bytes_estimate = memory_bytes_estimate();
		result.cache_key = cache_key_;
		result.metric_key = metric_key_;
		result.source_record_ids = ids_;
		if (result.stale) {
			result.warnings.push_back("distance table was built for an older metric-space version");
		}
		return result;
	}

  private:
	using sparse_cache_key = std::pair<std::size_t, std::size_t>;
	using sparse_cache_type = std::map<sparse_cache_key, distance_type>;

	auto offset(std::size_t lhs_position, std::size_t rhs_position) const -> std::size_t
	{
		return lhs_position * record_count_ + rhs_position;
	}

	static auto dense_slot_count(std::size_t record_count) -> std::size_t
	{
		return detail::distance_table_dense_slot_count(record_count);
	}

	static auto initial_memory_bytes_estimate(distance_table_mode mode, std::size_t record_count) -> std::size_t
	{
		if (mode == distance_table_mode::eager) {
			return estimate_distance_table_memory_bytes<distance_type>(record_count);
		}
		return detail::checked_distance_table_size_sum(
			detail::distance_table_record_id_storage_bytes(record_count),
			detail::distance_table_position_index_storage_bytes(record_count),
			"distance table memory estimate exceeds size_t capacity");
	}

	auto enforce_budget() const -> void
	{
		if (options_.mode == distance_table_mode::eager && options_.max_dense_records > 0 &&
			record_count_ > options_.max_dense_records) {
			throw RepresentationError(
				"distance table dense storage exceeds max_dense_records: records=" +
				std::to_string(record_count_) + " estimated_bytes=" + std::to_string(memory_bytes_estimate_) +
				" max_dense_records=" + std::to_string(options_.max_dense_records) +
				"; use live distances, exact scan, or a sparse index for larger spaces");
		}
		if (options_.max_memory_bytes > 0 && memory_bytes_estimate_ > options_.max_memory_bytes) {
			throw RepresentationError(
				"distance table dense storage exceeds max_memory_bytes: records=" +
				std::to_string(record_count_) + " estimated_bytes=" + std::to_string(memory_bytes_estimate_) +
				" max_memory_bytes=" + std::to_string(options_.max_memory_bytes) +
				"; use live distances, exact scan, or a sparse index for larger spaces");
		}
	}

	auto enforce_sparse_cache_budget(std::size_t projected_cached_distances) const -> void
	{
		if (options_.max_memory_bytes == 0) {
			return;
		}
		const auto projected_memory_bytes = memory_bytes_estimate_for(matrix_.size(), projected_cached_distances);
		if (projected_memory_bytes > options_.max_memory_bytes) {
			throw RepresentationError(
				"distance table sparse storage exceeds max_memory_bytes: records=" +
				std::to_string(record_count_) +
				" cached_distances=" + std::to_string(projected_cached_distances) +
				" projected_bytes=" + std::to_string(projected_memory_bytes) +
				" max_memory_bytes=" + std::to_string(options_.max_memory_bytes) +
				"; use a smaller query batch, exact scan, or an approximate index for larger spaces");
		}
	}

	auto fill_ratio() const -> double
	{
		if (record_count_ == 0) {
			return 1.0;
		}
		const auto possible_distances = static_cast<double>(record_count_) * static_cast<double>(record_count_);
		return static_cast<double>(cached_count_) / possible_distances;
	}

	auto current_memory_bytes_estimate() const -> std::size_t
	{
		return memory_bytes_estimate_for(matrix_.size(), sparse_cache_.size());
	}

	auto memory_bytes_estimate_for(std::size_t dense_slots, std::size_t sparse_cached_distances) const
		-> std::size_t
	{
		const auto dense_bytes = detail::checked_distance_table_size_product(
			dense_slots, sizeof(std::optional<distance_type>),
			"distance table memory estimate exceeds size_t capacity");
		const auto sparse_bytes = detail::checked_distance_table_size_product(
			sparse_cached_distances, sizeof(typename sparse_cache_type::value_type),
			"distance table memory estimate exceeds size_t capacity");
		const auto id_bytes = detail::checked_distance_table_size_product(
			ids_.size(), sizeof(RecordId), "distance table memory estimate exceeds size_t capacity");
		const auto index_bytes = detail::checked_distance_table_size_product(
			id_positions_.size(), sizeof(detail::record_position_lookup_type::value_type),
			"distance table memory estimate exceeds size_t capacity");
		return detail::checked_distance_table_size_sum(
			detail::checked_distance_table_size_sum(
				dense_bytes, sparse_bytes, "distance table memory estimate exceeds size_t capacity"),
			detail::checked_distance_table_size_sum(
				id_bytes, index_bytes, "distance table memory estimate exceeds size_t capacity"),
			"distance table memory estimate exceeds size_t capacity");
	}

	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the distance table");
		}
	}

	auto compute_source_distance_at_position(std::size_t lhs_position, std::size_t rhs_position) const
		-> distance_type
	{
		return space_->metric()(space_->records().at(lhs_position), space_->records().at(rhs_position));
	}

	const space_type *space_;
	std::size_t record_count_;
	std::size_t version_;
	distance_table_options options_;
	std::size_t dense_distance_slots_;
	std::size_t memory_bytes_estimate_;
	std::string metric_key_;
	std::string cache_key_;
	std::vector<RecordId> ids_;
	detail::record_position_lookup_type id_positions_;
	mutable std::vector<std::optional<distance_type>> matrix_;
	mutable sparse_cache_type sparse_cache_;
	mutable std::size_t cached_count_{};
	mutable std::size_t hits_{};
	mutable std::size_t misses_{};
};

template <typename Space>
auto matrix(const Space &space, distance_table_mode mode = distance_table_mode::eager) -> DistanceTable<Space>
{
	return DistanceTable<Space>(space, mode);
}

template <typename Space> auto matrix(const Space &space, distance_table_options options) -> DistanceTable<Space>
{
	return DistanceTable<Space>(space, options);
}

template <typename Space> auto snapshot(const DistanceTable<Space> &cache) -> typename DistanceTable<Space>::snapshot_type
{
	return cache.snapshot();
}

} // namespace mtrc::space::storage

#endif
