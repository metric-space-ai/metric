/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RANDOM_EMD_HPP
#define _METRIC_DISTANCE_K_RANDOM_EMD_HPP

namespace metric {
	

/**
 * @brief
 *
 * Earth Mover’s Distance (EMD), also known as the first Wasserstein distance. 
 * Çhysical interpretation ùà the EMDis easy to understand: imagine the two datasets to be piles of earth, and the goal is to move the first pile around to match the second. 
 * The Earth Mover’s Distance is the minimum amount of work involved, where “amount of work” is the amount of earth you have to move multiplied by the distance you have to move it. 
 * The EMD can also be shown to be equal to the area between the two empirical CDFs, which is calculated py PMQ. 
 *
 * @tparam Sample - sample type
 * @tparam D - distance return type
 */
template <typename Sample, typename D = double>
struct RandomEMD {
    using distance_return_type = D;

    /**
     * @brief Construct a new EMD object
     *
     */
    explicit RandomEMD()
    {
    }

    /**
     * @brief Construct a new EMD object
     *
     * @param precision used for integration. Should be in (0, 1). Less means more accurate.
     */
    explicit RandomEMD(double precision) : precision_(precision)
    {
    }

    /**
     * @brief calculate EMD distance between two samples
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

#include "RandomEMD.cpp"
#endif
