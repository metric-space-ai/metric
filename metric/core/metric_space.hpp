// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_METRIC_SPACE_HPP
#define _METRIC_CORE_METRIC_SPACE_HPP

#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <metric/record/id.hpp>

#include "concepts.hpp"
#include "metric_traits.hpp"
#include "version.hpp"

namespace mtrc::core {

template <typename Record, typename Metric> class MetricSpace {
  public:
	using record_type = Record;
	using metric_type = Metric;
	using distance_type = metric_result_t<metric_type, record_type>;

	MetricSpace(std::vector<record_type> records, metric_type metric)
		: records_(std::move(records)), metric_(std::move(metric)), ids_(make_initial_ids(records_.size())),
		  id_positions_(make_position_lookup(records_.size(), ids_)), next_record_id_(records_.size())
	{
		static_assert(MetricCallable_v<metric_type, record_type>,
					  "mtrc::core::MetricSpace requires a metric callable accepting two records");
	}

	MetricSpace(std::vector<record_type> records, metric_type metric, std::vector<RecordId> ids,
				std::size_t next_record_id, SpaceVersion version = initial_space_version)
		: records_(std::move(records)), metric_(std::move(metric)), ids_(std::move(ids)),
		  id_positions_(make_position_lookup(records_.size(), ids_)),
		  next_record_id_(validate_next_record_id(ids_, next_record_id)), version_(version)
	{
		static_assert(MetricCallable_v<metric_type, record_type>,
					  "mtrc::core::MetricSpace requires a metric callable accepting two records");
	}

	auto size() const -> std::size_t { return records_.size(); }
	auto empty() const -> bool { return records_.empty(); }
	auto version() const -> SpaceVersion { return version_; }
	auto next_record_id() const -> std::size_t { return next_record_id_; }
	auto touch() -> void { version_ = next_space_version(version_); }

	auto id(std::size_t position) const -> RecordId { return id_at(position); }

	auto id_at(std::size_t position) const -> RecordId
	{
		validate_position(position);
		return ids_[position];
	}

	auto contains(RecordId id) const -> bool
	{
		return id_positions_.find(id) != id_positions_.end();
	}

	auto position_of(RecordId id) const -> std::size_t
	{
		const auto position = id_positions_.find(id);
		if (position == id_positions_.end()) {
			throw std::out_of_range("record id is not in the metric space");
		}
		return position->second;
	}

	auto record(RecordId id) const -> const record_type & { return records_.at(position_of(id)); }
	auto operator[](RecordId id) const -> const record_type & { return record(id); }

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type { return metric_(record(lhs), record(rhs)); }

	auto operator()(RecordId lhs, RecordId rhs) const -> distance_type { return distance(lhs, rhs); }

	auto records() const -> const std::vector<record_type> & { return records_; }
	auto metric() const -> const metric_type & { return metric_; }

	auto insert(record_type record) -> RecordId
	{
		const auto id = RecordId::from_index(next_record_id_);
		const auto position = records_.size();
		id_positions_.reserve(ids_.size() + 1);
		const auto inserted = id_positions_.emplace(id, position);
		if (!inserted.second) {
			throw std::logic_error("metric space RecordId generator reused an existing RecordId");
		}
		bool record_inserted = false;
		try {
			records_.push_back(std::move(record));
			record_inserted = true;
			ids_.push_back(id);
		} catch (...) {
			if (record_inserted) {
				records_.pop_back();
			}
			id_positions_.erase(id);
			throw;
		}
		++next_record_id_;
		version_ = next_space_version(version_);
		return id;
	}

	auto replace(RecordId id, record_type record) -> void
	{
		records_[position_of(id)] = std::move(record);
		version_ = next_space_version(version_);
	}

	auto erase(RecordId id) -> bool
	{
		const auto found = id_positions_.find(id);
		if (found == id_positions_.end()) {
			return false;
		}
		const auto position = found->second;
		records_.erase(records_.begin() + static_cast<std::ptrdiff_t>(position));
		ids_.erase(ids_.begin() + static_cast<std::ptrdiff_t>(position));
		id_positions_.erase(found);
		refresh_positions_from(position);
		version_ = next_space_version(version_);
		return true;
	}

  private:
	using position_lookup_type = std::unordered_map<RecordId, std::size_t>;

	static auto make_initial_ids(std::size_t count) -> std::vector<RecordId>
	{
		std::vector<RecordId> ids;
		ids.reserve(count);
		for (std::size_t index = 0; index < count; ++index) {
			ids.push_back(RecordId::from_index(index));
		}
		return ids;
	}

	static auto make_position_lookup(std::size_t record_count, const std::vector<RecordId> &ids)
		-> position_lookup_type
	{
		if (ids.size() != record_count) {
			throw std::invalid_argument("metric space identity needs one RecordId per record");
		}

		position_lookup_type positions;
		positions.reserve(ids.size());
		for (std::size_t position = 0; position < ids.size(); ++position) {
			const auto inserted = positions.emplace(ids[position], position);
			if (!inserted.second) {
				throw std::invalid_argument("metric space identity contains duplicate RecordIds");
			}
		}
		return positions;
	}

	static auto validate_next_record_id(const std::vector<RecordId> &ids, std::size_t next_record_id) -> std::size_t
	{
		std::size_t minimum_next_id = 0;
		for (const auto id : ids) {
			if (id.index() >= minimum_next_id) {
				minimum_next_id = id.index() + 1;
			}
		}
		if (next_record_id < minimum_next_id) {
			throw std::invalid_argument("metric space next RecordId would reuse an existing RecordId");
		}
		return next_record_id;
	}

	auto refresh_positions_from(std::size_t first_position) -> void
	{
		for (std::size_t position = first_position; position < ids_.size(); ++position) {
			id_positions_.at(ids_[position]) = position;
		}
	}

	auto validate_position(std::size_t position) const -> void
	{
		if (position >= records_.size()) {
			throw std::out_of_range("record position is outside the metric space");
		}
	}

	std::vector<record_type> records_;
	metric_type metric_;
	std::vector<RecordId> ids_;
	position_lookup_type id_positions_;
	std::size_t next_record_id_{};
	SpaceVersion version_{initial_space_version};
};

template <typename Container, typename Metric>
auto make_space(Container records, Metric metric)
	-> MetricSpace<typename std::decay<typename Container::value_type>::type, typename std::decay<Metric>::type>
{
	using record_type = typename std::decay<typename Container::value_type>::type;
	using metric_type = typename std::decay<Metric>::type;
	return MetricSpace<record_type, metric_type>(std::vector<record_type>(records.begin(), records.end()),
												 std::move(metric));
}

// One-line summary of a finite metric space: its size and the metric law its
// metric is admitted under. Lets a caller `std::cout << space;` to see what they
// hold, complementing describe_structure() (which computes the geometry).
template <typename Record, typename Metric>
auto operator<<(std::ostream &os, const MetricSpace<Record, Metric> &space) -> std::ostream &
{
	os << "MetricSpace(records=" << space.size()
	   << ", metric_law=" << metric_law_name(metric_traits<Metric>::law) << ')';
	return os;
}

} // namespace mtrc::core

namespace mtrc {
using core::make_space;
using core::MetricSpace;
} // namespace mtrc

#endif
