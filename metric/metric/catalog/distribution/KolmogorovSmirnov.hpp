/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RANDOM_KOLMOGOROV_SMIRNOV_HPP
#define _METRIC_DISTANCE_K_RANDOM_KOLMOGOROV_SMIRNOV_HPP

namespace mtrc {

/**
 * @brief
 *
 * To compare the two samples, we construct their empirical cumulative distribution functions (CDF). CDFs is calculated
 * by PMQ. The Kolmogorov-Smirnov (KS) distance is defined to be the largest absolute difference between the two
 * empirical CDFs evaluated at any point.
 *
 * @warning QUARANTINED — NOT an admitted true metric (`metric_traits<>::law ==
 * metric_law::distance`). The true sup-CDF (L∞) distance is a metric, but the
 * shipped class takes the max only over the concatenated data points of PMQ's
 * Akima-interpolated empirical CDF, which overshoots `[0,1]`. As shipped it
 * violates the triangle inequality, returns values `> 1`, and silently returns
 * `0` for tied or fully disjoint all-equal samples (false identity). See
 * `docs/metrics/quarantine-inventory.md` and
 * `tests/core_smoke/metric_distribution_quarantine_smoke.cpp`.
 *
 * @tparam Sample - sample type
 * @tparam D - distance return type
 */
template <typename Sample, typename D = double> class KolmogorovSmirnov {
  public:
	using distance_type = D;

	/**
	 * @brief Construct a new Kolmogorov-Smirnov object
	 *
	 */
	explicit KolmogorovSmirnov() = default;

	/**
	 * @brief calculate Kolmogorov-Smirnov distance between two samples
	 *
	 * @param sample_1 first sample
	 * @param sample_2 second sample
	 * @return distance
	 */
	[[deprecated("KolmogorovSmirnov is a quarantined statistical divergence, not a true metric; "
				 "use an admitted metric (mtrc::Wasserstein / mtrc::EMD) for metric geometry")]]
	distance_type operator()(const Sample &sample_1, const Sample &sample_2) const;
};

} // namespace mtrc

#include "KolmogorovSmirnov.cpp"
#endif
