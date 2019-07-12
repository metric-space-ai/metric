/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019  Panda Team
*/
#include "matrix.hpp"
//#include "../../../3rdparty/blaze/Math.h"

/*** standard euclidian (L2) Metric ***/
// template <typename Container>
// struct L2_Metric_STL;

// template <typename Container>
// struct L2_Metric_STL
// {
//     typedef typename Container::value_type result_type;
//     static_assert(std::is_floating_point<result_type>::value,
//                   "T must be a float type");

//     result_type operator()(const Container &a, const Container &b) const
//     {
//         result_type sum = 0;
//         for (auto it1 = a.begin(), it2 = b.begin();
//              it1 != a.end() || it2 != b.end(); ++it1, ++it2)
//         {
//             sum += (*it1 - *it2) * (*it1 - *it2);
//         }
//         return std::sqrt(sum);
//     }
// };

namespace metric {
namespace space {
/*** constructor: empty Matrix **/
template <typename recType, typename Metric, typename distType>
Matrix<recType, Metric, distType>::Matrix(Metric d) : metric_(d), data_() {}

/*** constructor: with a single data record **/
template <typename recType, typename Metric, typename distType>
Matrix<recType, Metric, distType>::Matrix(const recType &p, Metric d)
    : metric_(d), data_(p) {}

/*** constructor: with a vector data records **/
template <typename recType, typename Metric, typename distType>
Matrix<recType, Metric, distType>::Matrix(const std::vector<recType> &p,
                                          Metric d)
    : metric_(d), data_(p) {
  for (size_t i = 0; i < D_.rows(); ++i) {
    D_(i, i) = 0;
    for (size_t j = i + 1; j < D_.rows(); ++j) {
      D_(i, j) = metric_(p[i], p[j]);
    }
  }
}

template <typename recType, typename Metric, typename distType>
distType Matrix<recType, Metric, distType>::operator()(size_t i, size_t j) {
  return (D_(i, j));
}

template <typename recType, typename Metric, typename distType>
recType Matrix<recType, Metric, distType>::operator[](size_t id) {
  return (data_(id));
}
} // namespace space
} // namespace metric
