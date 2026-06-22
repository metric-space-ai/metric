/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RANDOM_CRAMER_VON_MISES_HPP
#define _METRIC_DISTANCE_K_RANDOM_CRAMER_VON_MISES_HPP

namespace mtrc {

/**
 * @brief
 *
 * The Cramér-von Mises (CM) distance is obtained by summing the squared difference between the two empirical CDFs along
 * the x-axis and then taking the square root of the sum (the root-L2-CDF form, not the squared CvM statistic). The
 * relationship between the CM distance and the EMD is analogous to that of the L1 and L2 norms. CDFs is calculated by PMQ.
 *
 * @warning QUARANTINED — NOT an admitted true metric (`metric_traits<>::law ==
 * metric_law::distance`). The root-L2-CDF *form* is the correct metric
 * candidate, but the shipped computation uses PMQ's Akima-interpolated empirical
 * CDF, which is not a valid CDF (it overshoots `[0,1]`). As shipped it returns
 * `NaN` on tied sample values (so even `d(a,a)` is `NaN` for samples with
 * repeats), loops forever on zero-range samples, and its triangle inequality is
 * broken by the CDF overshoot. Admission requires a tie-safe empirical CDF over
 * a fixed support. See `docs/metrics/quarantine-inventory.md` and
 * `tests/core_smoke/metric_distribution_quarantine_smoke.cpp`.
 *
 * @tparam Sample - sample type
 * @tparam D - distance return type
 */
template <typename Sample, typename D = double> class CramervonMises {
  public:
	using distance_type = D;

	/**
	 * @brief Construct a new Cramer-von Nises object
	 *
	 */
	explicit CramervonMises() = default;

	/**
	 * @brief Construct a new Cramer-von Nises object
	 *
	 * @param precision  for integration. Should be in (0, 1). Less means more accurate.
	 */
	explicit CramervonMises(double precision) : precision(precision) {}

	/**
	 * @brief calculate Cramer-von Nises distance between two samples
	 *
	 * @param sample_1 first sample
	 * @param sample_2 second sample
	 * @return distance
	 */
	distance_type operator()(const Sample &sample_1, const Sample &sample_2) const;

  private:
	double precision = 0.01;
};

} // namespace mtrc

#include "CramervonMises.cpp"
#endif
