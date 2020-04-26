/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/

#include "Standards.hpp"

#include <cmath>
#include <algorithm>

#include "../../../3rdparty/blaze/Blaze.h"

namespace metric {

template <typename V>
template <typename Container>
auto Euclidean<V>::operator()(const Container& a, const Container& b) const ->
    typename std::enable_if<!std::is_same<Container, V>::value, distance_type>::type
{
    //static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
        sum += (*it1 - *it2) * (*it1 - *it2);
    }
    return std::sqrt(sum);
}

template <typename V>
auto Euclidean<V>::operator()(const V& a, const V& b) const -> distance_type
{
    static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    sum += (a - b) * (a - b);
    return std::sqrt(sum);
}

template <typename V>
template <template <typename, bool> class Container, typename ValueType, bool F> // detect Blaze object by signature
double Euclidean<V>::operator()(
        const Container<ValueType, F> & a, const Container<ValueType, F> & b) const {
    return blaze::norm(a - b);
}

template <typename V>
template <typename Container>
auto Euclidean_thresholded<V>::operator()(const Container& a, const Container& b) const -> distance_type
{
    static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2) {
        sum += (*it1 - *it2) * (*it1 - *it2);
    }
    return std::min(thres, value_type(factor * std::sqrt(sum)));
}

template <typename V>
template <typename Container>
auto Euclidean_hard_clipped<V>::operator()(const Container& a, const Container& b) const -> distance_type
{
    static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2) {
        sum += (*it1 - *it2) * (*it1 - *it2);
    }
    return std::min(max_distance_, value_type(scal_ * std::sqrt(sum)));
}

template <typename V>
template <typename Container>
auto Euclidean_soft_clipped<V>::operator()(const Container& a, const Container& b) const -> distance_type
{
    static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2) {
        sum += (*it1 - *it2) * (*it1 - *it2);
    }
    auto distance = std::sqrt(sum);
    if (distance > x_) {
        return F_ * (value_type(1) - std::exp((-distance + x_) / T_)) + y_;
    } else {
        return scal_ * distance;
    }
}

template <typename V>
template <typename Container>
Euclidean_standardized<V>::Euclidean_standardized(const Container& A)
    : mean(A[0].size(), 0)
    , sigma(A[0].size(), 0)
{

    for (auto it = A.begin(); it != A.end(); ++it) {
        for (size_t i = 0; i < A[0].size(); ++i) {
            mean[i] += *it[i];
        }
    }
    for (size_t i = 0; i < mean.size(); ++i) {
        mean[i] /= value_type(mean.size());
    }

    for (auto it = A.begin(); it != A.end(); ++it) {
        for (size_t i = 0; i < A[0].size(); ++i) {
            sigma[i] += std::pow(*it[i] - mean[i], 2);
        }
    }
    for (size_t i = 0; i < sigma.size(); ++i) {
        sigma[i] = std::sqrt(sigma[i] / value_type(sigma.size()));
    }
}

template <typename V>
template <typename Container>
auto Euclidean_standardized<V>::operator()(const Container& a, const Container& b) const -> distance_type
{
    //static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        sum += std::pow((a[i] - mean[i]) / sigma[i] - (b[i] - mean[i]) / sigma[i], 2);
    }
    return std::sqrt(sum);
}

template <typename V>
template <typename Container>
Manhatten_standardized<V>::Manhatten_standardized(const Container& A)
    : mean(A[0].size(), 0)
    , sigma(A[0].size(), 0)
{

    for (auto it = A.begin(); it != A.end(); ++it) {
        for (size_t i = 0; i < A[0].size(); ++i) {
            mean[i] += *it[i];
        }
    }
    for (size_t i = 0; i < mean.size(); ++i) {
        mean[i] /= value_type(mean.size());
    }

    for (auto it = A.begin(); it != A.end(); ++it) {
        for (size_t i = 0; i < A[0].size(); ++i) {
            sigma[i] += std::pow(*it[i] - mean[i], 2);
        }
    }
    for (size_t i = 0; i < sigma.size(); ++i) {
        sigma[i] = std::sqrt(sigma[i] / value_type(sigma.size()));
    }
}

template <typename V>
template <typename Container>
auto Manhatten_standardized<V>::operator()(const Container& a, const Container& b) const -> distance_type
{
    //static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        sum += std::abs((a[i] - mean[i]) / sigma[i] - (b[i] - mean[i]) / sigma[i]);
    }
    return sum;
}

template <typename V>
template <typename Container>
auto Manhatten<V>::operator()(const Container& a, const Container& b) const -> distance_type
{
    static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2) {
        sum += std::abs(*it1 - *it2);
    }
    return sum;
}

template <typename V>
template <typename Container>
auto P_norm<V>::operator()(const Container& a, const Container& b) const -> distance_type
{
    static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2) {
        sum += std::pow(std::abs(*it1 - *it2), p);
    }
    return std::pow(sum, 1 / p);
}

template <typename V>
template <typename Container>
auto Cosine<V>::operator()(const Container& A, const Container& B) const -> distance_type
{
    value_type dot = 0, denom_a = 0, denom_b = 0;
    for (auto it1 = A.begin(), it2 = B.begin(); it1 != A.end() || it2 != B.end(); ++it1, ++it2) {
        dot += *it1 * *it2;
        denom_a += *it1 * *it1;
        denom_b += *it2 * *it2;
    }
    return std::acos(dot / (std::sqrt(denom_a) * std::sqrt(denom_b))) / M_PI;
}

template <typename V>
template <typename Container>
auto Weierstrass<V>::operator()(const Container& A, const Container& B) const -> distance_type
{
    value_type dot_ab = 0, dot_a = 0, dot_b = 0;
    for (auto it1 = A.begin(), it2 = B.begin(); it1 != A.end() || it2 != B.end(); ++it1, ++it2) {
        dot_ab += *it1 * *it2;
        dot_a += *it1 * *it1;
        dot_b += *it2 * *it2;
    }
    return std::acosh(std::sqrt(1 + dot_a) * std::sqrt(1 + dot_b) - dot_ab);
}

template <typename V>
template <typename Container>
auto CosineInverted<V>::operator()(const Container& A, const Container& B) const -> distance_type
{
    value_type dot = 0, denom_a = 0, denom_b = 0;
    for (auto it1 = A.begin(), it2 = B.begin(); it1 != A.end() || it2 != B.end(); ++it1, ++it2) {
        dot += *it1 * *it2;
        denom_a += *it1 * *it1;
        denom_b += *it2 * *it2;
    }
    return std::abs(1 - dot / (std::sqrt(denom_a) * std::sqrt(denom_b)));
}

template <typename V>
template <typename Container>
auto Chebyshev<V>::operator()(const Container& lhs, const Container& rhs) const -> distance_type
{
    distance_type res = 0;
    for (std::size_t i = 0; i < lhs.size(); i++) {
        auto m = std::abs(lhs[i] - rhs[i]);
        if (m > res)
            res = m;
    }
    return res;
}

}  // namespace metric
