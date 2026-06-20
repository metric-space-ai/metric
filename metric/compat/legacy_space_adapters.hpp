// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_COMPAT_LEGACY_SPACE_ADAPTERS_HPP
#define _METRIC_COMPAT_LEGACY_SPACE_ADAPTERS_HPP

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include "../core/metric_space.hpp"
#include "../core/record_id.hpp"

namespace metric::compat {

inline constexpr auto record_id_from_legacy_index(std::size_t index) -> metric::RecordId
{
	return metric::RecordId::from_index(index);
}

inline constexpr auto legacy_index(metric::RecordId id) -> std::size_t { return id.index(); }

namespace detail {
template <typename LegacySpace>
using legacy_record_t =
	std::decay_t<decltype(std::declval<const LegacySpace &>()[std::declval<std::size_t>()])>;
} // namespace detail

template <typename LegacySpace>
auto legacy_records(const LegacySpace &legacy) -> std::vector<detail::legacy_record_t<LegacySpace>>
{
	std::vector<detail::legacy_record_t<LegacySpace>> records;
	records.reserve(legacy.size());
	for (std::size_t index = 0; index < legacy.size(); ++index) {
		records.push_back(legacy[index]);
	}
	return records;
}

template <typename LegacySpace, typename Metric>
auto to_metric_space(const LegacySpace &legacy, Metric metric)
	-> metric::MetricSpace<detail::legacy_record_t<LegacySpace>, std::decay_t<Metric>>
{
	return metric::make_space(legacy_records(legacy), std::move(metric));
}

} // namespace metric::compat

#endif
