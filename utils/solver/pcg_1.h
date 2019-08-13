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

/*
	x = pcg(mat, b, pre; tol, maxits, maxtime, verbose, pcgIts, stag_test)

	solves a symmetric linear system using preconditioner `pre`.
	# Arguments
	* `pre` can be a function or a matrix.  If a matrix, a function to solve it is created with cholFact.
	* `tol` is set to 1e-6 by default,
	* `maxits` defaults to MAX_VAL
	* `maxtime` defaults to MAX_VAL.  It measures seconds.
	* `verbose` defaults to false
	* `pcgIts` is an array for returning the number of pcgIterations.  Default is length 0, in which case nothing is returned.
	* `stag_test=k` stops the code if rho[it] > (1-1/k) rho[it-k].  Set to 0 to deactivate.
*/

#pragma once
#include <functional>
#include <chrono>
#include <blaze/Math.h>
#include "graphalgs.h"
#include "solvertypes.h"

using namespace std;
using blaze::CompressedMatrix;
using blaze::DynamicVector;
using namespace std::chrono;

namespace laplacians {

	const double EPS = 2.220446049250313e-16;

	template<typename Tv>
	void axpy2(const Tv al, const DynamicVector<Tv> &p, DynamicVector<Tv> &x) {
		x += al * p;
	}

	template <typename Tv>
	inline void bzbeta(const Tv beta, DynamicVector<Tv> &p, const DynamicVector<Tv> &z) {
		p = z + beta * p;
	}

	template<typename Tv>
	DynamicVector<Tv> pcg(const CompressedMatrix<Tv, blaze::columnMajor>& mat, const DynamicVector<Tv>& b, SolverB<Tv> pre,
		vector<size_t>& pcgIts, float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
		size_t stag_test = 0);

	template<typename Tv>
	DynamicVector<Tv> pcg(const CompressedMatrix<Tv, blaze::columnMajor>& mat, const DynamicVector<Tv> &b, const CompressedMatrix<Tv, blaze::columnMajor>& pre,
		vector<size_t>& pcgIts, float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
		size_t stag_test = 0)
	{
		Factorization<Tv> fact = cholesky(pre);

		SolverB<Tv> F = [=](const DynamicVector<Tv> &b) {
			DynamicVector<Tv> x = chol_subst(fact.Lower, b);
			return x;
		};

		return pcg(mat, b, F, pcgIts, tol, maxits, maxtime, verbose);
	}

	template<typename Tv>
	SubSolver<Tv> pcgSolver(const CompressedMatrix<Tv, blaze::columnMajor>& mat, SolverB<Tv> pre, vector<size_t>& pcgIts,
		float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false) {

		return [=, &mat](const DynamicVector<Tv> &b, vector<size_t>& pcgIts) {
			return pcg(mat, b, pre, pcgIts, tol, maxits, maxtime, verbose);
		};
	}

	template<typename Tv>
	SubSolver<Tv> pcgSolver(const CompressedMatrix<Tv, blaze::columnMajor>& mat, const CompressedMatrix<Tv, blaze::columnMajor>& pre, vector<size_t>& pcgIts,
		float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false) {

		return [=, &mat](const DynamicVector<Tv> &b, vector<size_t>& pcgIts) {
			return pcg(mat, b, pre, pcgIts, tol, maxits, maxtime, verbose);
		};
	}
}

#include "pcg_1.cpp"