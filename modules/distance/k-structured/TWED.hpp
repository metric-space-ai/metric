/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_K_RANDOM_TWED_HPP
#define _METRIC_DISTANCE_K_RANDOM_TWED_HPP

#include "../../../3rdparty/blaze/Math.h"

namespace metric {

/**
 * @class TWED
 *
 * @brief Time warp Elastic Distance (for curves)
 */
template <typename V>
struct TWED {
    using value_type = V;
    using distance_type = value_type;

    /**
     * @brief Construct a new TWED object
     *
     * @param penalty_
     * @param elastic_
     */
    TWED(const value_type& penalty_, const value_type& elastic_)
        : penalty(penalty_)
        , elastic(elastic_)
    {
    }

    /**
     * @brief Calculate TWE distance between given containers
     *
     * @param As first container
     * @param Bs second container
     * @return TWE distance between given containers
     */
    template <typename Container>
    value_type operator()(const Container& As, const Container& Bs) const;

    value_type penalty = 0;
    value_type elastic = 1;
    bool is_zero_padded = false;
};

}  // namespace metric

#include "TWED.cpp"

#endif  // Header Guard
