// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_METRIC_SPACE_HPP
#define _METRIC_CORE_METRIC_SPACE_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "concepts.hpp"
#include "record_id.hpp"
#include "version.hpp"

namespace metric::core {

template <typename Record, typename Metric> class MetricSpace {
  public:
	using record_type = Record;
	using metric_type = Metric;
	using distance_type = metric_result_t<metric_type, record_type>;

	MetricSpace(std::vector<record_type> records, metric_type metric)
		: records_(std::move(records))
		, metric_(std::move(metric))
		, ids_(make_initial_ids(records_.size()))
		, next_record_id_(records_.size())
	{
		static_assert(MetricCallable_v<metric_type, record_type>,
					  "metric::core::MetricSpace requires a metric callable accepting two records");
	}

	auto size() const -> std::size_t { return records_.size(); }
	auto empty() const -> bool { return records_.empty(); }
	auto version() const -> SpaceVersion { return version_; }
	auto touch() -> void { version_ = next_space_version(version_); }

	auto id(std::size_t position) const -> RecordId { return id_at(position); }

	auto id_at(std::size_t position) const -> RecordId
	{
		validate_position(position);
		return ids_[position];
	}

	auto contains(RecordId id) const -> bool
	{
		for (const auto current : ids_) {
			if (current == id) {
				return true;
			}
		}
		return false;
	}

	auto position_of(RecordId id) const -> std::size_t
	{
		for (std::size_t position = 0; position < ids_.size(); ++position) {
			if (ids_[position] == id) {
				return position;
			}
		}
		throw std::out_of_range("record id is not in the metric space");
	}

	auto record(RecordId id) const -> const record_type & { return records_.at(position_of(id)); }
	auto operator[](RecordId id) const -> const record_type & { return record(id); }

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type
	{
		return metric_(record(lhs), record(rhs));
	}

	auto operator()(RecordId lhs, RecordId rhs) const -> distance_type { return distance(lhs, rhs); }

	auto records() const -> const std::vector<record_type> & { return records_; }
	auto metric() const -> const metric_type & { return metric_; }

	auto insert(record_type record) -> RecordId
	{
		const auto id = RecordId::from_index(next_record_id_++);
		records_.push_back(std::move(record));
		ids_.push_back(id);
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
		for (std::size_t position = 0; position < ids_.size(); ++position) {
			if (ids_[position] == id) {
				records_.erase(records_.begin() + static_cast<std::ptrdiff_t>(position));
				ids_.erase(ids_.begin() + static_cast<std::ptrdiff_t>(position));
				version_ = next_space_version(version_);
				return true;
			}
		}
		return false;
	}

  private:
	static auto make_initial_ids(std::size_t count) -> std::vector<RecordId>
	{
		std::vector<RecordId> ids;
		ids.reserve(count);
		for (std::size_t index = 0; index < count; ++index) {
			ids.push_back(RecordId::from_index(index));
		}
		return ids;
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

} // namespace metric::core

namespace metric {
using core::make_space;
using core::MetricSpace;
} // namespace metric

#endif
