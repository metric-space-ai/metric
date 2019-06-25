/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/

#include "Standards.hpp"

#include <cmath>

namespace metric
{
namespace distance
{

// template <typename Container>
// class Euclidian
// {
//     typedef typename Container::value_type T;
//     static_assert(
//         std::is_floating_point<T>::value, "T must be a float type");

//     T operator()(const Container &a, const Container &b) const
//     {
//         T sum = 0;
//         for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2)
//         {
//             sum += (*it1 - *it2) * (*it1 - *it2);
//         }
//         return std::sqrt(sum);
//     }
// };

template <typename V>
template<typename Container>
auto Euclidian<V>::operator()(const Container &a, const Container &b) const -> typename std::enable_if<!std::is_same<Container,V>::value,distance_type>::type
{
  static_assert(
      std::is_floating_point<value_type>::value, "T must be a float type");
  distance_type sum = 0;
  for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2)
  {
    sum += (*it1 - *it2) * (*it1 - *it2);
  }
  return std::sqrt(sum);
}

    template <typename V>
    auto Euclidian<V>::operator()(const V &a, const V &b) const -> distance_type
    {
        static_assert(
            std::is_floating_point<value_type>::value, "T must be a float type");
        distance_type sum = 0;
        sum += (a - b)*(a-b);
        return std::sqrt(sum);
    }

template<typename V>
template<typename Container>
auto Euclidian_thresholded<V>::operator()(const Container &a, const Container &b) const -> distance_type
{
    static_assert(
        std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2)
    {
        sum += (*it1 - *it2) * (*it1 - *it2);
    }
    return std::min(thres, value_type(factor * sqrt(sum)));
}

template<typename V>
template<typename Container>
auto Manhatten<V>::operator()(const Container &a, const Container &b) const -> distance_type
{
    static_assert(
        std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2)
    {
        sum += std::abs(*it1 - *it2);
    }
    return sum;
}

template<typename V>
template <typename Container>
auto P_norm<V>::operator()(const Container &a, const Container &b) const -> distance_type
{
    static_assert(
        std::is_floating_point<value_type>::value, "T must be a float type");
    distance_type sum = 0;
    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2)
    {
        sum += std::pow(std::abs(*it1 - *it2), p);
    }
    return std::pow(sum, 1 / p);
}

template<typename V>
template <typename Container>
auto Cosine<V>::operator()(const Container &A, const Container &B) const -> distance_type
{
  value_type dot = 0, denom_a = 0, denom_b = 0;
  for (auto it1 = A.begin(), it2 = B.begin(); it1 != A.end() || it2 != B.end(); ++it1, ++it2)
  {
    dot += *it1 * *it2;
    denom_a += *it1 * *it1;
    denom_b += *it2 * *it2;
  }
  return dot / (std::sqrt(denom_a) * std::sqrt(denom_b));
}

} // namespace distance
} // namespace metric
