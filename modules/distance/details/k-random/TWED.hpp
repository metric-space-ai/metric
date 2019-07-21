/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_DETAILS_K_RANDOM_TWED_HPP
#define _METRIC_DISTANCE_DETAILS_K_RANDOM_TWED_HPP

namespace metric
{

namespace distance
{
/*** Time Warp Elastic Distance (for curves) ***/
template <typename V>
struct TWED
{
  using value_type = V;
  using distance_type = value_type;


#ifdef _BLASE_BLASE_H_
  using value_type = V;
  using distance_type = value_type;
  //  using record_type = blaze::CompressedVector<value_type>;

  value_type penalty = 0;
  value_type elastic = 1;
  bool is_zero_padded = false;
  explicit TWED() = default;
  TWED(const value_type & penalty_, const value_type & elastic_, bool is_zero_paded_):
      penalty(penalty_), elastic(elastic_), is_zero_padded(is_zero_padded_) {}
#else

  value_type penalty = 0;
  value_type elastic = 1;
  explicit TWED() = default;
  TWED(const value_type & penalty_, const value_type & elastic_):penalty(penalty_), elastic(elastic_) {}
#endif

  template<typename Container>
  value_type  operator()(const Container &As, const Container &Bs) const;
// #ifdef _BLAZE_BLAZE_H_
//     typename Container::value_type
//     operator()(blaze::CompressedVector<Container::value_type> const &As, blaze::CompressedVector<Container::value_type> const &Bs, Container::value_type const &penalty = 0, Container::value_type const &elastic = 1, bool is_zero_padded = false) const;
// #endif
};
} // namespace distance

} // namespace metric

#include "TWED.cpp"

#endif // Header Guard
