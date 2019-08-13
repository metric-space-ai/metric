/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch
*/

#ifndef _METRIC_MAPPING_DBSCAN_HPP
#define _METRIC_MAPPING_DBSCAN_HPP

/*
A DBSCAN implementation based on distance matrix.
*/

//   References:
//
//       Martin Ester, Hans-peter Kriegel, Jörg S, and Xiaowei Xu
//       A density-based algorithm for discovering clusters
//       in large spatial databases with noise. 1996.

#include <vector>
#include <string>

namespace metric {

/**
 * @brief 
 * 

 * @param data 
 * @param eps 
 * @param minpts 
 * @param distance_measure 
 * @return 
 */
template <typename T>
std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> dbscan(
    const std::vector<std::vector<T>>& data, T eps, int minpts, std::string distance_measure = "euclidian");

}  // namespace metric

#include "dbscan.cpp"
#endif
