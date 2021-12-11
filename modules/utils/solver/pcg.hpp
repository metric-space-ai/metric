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

/* 
	Implementations of cg and pcg.
	Look at two approaches: Matlab's, and
	hypre: https://github.com/LLNL/hypre/blob/master/src/krylov/pcg.c
	Started by Dan Spielman.
*/

#ifndef _METRIC_UTILS_SOLVER_PCG_HPP
#define _METRIC_UTILS_SOLVER_PCG_HPP
#include <functional>
#include <chrono>
#include "blaze/Math.h"
#include "helper/graphalgs.hpp"
#include "helper/solvertypes.hpp"

namespace metric {

const double EPS = 2.220446049250313e-16;

/*
		x = pcg(mat, b, pre; tol, maxits, maxtime, verbose, pcgIts, stag_test)

		solves a symmetric linear system using preconditioner `pre`.
		# Arguments
		* `pre` can be a function or a matrix.  If a matrix, a function to solve it is created with Cholesky Factorization.
		* `tol` is set to 1e-6 by default,
		* `maxits` defaults to MAX_VAL
		* `maxtime` defaults to MAX_VAL.  It measures seconds.
		* `verbose` defaults to false
		* `pcgIts` is an array for returning the number of pcgIterations.  Default is length 0, in which case nothing is returned.
		* `stag_test=k` stops the code if rho[it] > (1-1/k) rho[it-k].  Set to 0 to deactivate.
	*/

/**
 * @brief solves a symmetric linear system using preconditioner `pre`.
 * 
 * @param mat 
 * @param b 
 * @param pre  can be a function or a matrix.  If a matrix, a function to solve it is created with Cholesky Factorization.
 * @param pcgIts  is an array for returning the number of pcgIterations.  Default is length 0, in which case nothing is returned.
 * @param tol is set to 1e-6 by default,
 * @param maxits defaults to MAX_VAL
 * @param maxtime defaults to MAX_VAL.  It measures seconds.
 * @param verbose  defaults to false
 * @param stag_test `stag_test=k` stops the code if rho[it] > (1-1/k) rho[it-k].  Set to 0 to deactivate.
 * @return
 */
template<typename Tv>
blaze::DynamicVector<Tv> pcg(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& mat,
    const blaze::DynamicVector<Tv>& b, 
    SolverB<Tv> pre,
    std::vector<size_t>& pcgIts,
    float tol = 1e-6,
    double maxits = HUGE_VAL, 
    double maxtime = HUGE_VAL,
    bool verbose = false,
    size_t stag_test = 0);

/**
 * @brief solves a symmetric linear system using preconditioner `pre`.
 * 
 * @param mat 
 * @param b 
 * @param pre  can be a function or a matrix.  If a matrix, a function to solve it is created with Cholesky Factorization.
 * @param pcgIts  is an array for returning the number of pcgIterations.  Default is length 0, in which case nothing is returned.
 * @param tol is set to 1e-6 by default,
 * @param maxits defaults to MAX_VAL
 * @param maxtime defaults to MAX_VAL.  It measures seconds.
 * @param verbose  defaults to false
 * @param stag_test `stag_test=k` stops the code if rho[it] > (1-1/k) rho[it-k].  Set to 0 to deactivate.
 * @return
 */

template <typename Tv>
blaze::DynamicVector<Tv> pcg(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& mat,
    const blaze::DynamicVector<Tv>& b,
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& pre,
    std::vector<size_t>& pcgIts,
    float tol = 1e-6,
    double maxits = HUGE_VAL,
    double maxtime = HUGE_VAL,
    bool verbose = false,
    size_t stag_test = 0);

/**
 * @brief 
 * 
 * @param mat 
 * @param pre 
 * @param pcgIts 
 * @param tol 
 * @param maxits 
 * @param maxtime 
 * @param verbose 
 * @return
 */
template <typename Tv>
SubSolver<Tv> pcgSolver(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& mat,
    SolverB<Tv> pre,
    std::vector<size_t>& pcgIts,
    float tol = 1e-6,
    double maxits = HUGE_VAL,
    double maxtime = HUGE_VAL,
    bool verbose = false);

/**
 * @brief 
 * 
 * @param mat 
 * @param pre 
 * @param pcgIts 
 * @param tol 
 * @param maxits 
 * @param maxtime 
 * @param verbose 
 * @return
 */
template <typename Tv>
SubSolver<Tv> pcgSolver(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& mat,
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& pre,
    std::vector<size_t>& pcgIts,
    float tol = 1e-6,
    double maxits = HUGE_VAL,
    double maxtime = HUGE_VAL,
    bool verbose = false);

/**
 * @brief 
 * 
 * @param al 
 * @param p 
 * @param x 
 */
template <typename Tv>
void axpy2(const Tv al, const blaze::DynamicVector<Tv>& p, blaze::DynamicVector<Tv>& x);

/**
 * @brief 
 * 
 * @param beta 
 * @param p 
 * @param z 
 */
template <typename Tv>
void bzbeta(const Tv beta, blaze::DynamicVector<Tv>& p, const blaze::DynamicVector<Tv>& z);


}

#include "pcg/pcg.cpp"
#endif
