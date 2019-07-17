/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_DETAILS_K_RELATED_STANDARDS_HPP
#define _METRIC_DISTANCE_DETAILS_K_RELATED_STANDARDS_HPP

#define DECLARE_METRIC_TYPES                            \
    using value_type = typename Container::value_type;  \
    using distance_type = value_type;                   \
    using record_type = Container;


namespace metric
{

    namespace distance
    {
/*** Euclidian (L2) Metric ***/
        template <typename V>
        struct Euclidian
        {
            using value_type = V;
            using distance_type = value_type;

            explicit Euclidian() = default;

            template<typename Container>
            typename std::enable_if<!std::is_same<Container,V>::value,distance_type>::type
            operator()(const Container &a, const Container &b) const;

            distance_type operator()(const V &a, const V &b) const;

        };

/***  Manhatten/Cityblock (L1) Metric ***/
        template <typename V>
        struct Manhatten
        {
            using value_type = V;
            using distance_type = value_type;

            explicit Manhatten() = default;

            template<typename Container>
            distance_type  operator()(const Container &a, const Container &b) const;
        };

/*** Minkowski (L general) Metric ***/
        template <typename V>
        struct P_norm
        {
            using value_type = V;
            using distance_type = value_type;

            value_type p = 1;

            P_norm() = default;
            explicit P_norm(const value_type & p_): p(p_) {}

            template<typename Container>
            distance_type operator()(const Container &a, const Container &b) const;
        };

/*** Minkowski Metric (L... / P_Norm) ***/
        template <typename V>
        struct Euclidian_thresholded
        {
            using value_type = V;
            using distance_type = value_type;

            value_type thres = 1000.0;
            value_type factor = 3000.0;

            explicit Euclidian_thresholded() = default;
            Euclidian_thresholded(value_type thres_, value_type factor_) : thres(thres_), factor(factor_) {}

            template<typename Container>
            distance_type  operator()(const Container &a, const Container &b) const;
        };

/*** Cosine Metric ***/
        template <typename V>
        struct Cosine
        {
            using value_type = V;
            using distance_type = value_type;

            template<typename Container>
            distance_type  operator()(const Container &a, const Container &b) const;
        };
/*** Chebyshev Metric ***/
    template <typename V> struct Chebyshev {
      using value_type = V;
      using distance_type = value_type;

      explicit Chebyshev() = default;

      template <typename Container>
      distance_type operator()(const Container &lhs, const Container &rhs) const;
    };


    } // namespace distance


} // namespace metric

#include "Standards.cpp"

#endif // Header Guard
