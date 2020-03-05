/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RANDOM_CRAMER_VON_NISES_HPP
#define _METRIC_DISTANCE_K_RANDOM_CRAMER_VON_NISES_HPP

namespace metric {
	

/**
 * @brief
 *
 * The Cramér-von Mises (CM) distance is obtained by summing the squared difference between the two empirical CDFs along the x-axis (and then taking the square root of the sum to make it an actual distance). 
 * The relationship between the CM distance and the EMD is analogous to that of the L1 and L2 norms. 
 * CDFs is calculated by PMQ.
 * 
 * @tparam Sample - sample type
 * @tparam D - distance return type
 */
template <typename Sample, typename D = double>
struct CramervonNises {
    using distance_return_type = D;

    /**
     * @brief Construct a new Cramer-von Nises object
     *
     */
    explicit CramervonNises()
    {
    }

    /**
     * @brief Construct a new Cramer-von Nises object
     *
     * @param precision  for integration. Should be in (0, 1). Less means more accurate.
     */
    explicit CramervonNises(double precision) : precision_(precision)
    {
    }

    /**
     * @brief calculate Cramer-von Nises distance between two samples
     *
     * @param sample_1 first sample
     * @param sample_2 second sample
     * @return distance
     */
    distance_return_type operator()(const Sample& sample_1, const Sample& sample_2);

private:
	double precision_ = 0.01;
};


}  // namespace metric

#include "CramervonNises.cpp"
#endif
