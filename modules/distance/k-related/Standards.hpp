/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_K_RELATED_STANDARDS_HPP
#define _METRIC_DISTANCE_K_RELATED_STANDARDS_HPP

#include <type_traits>

namespace metric {

/**
 * @class Euclidian
 * 
 * @brief Euclidian (L2) Metric
 */
template <typename V = double>
struct Euclidian {
    using value_type = V;
    using distance_type = value_type;

    explicit Euclidian() = default;

    /**
     * @brief Calculate Euclidian distance in R^n
     *
     * @param a first vector
     * @param b second vector
     * @return euclidian distance between a and b
     */
    template <typename Container>
    typename std::enable_if<!std::is_same<Container, V>::value, distance_type>::type operator()(
        const Container& a, const Container& b) const;

    /**
     * @brief Calculate Euclidian distance in R
     *
     * @param a first value 
     * @param b second value
     * @return euclidian distance between a and b
     */

    distance_type operator()(const V& a, const V& b) const;
};

/**
 * @class Manhatten
 * 
 * @brief Manhatten/Cityblock (L1) Metric
 *
 */
template <typename V = double>
struct Manhatten {
    using value_type = V;
    using distance_type = value_type;

    explicit Manhatten() = default;

    /**
     * @brief Calculate Manhatten distance in R^n
     *
     * @param a first vector
     * @param b second vector
     * @return Manhatten distance between a and b
     */

    template <typename Container>
    distance_type operator()(const Container& a, const Container& b) const;
};

/**
 * @class P_norm
 * 
 * @brief Minkowski (L general) Metric
 *
 */
template <typename V = double>
struct P_norm {
    using value_type = V;
    using distance_type = value_type;

    //P_norm() = default;
    /**
     * @brief Construct a new P_norm object
     *
     * @param p_
     */
    explicit P_norm(const value_type& p_ = 1)
        : p(p_)
    {
    }

    /**
     * @brief calculate Minkowski distance 
     *
     * @param a first vector
     * @param b second vector
     * @return Minkowski distance between a and b
     */
    template <typename Container>
    distance_type operator()(const Container& a, const Container& b) const;

    value_type p = 1;
};

/**
 * @class Euclidian_threshold
 *
 * @brief Minkowski Metric (L... / P_Norm)
 *
 */
template <typename V = double>
struct Euclidian_thresholded {
    using value_type = V;
    using distance_type = value_type;

    explicit Euclidian_thresholded() = default;

    /**
     * @brief Construct a new Euclidian_thresholded object
     *
     * @param thres_
     * @param factor_
     */
    Euclidian_thresholded(value_type thres_, value_type factor_)
        : thres(thres_)
        , factor(factor_)
    {
    }

    /**
     * @brief
     *
     * @param a
     * @param b
     * @return
     */
    template <typename Container>
    distance_type operator()(const Container& a, const Container& b) const;

    value_type thres = 1000.0;
    value_type factor = 3000.0;
};

/**
 * @class Cosine
 *
 * @brief Cosine similarity
 *
 */

template <typename V = double>
struct Cosine {
    using value_type = V;
    using distance_type = value_type;

    /**
     * @brief calculate cosine similariy between two non-zero vector
     *
     * @param a first vector
     * @param b second vector
     * @return cosine similarity between a and b
     */
    template <typename Container>
    distance_type operator()(const Container& a, const Container& b) const;
};

/**
 * @class CosineInverted
 *
 * @brief CosineInverted similarity. Means 1 - Cosine.
 *
 */

template <typename V = double>
struct CosineInverted {
    using value_type = V;
    using distance_type = value_type;

    /**
     * @brief calculate cosine similariy between two non-zero vector
     *
     * @param a first vector
     * @param b second vector
     * @return cosine similarity between a and b
     */
    template <typename Container>
    distance_type operator()(const Container& a, const Container& b) const;
};

/**
 * @class Chebyshev
 *
 * @brief Chebyshev metric
 *
 */
template <typename V = double>
struct Chebyshev {
    using value_type = V;
    using distance_type = value_type;

    explicit Chebyshev() = default;

    /**
     * @brief calculate chebyshev metric
     *
     * @param lhs first container
     * @param rhs second container
     * @return Chebtshev distance between lhs and rhs
     */
    template <typename Container>
    distance_type operator()(const Container& lhs, const Container& rhs) const;
};

}  // namespace metric

#include "Standards.cpp"

#endif  // Header Guard
