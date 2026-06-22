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
#include "helper/graphalgs.hpp"
#include "helper/solvertypes.hpp"
#include <chrono>
#include <cmath>
#include <functional>
#include <metric/numeric/Math.h>
#include <stdexcept>

namespace mtrc {

const double EPS = 2.220446049250313e-16;

/*
		x = pcg(mat, b, pre; tol, maxits, maxtime, verbose, pcgIts, stag_test)

		solves a symmetric linear system using preconditioner `pre`.
		# Arguments
		* `pre` can be a function or a matrix.  If a matrix, a function to solve it is created with Cholesky
   Factorization.
		* `tol` is set to 1e-6 by default,
		* `maxits` defaults to MAX_VAL
		* `maxtime` defaults to MAX_VAL.  It measures seconds.
		* `verbose` defaults to false
		* `pcgIts` is an array for returning the number of pcgIterations.  Default is length 0, in which case nothing is
   returned.
		* `stag_test` is a running-minimum stagnation guard (see the @param documentation below).
   Set to 0 to deactivate.
	*/

/**
 * @brief Solves the symmetric positive-(semi)definite linear system `mat x = b` with preconditioner
 * `pre`.
 *
 * Objective: PCG minimizes the quadratic energy E(x) = 1/2 x^T mat x - b^T x, i.e. it drives the
 * residual `mat x - b` to zero in the mat-inner-product. When `mat` is a graph Laplacian this is the
 * graph Dirichlet energy (the electrical-flow objective); for a general SPD `mat` it is that
 * matrix's quadratic form. This is the metric-space objective the solver reports solving.
 *
 * Preconditions (enforced; violations throw std::invalid_argument):
 *  - `mat` is square and `b.size() == mat.rows()`.
 *  - `mat` is symmetric positive (semi-)definite and `pre` is a symmetric positive-definite
 *    approximation of its inverse; the preconditioned residual dot(r, pre(r)) must be positive.
 *  - `pre(v)` returns a vector of the same dimension as its input.
 *  - `b` is finite (no NaN/Inf entries).
 *
 * Numerical behavior: a literally zero `b` returns the zero vector. The iteration terminates on
 * relative residual `< tol`, on loss of curvature/positive-definiteness, on stagnation, on
 * `maxtime`, or after `maxits` iterations; when `maxits` is left at its HUGE_VAL default an
 * effective finite cap (max(10*n, 1000)) is applied so the loop cannot hang. The returned vector
 * is the best iterate seen (lowest relative residual), so it is never worse than the zero guess.
 *
 * @param mat
 * @param b
 * @param pre  can be a function or a matrix.  If a matrix, a function to solve it is created with Cholesky
 * Factorization.
 * @param pcgIts  is an array for returning the number of pcgIterations.  Default is length 0, in which case nothing is
 * returned.
 * @param tol is set to 1e-6 by default,
 * @param maxits defaults to MAX_VAL
 * @param maxtime defaults to MAX_VAL.  It measures seconds.
 * @param verbose  defaults to false
 * @param stag_test stagnation guard: with factor f = 1 - 1/stag_test, the iteration stops after
 * `stag_test` consecutive steps in which the preconditioned residual rho fails to improve below
 * f * (best rho so far). Set to 0 to deactivate. (This is a running-minimum guard, not a fixed
 * k-step lookback.)
 * @return
 */
template <typename Tv>
mtrc::numeric::DynamicVector<Tv> pcg(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &mat,
									   const mtrc::numeric::DynamicVector<Tv> &b, SolverB<Tv> pre,
									   std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = HUGE_VAL,
									   double maxtime = HUGE_VAL, bool verbose = false, size_t stag_test = 0);

/**
 * @brief solves a symmetric linear system using preconditioner `pre`.
 *
 * @param mat
 * @param b
 * @param pre  can be a function or a matrix.  If a matrix, a function to solve it is created with Cholesky
 * Factorization.
 * @param pcgIts  is an array for returning the number of pcgIterations.  Default is length 0, in which case nothing is
 * returned.
 * @param tol is set to 1e-6 by default,
 * @param maxits defaults to MAX_VAL
 * @param maxtime defaults to MAX_VAL.  It measures seconds.
 * @param verbose  defaults to false
 * @param stag_test stagnation guard: with factor f = 1 - 1/stag_test, the iteration stops after
 * `stag_test` consecutive steps in which the preconditioned residual rho fails to improve below
 * f * (best rho so far). Set to 0 to deactivate. (This is a running-minimum guard, not a fixed
 * k-step lookback.)
 * @return
 */

template <typename Tv>
mtrc::numeric::DynamicVector<Tv> pcg(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &mat,
									   const mtrc::numeric::DynamicVector<Tv> &b,
									   const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &pre,
									   std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = HUGE_VAL,
									   double maxtime = HUGE_VAL, bool verbose = false, size_t stag_test = 0);

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
SubSolver<Tv> pcgSolver(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &mat, SolverB<Tv> pre,
						std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = HUGE_VAL,
						double maxtime = HUGE_VAL, bool verbose = false);

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
SubSolver<Tv> pcgSolver(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &mat,
						const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &pre,
						std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = HUGE_VAL,
						double maxtime = HUGE_VAL, bool verbose = false);

/**
 * @brief
 *
 * @param al
 * @param p
 * @param x
 */
template <typename Tv>
void axpy2(const Tv al, const mtrc::numeric::DynamicVector<Tv> &p, mtrc::numeric::DynamicVector<Tv> &x);

/**
 * @brief
 *
 * @param beta
 * @param p
 * @param z
 */
template <typename Tv>
void bzbeta(const Tv beta, mtrc::numeric::DynamicVector<Tv> &p, const mtrc::numeric::DynamicVector<Tv> &z);

} // namespace mtrc

#include "pcg/pcg.cpp"
#endif
