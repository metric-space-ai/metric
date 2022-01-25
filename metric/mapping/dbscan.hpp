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

#include "../space/matrix.hpp"
#include <string>
#include <vector>
namespace metric {

/**
 * @brief Density-based spatial clustering of applications with noise (DBSCAN)
 *
 * @param dm distance matrix
 * @param eps the maximum distance between neighbor objects
 * @param minpts minimum number of neighboring objects needed to form a cluster
 * @return three vectors in a tuple, first vector of size N ( size of initial dataset) contains number of the cluster
 *          of corresponding source point, second contains indices of center points of clusters, third vector contains
 *          size of corresponding cluster.
 */
template <typename RecType, typename Metric, typename T>
std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> dbscan(const metric::Matrix<RecType, Metric> &dm,
																		T eps, std::size_t minpts);

} // namespace metric

#include "dbscan.cpp"
#endif
