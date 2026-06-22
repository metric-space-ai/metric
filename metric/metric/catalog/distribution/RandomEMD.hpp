/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RANDOM_EMD_HPP
#define _METRIC_DISTANCE_K_RANDOM_EMD_HPP

namespace mtrc {

/**
 * @brief
 *
 * Earth Mover�s Distance (EMD), also known as the first Wasserstein distance.
 * �hysical interpretation �� the EMDis easy to understand: imagine the two datasets to be piles of earth, and the goal
 * is to move the first pile around to match the second. The Earth Mover�s Distance is the minimum amount of work
 * involved, where �amount of work� is the amount of earth you have to move multiplied by the distance you have to move
 * it. The EMD can also be shown to be equal to the area between the two empirical CDFs, which is calculated py PMQ.
 *
 * @warning QUARANTINED — NOT an admitted true metric (`metric_traits<>::law ==
 * metric_law::distance`). The class only *approximates* the 1-Wasserstein
 * distance: it integrates `|F1 - F2|` over a pair-dependent grid of the Akima-
 * interpolated empirical CDF from PMQ, which is not a valid CDF (it overshoots
 * `[0,1]`). As shipped it violates the triangle inequality, returns `NaN` on
 * tied sample values, and loops forever on zero-range (all-equal) samples. Use
 * the admitted `mtrc::Wasserstein<V>` for a true transport metric. See
 * `docs/metrics/quarantine-inventory.md` and
 * `tests/core_smoke/metric_distribution_quarantine_smoke.cpp`.
 *
 * @tparam Sample - sample type
 * @tparam D - distance return type
 */
template <typename Sample, typename D = double> struct RandomEMD {
	using distance_type = D;

	/**
	 * @brief Construct a new EMD object
	 *
	 */
	explicit RandomEMD() = default;

	/**
	 * @brief Construct a new EMD object
	 *
	 * @param precision used for integration. Should be in (0, 1). Less means more accurate.
	 */
	explicit RandomEMD(double precision) : precision(precision) {}

	/**
	 * @brief calculate EMD distance between two samples
	 *
	 * @param sample_1 first sample
	 * @param sample_2 second sample
	 * @return distance
	 */
	[[deprecated("RandomEMD is a quarantined non-metric (a Monte-Carlo approximation, not a true "
				 "metric) with degenerate-input failure modes; use mtrc::Wasserstein / mtrc::EMD instead")]]
	distance_type operator()(const Sample &sample_1, const Sample &sample_2) const;

  private:
	const double precision = 0.01;
};

} // namespace mtrc

#include "RandomEMD.cpp"
#endif
