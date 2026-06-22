// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_MATRIX_CACHE_HPP
#define _METRIC_REPRESENTATIONS_MATRIX_CACHE_HPP

#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/core/errors.hpp>
#include <metric/core/concepts.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/record/id.hpp>
#include "diagnostics.hpp"

namespace mtrc::space::storage {

enum class distance_table_mode {
	eager,
	lazy,
};

struct distance_table_options {
	distance_table_mode mode{distance_table_mode::eager};
	std::size_t max_dense_records{};
};

template <typename Distance> struct distance_table_snapshot_cell {
	RecordId lhs;
	RecordId rhs;
	Distance distance;
};

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
	std::string metric_key;
	std::string cache_key;
	std::vector<RecordId> source_record_ids;
	std::vector<cell_type> distances;

	auto contains(RecordId id) const -> bool
	{
		return mtrc::contains_record_id(source_record_ids, id);
	}

	auto position_of(RecordId id) const -> std::size_t
	{
		return mtrc::position_of_record_id(source_record_ids, id, "record id is outside the distance table snapshot");
	}

	auto has_distance(RecordId lhs, RecordId rhs) const -> bool
	{
		return has_distance_table_snapshot_cell(distances, lhs, rhs);
	}

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type
	{
		return distance_table_snapshot_cell_distance_or_throw(
			distances, lhs, rhs, "distance cell is not present in the distance table snapshot");
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
		: DistanceTable(space, distance_table_options{mode, 0})
	{
	}

	explicit DistanceTable(const space_type &space, distance_table_options options)
		: space_(&space), record_count_(space.size()), version_(space.version()), options_(options),
		  dense_distance_slots_(dense_slot_count(record_count_)), metric_key_(core::metric_cache_key(space.metric()))
	{
		if (options_.max_dense_records > 0 && record_count_ > options_.max_dense_records) {
			throw RepresentationError("distance table dense storage exceeds max_dense_records");
		}
		ids_ = mtrc::record_ids(space);
		cache_key_ = representation_cache_key("distance_table", metric_key_, version_, ids_);

		matrix_.resize(dense_distance_slots_);
		if (options_.mode == distance_table_mode::eager) {
			for (std::size_t lhs = 0; lhs < record_count_; ++lhs) {
				for (std::size_t rhs = 0; rhs < record_count_; ++rhs) {
					matrix_[offset(lhs, rhs)] = space.distance(ids_[lhs], ids_[rhs]);
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
		auto &cached = matrix_[offset(lhs_position, rhs_position)];
		if (cached.has_value()) {
			++hits_;
			return *cached;
		}
		++misses_;
		cached = space_->distance(lhs, rhs);
		++cached_count_;
		return *cached;
	}

	auto record_count() const -> std::size_t { return record_count_; }
	auto id(std::size_t position) const -> RecordId
	{
		validate_position(position);
		return ids_[position];
	}
	auto position_of(RecordId id) const -> std::size_t
	{
		return mtrc::position_of_record_id(ids_, id, "record id is outside the distance table");
	}
	auto contains(RecordId id) const -> bool
	{
		return mtrc::contains_record_id(ids_, id);
	}
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }
	auto cached_distances() const -> std::size_t { return cached_count_; }
	auto dense_distance_slots() const -> std::size_t { return dense_distance_slots_; }
	auto mode() const -> distance_table_mode { return options_.mode; }
	auto max_dense_records() const -> std::size_t { return options_.max_dense_records; }
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
		result.metric_key = metric_key_;
		result.cache_key = cache_key_;
		result.source_record_ids = ids_;
		result.distances = distance_table_snapshot_cells<distance_type>(
			ids_, record_count_, cached_count_,
			[this](std::size_t lhs, std::size_t rhs) -> const std::optional<distance_type> & {
				return matrix_[offset(lhs, rhs)];
			});
		return result;
	}

	auto stats() const -> distance_table_stats
	{
		distance_table_stats result;
		result.hits = hits_;
		result.misses = misses_;
		result.fill_ratio =
			matrix_.empty() ? 1.0 : static_cast<double>(cached_count_) / static_cast<double>(matrix_.size());
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
		result.memory_bytes_estimate =
			matrix_.size() * sizeof(std::optional<distance_type>) + ids_.size() * sizeof(RecordId);
		result.cache_key = cache_key_;
		result.metric_key = metric_key_;
		result.source_record_ids = ids_;
		if (result.stale) {
			result.warnings.push_back("distance table was built for an older metric-space version");
		}
		return result;
	}

  private:
	auto offset(std::size_t lhs_position, std::size_t rhs_position) const -> std::size_t
	{
		return lhs_position * record_count_ + rhs_position;
	}

	static auto dense_slot_count(std::size_t record_count) -> std::size_t
	{
		if (record_count != 0 && record_count > std::numeric_limits<std::size_t>::max() / record_count) {
			throw RepresentationError("distance table dense storage exceeds size_t capacity");
		}
		return record_count * record_count;
	}

	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the distance table");
		}
	}

	const space_type *space_;
	std::size_t record_count_;
	std::size_t version_;
	distance_table_options options_;
	std::size_t dense_distance_slots_;
	std::string metric_key_;
	std::string cache_key_;
	std::vector<RecordId> ids_;
	mutable std::vector<std::optional<distance_type>> matrix_;
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
