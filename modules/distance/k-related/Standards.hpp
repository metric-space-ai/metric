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
     std::is_same<R, RT>::value
      && determine_container_type<R>::code != 2  // non-Blaze type
      && determine_container_type<R>::code != 3, // non-Eigen type
     DistanceType
    >::type
    operator()(const R & a, const R & b) const;

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value && determine_container_type<R>::code == 2, // Blaze vectors and sparse matrices
     DistanceType
    >::type
    operator()(const R & a, const R & b) const;


    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value && determine_container_type<R>::code == 3, // Eigen, [] to access elements (or we can use Eigen-specific matrix operations)
     DistanceType
    >::type
    operator()(const R & a, const R & b) const;

};



template <typename RT>
class Euclidean_thresholded {

public:

    using RecordType = RT;
    using ValueType = determine_ValueType<RecordType>;

    Euclidean_thresholded(DistanceType thres_, double factor_)
        : thres(thres_)
        , factor(factor_)
    {}

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value
      && determine_container_type<R>::code != 2  // non-Blaze type
      && determine_container_type<R>::code != 3, // non-Eigen type
     DistanceType
    >::type
    operator()(const R & a, const R & b) const;

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value && determine_container_type<R>::code == 2, // Blaze vectors and sparse matrices
     DistanceType
    >::type
    operator()(const R & a, const R & b) const;


    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value && determine_container_type<R>::code == 3, // Eigen, [] to access elements (or we can use Eigen-specific matrix operations)
     DistanceType
    >::type
    operator()(const R & a, const R & b) const;


private:

    DistanceType thres = 1000.0;
    double factor = 3000.0;

};


// test code to show how type detection works, TODO remove
template <typename RT>
class TestMetric {

public:

    using RecordType = RT;
    using ValueType = determine_ValueType<RecordType>;

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value
      && isBlazeDynamicVector<R>::value,
     DistanceType
    >::type
    operator()(const R & a, const R & b) const {
        return 1;
    }

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value
      && !isBlazeDynamicVector<R>::value,
     DistanceType
    >::type
    operator()(const R & a, const R & b) const {
        return 0;
    }

};



}  // namespace metric

#include "Standards.cpp"

#endif  // Header Guard
