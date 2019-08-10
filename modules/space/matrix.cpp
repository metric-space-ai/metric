/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019  Panda Team
*/
#include "matrix.hpp"

namespace metric {

// /*** constructor: empty Matrix **/
// template <typename recType, typename Metric, typename distType>
// Matrix<recType, Metric, distType>::Matrix(Metric d) : metric_(d), data_() {}

// /*** constructor: with a single data record **/
// template <typename recType, typename Metric, typename distType>
// Matrix<recType, Metric, distType>::Matrix(const recType &p, Metric d)
//     : metric_(d), data_(p) {}

/*** constructor: with a vector data records **/
template <typename recType, typename Metric, typename distType>
Matrix<recType, Metric, distType>::Matrix(const std::vector<recType> &p,
                                          Metric d)
    : metric_(d), data_(p),D_(p.size()) {
  for (size_t i = 0; i < D_.columns(); ++i) {
    D_(i, i) = 0;
    for (size_t j = i + 1; j < D_.rows(); ++j) {
      auto distance = metric_(p[i], p[j]);
      D_(i, j) = distance;
    }
  }
}

template <typename recType, typename Metric, typename distType>
distType 
Matrix<recType, Metric, distType>::operator()(size_t i, size_t j) const {
  return D_(i,j);
}

template <typename recType, typename Metric, typename distType>
recType 
Matrix<recType, Metric, distType>::operator[](size_t id) const{
  return (data_(id));
}

template <typename recType, typename Metric, typename distType>
size_t 
Matrix<recType, Metric, distType>::size() const {
  return data_.size();
}

} // namespace metric
