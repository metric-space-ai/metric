// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_METRIC_TRAITS_HPP
#define _METRIC_CORE_METRIC_TRAITS_HPP

#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace mtrc::core {

enum class metric_law {
	distance,
	metric,
	pseudo_metric,
	unknown,
};

// Audit-friendly name for a metric_law value. Used by derived-space results and
// tests to report the metric status of a constructed or modified finite metric
// space without depending on RTTI.
inline auto metric_law_name(metric_law law) -> const char *
{
	switch (law) {
	case metric_law::distance:
		return "distance";
	case metric_law::metric:
		return "metric";
	case metric_law::pseudo_metric:
		return "pseudo_metric";
	case metric_law::unknown:
		return "unknown";
	}
	return "unknown";
}

enum class record_kind {
	custom,
	vector,
	aligned_vector,
	string,
	sequence,
	structured,
};

template <typename Metric> struct metric_traits {
	static constexpr auto law = metric_law::distance;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

template <typename Metric> constexpr bool metric_thread_safe_v = metric_traits<Metric>::thread_safe;

namespace detail {

template <typename Metric, typename = void> struct metric_cache_key_provider {
	static auto get(const Metric &) -> std::string { return std::string("metric_type:") + typeid(Metric).name(); }
};

template <typename Metric>
struct metric_cache_key_provider<
	Metric, std::void_t<decltype(metric_traits<Metric>::cache_key(std::declval<const Metric &>()))>> {
	static auto get(const Metric &metric) -> std::string { return metric_traits<Metric>::cache_key(metric); }
};

} // namespace detail

template <typename Metric> auto metric_cache_key(const Metric &metric) -> std::string
{
	return detail::metric_cache_key_provider<Metric>::get(metric);
}

} // namespace mtrc::core

namespace mtrc {
using core::metric_cache_key;
using core::metric_law;
using core::metric_law_name;
using core::metric_thread_safe_v;
using core::metric_traits;
using core::record_kind;
} // namespace mtrc

#endif
