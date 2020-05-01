/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/

#ifndef _METRIC_DISTANCE_K_RELATED_STANDARDS_HPP
#define _METRIC_DISTANCE_K_RELATED_STANDARDS_HPP

#include "../../utils.hpp"

namespace metric {


template <typename RT>
class Euclidean {

public:

    using RecordType = RT;
    using ValueType = contained_value_t<RecordType>;


    template <typename R>
    isIterCompatible<R>
    operator()(const R& a, const R& b) const;

    // Blaze vectors and sparse matrices
    template <typename R>
    isBlazeCompatible<R>
    operator()(const R& a, const R& b) const;

    // Eigen, [] to access elements (or we can use Eigen-specific matrix operations)
    template <typename R>
    isEigenCompatible<R>
    operator()(const R& a, const R& b) const;

};


template <typename RT>
class Euclidean_thresholded {

public:

    using RecordType = RT;
    using ValueType = contained_value_t<RecordType>;

    Euclidean_thresholded(DistanceType thres, double factor)
        : thres(thres)
        , factor(factor)
    {}

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value
      && container_type<R>::code != 2  // non-Blaze type
      && container_type<R>::code != 3, // non-Eigen type
     DistanceType
    >::type
    operator()(const R& a, const R& b) const;

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value && container_type<R>::code == 2, // Blaze vectors and sparse matrices
     DistanceType
    >::type
    operator()(const R& a, const R& b) const;

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value && container_type<R>::code == 3, // Eigen, [] to access elements (or we can use Eigen-specific matrix operations)
     DistanceType
    >::type
    operator()(const R& a, const R& b) const;

private:

    DistanceType thres = 1000.0;
    double factor = 3000.0;

};




/**
 * @class P_norm
 *
 * @brief Minkowski (L general) Metric
 *
 */
template <typename RT>
class P_norm {
public:
    using RecordType = RT;
    using ValueType = contained_value_t<RecordType>;
    //using typename BaseMetric<RT>::ValueType;
    static_assert(std::is_floating_point<ValueType>::value, "T must be a float type");
    /**
     * @brief Construct a new P_norm object
     *
     * @param p_
     */
    P_norm(const ValueType& p)
        : p(p)
    {
    }

    /**
     * @brief calculate Minkowski distance
     *
     * @param a first vector
     * @param b second vector
     * @return Minkowski distance between a and b
     */
    template <typename R>
    auto operator()(const R& a, const R& b) const -> DistanceType;

private:
    ValueType p = 1;
};



/**
 * @class Chebyshev
 *
 * @brief P. Chebyshev's metric
 *
 */
template <typename RT>
class Chebyshev {
public:
    using RecordType = RT;
    using ValueType = contained_value_t<RecordType>;
    //using typename BaseMetric<RT>::ValueType;
    static_assert(std::is_floating_point<ValueType>::value, "T must be a float type");
    /**
     * @brief calculate chebyshev metric
     *
     * @param lhs first container
     * @param rhs second container
     * @return Chebyshev distance between lhs and rhs
     */
    template <typename R>
    auto operator()(const R& a, const R& b) const -> DistanceType;
};



// test code to show how type detection works, TODO remove

/*

template <typename RT>
class TestMetric {

public:

    using RecordType = RT;
    using ValueType = contained_value<RecordType>;

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

// */

template <typename RT>
class TestMetric {

public:

    using RecordType = RT;
    using ValueType = contained_value<RecordType>;

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


template <typename RT>
class TestMetric2 {

public:

    using RecordType = RT;
    using ValueType = contained_value<RecordType>;

    template <typename R>
    isContainerOfTypeT<blaze::DynamicVector<void>, R> // void inner type does not matter
    operator()(const R & a, const R & b) const {
        return 1;
    }

    template <typename R>
    isNotContainerOfTypeT<blaze::DynamicVector<void>, R> // void inner type does not matter
    operator()(const R & a, const R & b) const {
        return 0;
    }

};




}  // namespace metric

#include "Standards.cpp"

#endif  // Header Guard
