/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef _METRIC_METRIC_CATALOG_STRUCTURED_GEOMETRY_HPP
#define _METRIC_METRIC_CATALOG_STRUCTURED_GEOMETRY_HPP

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include <metric/core/metric_traits.hpp>
#include <metric/metric/catalog/vector/Standards.hpp>

namespace mtrc {

/**
 * Hausdorff metric on non-empty finite set records.
 *
 * Formula:
 *   d_H(A, B) = max{sup_a inf_b d(a,b), sup_b inf_a d(a,b)}
 *
 * The default ground metric is Euclidean. For another ground metric, the
 * Hausdorff trait is promoted only when that ground metric is also admitted by
 * `metric_traits`. Input containers are treated as finite set representations:
 * order and duplicate points do not change the set value.
 */
template <typename GroundMetric = Euclidean<double>> class Hausdorff {
  public:
	using ground_metric_type = GroundMetric;
	using distance_type = double;

	explicit Hausdorff(GroundMetric ground_metric = GroundMetric()) : ground_metric_(std::move(ground_metric)) {}

	template <typename Set> auto operator()(const Set &a, const Set &b) const -> distance_type
	{
		if (a.empty() || b.empty()) {
			throw std::invalid_argument("Hausdorff requires non-empty finite sets");
		}
		return std::max(directed(a, b), directed(b, a));
	}

	auto ground_metric() const -> const GroundMetric & { return ground_metric_; }

  private:
	GroundMetric ground_metric_;

	template <typename Set> auto directed(const Set &from, const Set &to) const -> distance_type
	{
		distance_type worst = 0;
		for (const auto &point : from) {
			distance_type nearest = std::numeric_limits<distance_type>::infinity();
			for (const auto &candidate : to) {
				const auto value = static_cast<distance_type>(ground_metric_(point, candidate));
				if (!std::isfinite(value) || value < 0.0) {
					throw std::invalid_argument("Hausdorff ground metric returned a non-finite or negative value");
				}
				nearest = std::min(nearest, value);
			}
			worst = std::max(worst, nearest);
		}
		return worst;
	}
};

} // namespace mtrc

namespace mtrc::core {

template <typename GroundMetric> struct metric_traits<::mtrc::Hausdorff<GroundMetric>> {
	static constexpr auto law =
		metric_traits<GroundMetric>::law == metric_law::metric ? metric_law::metric : metric_law::distance;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = metric_traits<GroundMetric>::thread_safe;

	static auto cache_key(const ::mtrc::Hausdorff<GroundMetric> &metric) -> std::string
	{
		return std::string("mtrc::Hausdorff:ground=") + metric_cache_key(metric.ground_metric());
	}
};

} // namespace mtrc::core

#endif
