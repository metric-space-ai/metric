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

namespace metric::core {

template <typename Record, typename Metric> class MetricSpace {
  public:
	using record_type = Record;
	using metric_type = Metric;
	using distance_type = metric_result_t<metric_type, record_type>;

	MetricSpace(std::vector<record_type> records, metric_type metric)
		: records_(std::move(records))
		, metric_(std::move(metric))
	{
		static_assert(MetricCallable_v<metric_type, record_type>,
					  "metric::core::MetricSpace requires a metric callable accepting two records");
	}

	auto size() const -> std::size_t { return records_.size(); }
	auto empty() const -> bool { return records_.empty(); }
	auto version() const -> std::size_t { return version_; }
	auto touch() -> void { ++version_; }

	auto id(std::size_t index) const -> RecordId
	{
		validate_index(index);
		return RecordId::from_index(index);
	}

	auto record(RecordId id) const -> const record_type & { return records_.at(id.index()); }
	auto operator[](RecordId id) const -> const record_type & { return record(id); }

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type
	{
		return metric_(record(lhs), record(rhs));
	}

	auto operator()(RecordId lhs, RecordId rhs) const -> distance_type { return distance(lhs, rhs); }

	auto records() const -> const std::vector<record_type> & { return records_; }
	auto metric() const -> const metric_type & { return metric_; }

  private:
	auto validate_index(std::size_t index) const -> void
	{
		if (index >= records_.size()) {
			throw std::out_of_range("record id is outside the metric space");
		}
	}

	std::vector<record_type> records_;
	metric_type metric_;
	std::size_t version_{};
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
