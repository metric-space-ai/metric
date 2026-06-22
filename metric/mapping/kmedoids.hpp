/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch */

#ifndef _METRIC_MAPPING_KMEDOIDS_HPP
#define _METRIC_MAPPING_KMEDOIDS_HPP

#include <metric/space/matrix.hpp>

#include <tuple>
#include <vector>

namespace mtrc {
/**
 * @brief
 *
 * @param DM
 * @param k
 * @return
 */
template <typename RecType, typename Metric>
auto kmedoids(const mtrc::Matrix<RecType, Metric> &DM, int k)
	-> std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>;

} // namespace mtrc

#include "kmedoids.cpp"
#endif
