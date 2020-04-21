/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_K_RELATED_STANDARDS_HPP
#define _METRIC_DISTANCE_K_RELATED_STANDARDS_HPP

#include <cmath>

#include "../../utils.hpp"


namespace metric {



template <typename RT>
class Euclidean {

public:

    using RecordType = RT;
    using ValueType = determine_ValueType<RecordType>;

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value && determine_container_type<R>::code != 3, // non-Eigen type
     DistanceType
    >::type
    operator()(const R & a, const R & b) const {
        if (a.size() != b.size()) {
            DistanceType dnan = std::nan("Input container sizes do not match");
            return dnan;
        }
        DistanceType sum = 0;
        for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
            sum += (*it1 - *it2) * (*it1 - *it2);
        }
        return std::sqrt(sum);
    }


    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value && determine_container_type<R>::code == 3, // Eigen, [] to access elements (or we can use Eigen-specific matrix operations)
     DistanceType
    >::type
    operator()(const R & a, const R & b) const {
        DistanceType sum = 0;
        if (a.size() != b.size()) {
            DistanceType dnan = std::nan("Input container sizes do not match");
            return dnan;
        }
        for (size_t i = 0; i < a.size(); ++i) {
            sum += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(sum);
    }

};



}  // namespace metric

#include "Standards.cpp"

#endif  // Header Guard
