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
#include <blaze/math/DynamicMatrix.h>
#include <blaze/math/Rows.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include "../solver/solver.hpp"

//#include <cmath>
//#include "../../../3rdparty/blaze/Math.h"
//#include "../solver/solver.hpp"
//#include <blaze/math/DynamicMatrix.h>
//#include <blaze/math/Rows.h>
//#include <vector>
//#include <algorithm>
//#include <chrono>

namespace metric {

    /**
 * @param distanceMatrix Input distance matrix must have more nodes than processingChunkSize. 
 * @param partition Reference to the matrix that will contain the best result found
 * @param global_optimum_attempts  Is the number of attempts to find global optima, something like 100 - 1000
 * @param processing_chunk_size Must be > distanceMatrix nodes count
 * @param random_seed, leave unset for different results on different executions
 * @return 0 = no error, 1 = bad input, 2 = unknown error, 3 = distance matrix is corrupted
 */
int perform_graph_partition(
    blaze::DynamicMatrix<double> distance_matrix, 
    blaze::DynamicMatrix<int>& partition_matrix, 
    int global_optimum_attempts = 100, 
    int processing_chunk_size = 100, 
    __int64 random_seed = -1);
}

#include "partition.cpp"
#endif
