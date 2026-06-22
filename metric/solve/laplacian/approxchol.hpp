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

// Structures for the approxChol solver

#ifndef _METRIC_UTILS_SOLVER_APPROXCOL_HPP
#define _METRIC_UTILS_SOLVER_APPROXCOL_HPP
#include "helper/graphalgs.hpp"
#include "helper/lapwrappers.hpp"
#include "helper/solvertypes.hpp"
#include "pcg.hpp"
#include <chrono>
#include <functional>
#include <iostream>
#include <metric/numeric/Math.h>
#include <stdint.h>
#include <type_traits>
#include <utility>
#include <vector>

namespace mtrc {

/**
 * @brief Laplacian solver by Daniel A.Spielman, 2017
 *  This algorithm is an implementation of an approximate edge - by - edge elimination
 *  algorithm inspired by the Approximate Gaussian Elimination algorithm of Kyng and Sachdeva.
 *  approxchol_lap: the main solver.
 *
 *  Preconditions on the adjacency matrix `a` (violations throw std::invalid_argument):
 *   - square and symmetric (a == a^T);
 *   - all stored edge weights strictly positive (a zero/negative stored entry is not an edge);
 *   - zero diagonal (no self-loops); the Laplacian L = diag(a*1) - a is formed internally.
 *  Disconnected graphs are supported: each connected component is solved independently and the
 *  per-component nullspace (constant shift) is fixed by returning a mean-zero solution per
 *  component. The factorization is randomized but the returned solution is deterministic up to
 *  the PCG tolerance because PCG drives it to the unique mean-zero answer.
 *
 * @param a
 * @param pcgIts is an array for returning the number of pcgIterations.  Default is length 0, in which case nothing is
 returned.
 * @param tol is set to 1e-6 by default,
 * @param maxits defaults to MAX_VAL
 * @param maxtime defaults to MAX_VAL.  It measures seconds.
 * @param verbose defaults to false
 * @param params is additional parameters.
 * 		params = ApproxCholParams(order, output)
			order can be one of
			Deg(by degree, adaptive),
			WDeg(by original wted degree, nonadaptive),
			Given

 * @return
 */
template <typename Tv>
inline SubSolver<Tv> approxchol_lap(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
									std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = 1000,
									double maxtime = HUGE_VAL, bool verbose = false,
									ApproxCholParams params = ApproxCholParams());

/*
	LLp elements are all in the same column.
	row tells us the row, and val is the entry.
	val is set to zero for some edges that we should remove.
	next gives the next in the column.It points to itself to terminate.
	reverse is the index into lles of the other copy of this edge,
	since every edge is stored twice as we do not know the order of elimination in advance.
	*/

/**
 * @brief LLp elements are all in the same column.
 *
 * @tparam Tv
 */
template <typename Tv> class LLp {
  public:
	/**
	 * @brief Construct a new LLp object
	 *
	 * @param Arow
	 * @param Aval
	 * @param Anext
	 * @param Areverse
	 */
	LLp(const size_t Arow, const Tv Aval, const LLp *Anext, const LLp *Areverse)
		: row(Arow), val(Aval), next(Anext), reverse(Areverse)
	{
	}
	/**
	 * @brief Construct a new LLp object
	 *
	 * @param Arow
	 * @param Aval
	 */
	LLp(const size_t Arow, const Tv Aval) : row(Arow), val(Aval), next(this), reverse(this) {}

	/**
	 * @brief Construct a new LLp object
	 *
	 * @param Arow
	 * @param Aval
	 * @param Anext
	 */
	LLp(const size_t Arow, const Tv Aval, LLp *Anext) : row(Arow), val(Aval), next(Anext), reverse(this) {}

	/**
	 * @brief row tells us the row
	 *
	 */
	size_t row;

	/**
	 * @brief  val is the entry. val is set to zero for some edges that we should remove.
	 *
	 */
	Tv val;

	/**
	 * @brief gives the next in the column.It points to itself to terminate.
	 *
	 */
	LLp *next;

	/**
	 * @brief  the index into lles of the other copy of this edge,
	 * since every edge is stored twice as we do not know the order of elimination in advance.
	 *
	 */
	LLp *reverse;
};

/*

	LLmatp is the data structure used to maintain the matrix during elimination.
	It stores the elements in each column in a singly linked list(only next ptrs)
	Each element is an LLp(linked list pointer).
	The head of each column is pointed to by cols.

	We probably can get rid of degs - as it is only used to store initial degrees.

	*/

/**
 * @class LLmatp
 *
 * @brief the data structure used to maintain the matrix during elimination.
 *        It stores the elements in each column in a singly linked list(only next ptrs)
 *        Each element is an LLp(linked list pointer).
 *        The head of each column is pointed to by cols.
 *        We probably can get rid of degs - as it is only used to store initial degrees.
 *
 */
template <typename Tv> class LLmatp {
  public:
	/**
	 * @brief Construct a new LLmatp object
	 *
	 * @param A
	 */
	explicit LLmatp(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &A);

	/**
	 * @brief Destroy the LLmatp object
	 *
	 */
	~LLmatp()
	{
		for (size_t i = 0; i < llelems.size(); i++)
			delete llelems[i];
	}

	size_t n;
	std::vector<size_t> degs;
	std::vector<LLp<Tv> *> cols;
	std::vector<LLp<Tv> *> llelems;
};

/**
 * @brief Print a column in an LLmatp matrix.
 *        This is here for diagnostics.
 *
 * @param llmat
 * @param i
 */
template <typename Tv> void print_ll_col(const LLmatp<Tv> &llmat, const size_t i);

// NOTE: A fixed-ordering variant (LLord / LLMatOrd / LLcol and a compressCol(LLcol) overload)
// from the Julia port lived here but was never instantiated by the adaptive-degree solver that
// approxchol_lap actually uses. It was removed (the dead overload also contained a no-op
// sort(begin, begin + 1)); the active path uses LLmatp + compressCol(LLp*, ApproxCholPQ&).

// LDLinv

/*  LDLinv contains the information needed to solve the Laplacian systems.
	  It does it by applying Linv, then Dinv, then Linv (transpose).
	  But, it is specially constructed for this particular solver.
	  It does not explicitly make the matrix triangular.
	  Rather, col[i] is the name of the ith col to be eliminated
	*/

/**
 * @class LDLinv
 * @brief LDLinv contains the information needed to solve the Laplacian systems.
 *        It does it by applying Linv, then Dinv, then Linv (transpose).
 *        But, it is specially constructed for this particular solver.
 *        It does not explicitly make the matrix triangular.
 *   	  Rather, col[i] is the name of the ith col to be eliminated
 */
template <typename Tv> class LDLinv {
  public:
	/**
	 * @brief Construct a new LDLinv object
	 *
	 * @param A
	 */
	explicit LDLinv(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &A)
		: col(A.columns() - 1, 0), colptr(A.columns(), 0), d(A.columns(), 0) /* rowval and fval are empty*/
	{
	}

	/**
	 * @brief Construct a new LDLinv object
	 *
	 * @param A
	 */
	explicit LDLinv(const LLmatp<Tv> &A) : col(A.n - 1, 0), colptr(A.n, 0), d(A.n, 0) {}

	/**
	 * @brief
	 *
	 * @param b
	 * @return
	 */
	mtrc::numeric::DynamicVector<Tv> LDLsolver(const mtrc::numeric::DynamicVector<Tv> &b) const;

	/**
	 * @brief
	 *
	 */
	void debug() const;

	std::vector<size_t> col;
	std::vector<size_t> colptr;
	std::vector<size_t> rowval;
	std::vector<Tv> fval;
	std::vector<Tv> d;

  private:
	void forward(mtrc::numeric::DynamicVector<Tv> &y) const;
	void backward(mtrc::numeric::DynamicVector<Tv> &y) const;
};

/**
 * @class ApproxCholPQElem
 * @brief the data structure we use to keep track of degrees
 *
 */
class ApproxCholPQElem {
  public:
	/**
	 * @brief Construct a new ApproxCholPQElem object
	 *
	 */
	ApproxCholPQElem() {};

	/**
	 * @brief Construct a new ApproxCholPQElem object
	 *
	 * @param Aprev
	 * @param Anext
	 * @param Akey
	 */
	ApproxCholPQElem(const size_t Aprev, const size_t Anext, const size_t Akey) : prev(Aprev), next(Anext), key(Akey) {}

	size_t prev = 0;
	size_t next = 0;
	size_t key = 0;
};

/**
 * @class ApproxCholPQ
 * @brief An approximate priority queue.
 *   	  Items are bundled together into doubly-linked lists with all approximately the same key.
 *  	  minlist is the min list we know to be non-empty.
 *  	  It should always be a lower bound.
 *  	  keyMap maps keys to lists
 *
 */
class ApproxCholPQ {
  public:
	/**
	 * @brief Construct a new ApproxCholPQ object
	 *
	 * @param a
	 */
	explicit ApproxCholPQ(const std::vector<size_t> &a);

	/**
	 * @brief
	 *
	 * @param i
	 * @param newkey
	 * @param oldlist
	 * @param newlist
	 */
	void move(const size_t i, const size_t newkey, const size_t oldlist, const size_t newlist);

	/**
	 * @brief
	 *
	 * @param i
	 */
	void inc(const size_t i);

	/**
	 * @brief
	 *
	 * @param i
	 */
	void dec(const size_t i);

	/**
	 * @brief
	 *
	 * @return
	 */
	size_t pop();

	/**
	 * @brief
	 *
	 */
	void debug();

	std::vector<ApproxCholPQElem> elems; // indexed by node name
	std::vector<size_t> lists;
	size_t minlist;
	size_t nitems;
	size_t n;
};

// The approximate factorization

/**
 * @brief
 *
 * @param llmat
 * @param i
 * @param colspace
 * @return
 */
template <typename Tv> size_t get_ll_col(const LLmatp<Tv> &llmat, size_t i, std::vector<LLp<Tv> *> &colspace);

/**
 * @brief
 *
 * @param colspace
 * @param len
 */
template <typename Tv> void debugLLp(const std::vector<LLp<Tv> *> &colspace, size_t len);

/**
 * @brief
 *
 * @param colspace
 * @param len
 * @param pq
 * @return
 */
template <typename Tv> size_t compressCol(std::vector<LLp<Tv> *> &colspace, size_t len, ApproxCholPQ &pq);

// this one is greedy on the degree - also a big win

/**
 * @brief
 *
 * @tparam Tv
 * @param a
 * @return
 */
template <typename Tv> LDLinv<Tv> approxChol(LLmatp<Tv> &a);

/**
 * @brief
 *
 * @param a
 * @param pcgIts
 * @param tol
 * @param maxits
 * @param maxtime
 * @param verbose
 * @param params
 * @return
 */
template <typename Tv>
SubSolver<Tv> approxchol_lapGreedy(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
								   std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = 1000,
								   double maxtime = HUGE_VAL, bool verbose = false,
								   ApproxCholParams params = ApproxCholParams());

/**
 * @brief
 *
 * @param a
 * @param pcgIts
 * @param tol
 * @param maxits
 * @param maxtime
 * @param verbose
 * @param params
 * @return
 */
template <typename Tv>
inline SubSolver<Tv> approxchol_lap1(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
									 std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = 1000,
									 double maxtime = HUGE_VAL, bool verbose = false,
									 ApproxCholParams params = ApproxCholParams())
{
	return approxchol_lapGreedy(a, pcgIts, tol, maxits, maxtime, verbose, params);
}

/**
 * @brief Solver for the graph-Laplacian system L x = b, where L = diag(a*1) - a.
 *
 * Objective: the returned SubSolver computes the vertex potential x that minimizes the graph
 * Dirichlet energy E(x) = 1/2 * x^T L x - b^T x — equivalently the electrical-flow / effective-
 * resistance objective on the metric graph whose adjacency is `a`. On each connected component the
 * solution is fixed to the unique mean-zero representative of L's constant nullspace.
 *
 * Preconditions on `a` (validated; violations throw std::invalid_argument): square, symmetric, zero
 * diagonal, and strictly positive stored edge weights. The positivity check is enforced here at the
 * public entry so every dispatch arm — the native approxChol path AND the small-component dense
 * Cholesky path inside lapWrapComponents — shares one contract (the dense path does not otherwise
 * see the LLmatp positivity guard).
 *
 * @param a square symmetric adjacency matrix with strictly positive edge weights
 * @param pcgIts is an array for returning the number of pcgIterations.  Default is length 0.
 * @param tol is set to 1e-6 by default
 * @param maxits defaults to 1000
 * @param maxtime defaults to MAX_VAL.  It measures seconds.
 * @param verbose defaults to false
 * @param params additional ApproxCholParams
 * @return SubSolver<Tv>
 */
template <typename Tv>
inline SubSolver<Tv> approxchol_lap(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
									std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose,
									ApproxCholParams params)
{
	// Enforce the strictly-positive-edge-weight precondition for ALL components up front. The native
	// approxChol path checks this in the LLmatp ctor, but the small (<50 node) dense-Cholesky branch
	// of lapWrapComponents does not; scanning here gives every arm the same contract (same message as
	// the LLmatp guard).
	for (size_t j = 0; j < a.columns(); ++j)
		for (auto it = a.begin(j); it != a.end(j); ++it)
			if (!(it->value() > Tv(0)))
				throw std::invalid_argument("approxchol requires strictly positive adjacency edge weights");

	return lapWrapComponents(SolverA<Tv>(approxchol_lap1<Tv>), a, pcgIts, tol, maxits, maxtime, false);
}

// NOTE: A Julia-port `eigs`/`SqLinOp`/`aupd_wrapper` ARPACK eigensolver scaffold used to live
// here, fully commented out and referenced by nothing. It was removed: the Laplacian solver
// exposes approxchol_lap/pcg, and eigen-decomposition belongs in metric/numeric, not here.

} // namespace mtrc

#include "approxchol/approxchol.cpp"
#endif
