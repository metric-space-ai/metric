// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_METRIC_TRAITS_HPP
#define _METRIC_CORE_METRIC_TRAITS_HPP

namespace metric::core {

enum class metric_law {
	distance,
	metric,
	pseudo_metric,
	unknown,
};

enum class record_kind {
	custom,
	vector,
	string,
	sequence,
	structured,
};

template <typename Metric> struct metric_traits {
	static constexpr auto law = metric_law::distance;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

template <typename Metric>
constexpr bool metric_thread_safe_v = metric_traits<Metric>::thread_safe;

} // namespace metric::core

namespace metric {
using core::metric_law;
using core::metric_thread_safe_v;
using core::metric_traits;
using core::record_kind;
} // namespace metric

#endif
