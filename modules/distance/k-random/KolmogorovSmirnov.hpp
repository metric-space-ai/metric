/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RANDOM_KOLMOGOROV_SMIRNOV_HPP
#define _METRIC_DISTANCE_K_RANDOM_KOLMOGOROV_SMIRNOV_HPP

namespace metric {
	

/**
 * @brief
 *
 * To compare the two samples, we construct their empirical cumulative distribution functions (CDF). CDFs is calculated by PMQ. 
 * The Kolmogorov-Smirnov (KS) distance is defined to be the largest absolute difference between the two empirical CDFs evaluated at any point.
 * 
 * @tparam Sample - sample type
 * @tparam D - distance return type
 */
template <typename Sample, typename D = double>
struct KolmogorovSmirnov {
    using distance_return_type = D;

    /**
     * @brief Construct a new Kolmogorov-Smirnov object
     *
     */
    explicit KolmogorovSmirnov()
    {
    }

    /**
     * @brief calculate Kolmogorov-Smirnov distance between two samples
     *
     * @param sample_1 first sample
     * @param sample_2 second sample
     * @return distance
     */
    distance_return_type operator()(const Sample& sample_1, const Sample& sample_2);
};


}  // namespace metric

#include "KolmogorovSmirnov.cpp"
#endif
