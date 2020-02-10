/*
		This Source Code Form is subject to the terms of the Mozilla Public
		License, v. 2.0. If a copy of the MPL was not distributed with this
		file, You can obtain one at http://mozilla.org/MPL/2.0/.

		Copyright (c) 2019 Panda Team
		Copyright (c) 2019 Oleg Popov
*/

#ifndef _METRIC_UTILS_GRAPH_PARTITION_HPP
#define _METRIC_UTILS_GRAPH_PARTITION_HPP

#include <cmath>
#include "../../../3rdparty/blaze/Math.h"
#include "../solver/solver.hpp"
#include <blaze/math/DynamicMatrix.h>
#include <blaze/math/Rows.h>
#include <vector>
#include <algorithm>
#include <chrono>

namespace metric {

/**
 * @brief Apply Spielman-Srivastava sparsification: sampling by effective resistances.
 *
 * @param distanceMatrix Input distance matrix, must have more than 100 nodes. 
 * @param partition Reference to the matrix that will contain the best result found
 * @param attempts  Is the number of attempts to find global optima, SOMETHING LIKE 100 - 1000
 * @return true if there is no error, false otherwise
 */
bool perform_graph_partition(blaze::DynamicMatrix<double> distanceMatrix, blaze::DynamicMatrix<int> &partition, int globalOptimum = 100);

}

#include "partition.cpp"
#endif
