/*
		This Source Code Form is subject to the terms of the Mozilla Public
		License, v. 2.0. If a copy of the MPL was not distributed with this
		file, You can obtain one at http://mozilla.org/MPL/2.0/.

		Copyright (c) 2019 Panda Team
		Copyright (c) 2019 Oleg Popov
*/
/*
Laplacians is a package containing graph algorithms, with an emphasis on tasks related to 
spectral and algebraic graph theory. It contains (and will contain more) code for solving 
systems of linear equations in graph Laplacians, low stretch spanning trees, sparsifiation, 
clustering, local clustering, and optimization on graphs.

All graphs are represented by sparse adjacency matrices. This is both for speed, and because 
our main concerns are algebraic tasks. It does not handle dynamic graphs. It would be very slow 
to implement dynamic graphs this way.

https://github.com/danspielman/Laplacians.jl
*/

#ifndef _METRIC_UTILS_GRAPH_SPRSIFY_HPP
#define _METRIC_UTILS_GRAPH_SPRSIFY_HPP

#include <cmath>
#include <blaze/Math.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include "../solver/solver.hpp"

namespace metric {
/*
	Just implements Spielman-Srivastava
		as = sparsify(a; ep=0.5)

	Apply Spielman-Srivastava sparsification: sampling by effective resistances.
	`ep` should be less than 1.
*/

/**
 * @brief Apply Spielman-Srivastava sparsification: sampling by effective resistances.
 * 
 * @param a 
 * @param ep 
 * @param matrixConcConst 
 * @param JLfac 
 * @return
 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> sparsify_effective_resistance(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a, float ep = 0.3F, float matrixConcConst = 4.0F, float JLfac = 4.0F);

/**
 * Uses Kruskal's algorithm.
 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> sparsify_spanning_tree(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a, bool minimum = true);
}

#include "sparsify.cpp"
#endif
