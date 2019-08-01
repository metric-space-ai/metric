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


#pragma once

#include <blaze/Math.h>
#include <vector>
#include <chrono>
#include "graphalgs.h"
#include "solvertypes.h"

using namespace std;
using blaze::CompressedMatrix;
using blaze::DynamicMatrix;
using blaze::DynamicVector;
using namespace std::chrono;

namespace laplacians {

	template <typename Tv>
	inline DynamicVector<Tv> nullSolver(const DynamicVector<Tv>& a, vector<size_t>& pcg) {
		return DynamicVector<Tv>(1, 0);
	}

	//Cholesky-based Substitution

	template <typename Tv>
	inline DynamicVector<Tv> chol_subst(const CompressedMatrix<Tv, blaze::columnMajor> &Lower, const CompressedMatrix<Tv, blaze::columnMajor> &B) {
		DynamicVector<Tv> res(B.rows());
		DynamicMatrix<Tv, blaze::columnMajor> B1 = B, L = Lower;

		potrs(L, B1, 'L');

		res = column(B1, 0);

		return res;
	}

	template <typename Tv>
	inline DynamicVector<Tv> chol_subst(const CompressedMatrix<Tv, blaze::columnMajor> &Lower, const DynamicVector<Tv> &b) {

		DynamicMatrix<Tv, blaze::columnMajor> L = Lower;
		DynamicVector<Tv> b1 = b;

		potrs(L, b1, 'L');

		return b1;
	}

	// Function: pass A matrix, return A matrix factorization

	template <typename Tv>
	inline Factorization<Tv> cholesky(const CompressedMatrix<Tv, blaze::columnMajor> &A) {
		DynamicMatrix<Tv, blaze::columnMajor> A1(A), L;
		Factorization<Tv> F;

		blaze::llh(A1, L);

		F.Lower = L;

		return F;
	}
	
	//	lapWrapComponents function

	//Apply the ith solver on the ith component
	template <typename Tv>
	SubSolver<Tv> BlockSolver(const vector<vector<size_t>>& comps, vector<SubSolver<Tv>>& solvers,
		vector<size_t>& pcgIts, float tol = 1e-6F, double maxits = HUGE_VAL, double maxtime = HUGE_VAL,
		bool verbose = false);

	template <typename Tv>
	SubSolverMat<Tv> wrapInterfaceMat(const FactorSolver<Tv> solver, const CompressedMatrix<Tv, blaze::columnMajor>& a,
		vector<size_t>& pcgIts, float tol = 0,
		double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
		ApproxCholParams params = ApproxCholParams());

	template <typename Tv>
	SolverAMat<Tv> wrapInterfaceMat(const FactorSolver<Tv> solver) {
		return SolverAMat<Tv>([=](const CompressedMatrix<Tv, blaze::columnMajor> &a, vector<size_t>& pcgIts, float tol = 1e-6,
			double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false, ApproxCholParams params = ApproxCholParams())
		{
			return wrapInterfaceMat(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
		});
	}

	template <typename Tv>
	SubSolver<Tv> wrapInterface(const FactorSolver<Tv> solver, const CompressedMatrix<Tv, blaze::columnMajor>& a,
		vector<size_t>& pcgIts, float tol = 0,
		double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
		ApproxCholParams params = ApproxCholParams());

	template <typename Tv>
	SolverA<Tv> wrapInterface(const FactorSolver<Tv> solver) {
		return SolverA<Tv>([=](const CompressedMatrix<Tv, blaze::columnMajor> &a, vector<size_t>& pcgIts, float tol = 1e-6,
			double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false, ApproxCholParams params = ApproxCholParams())
		{
			return wrapInterface(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
		});
	}

	//This functions wraps cholfact so that it satisfies our interface.
	template <typename Tv>
	SolverAMat<Tv> chol_sddm_mat() {
		return wrapInterfaceMat<Tv>(cholesky<Tv>);
	}

	template <typename Tv>
	SolverA<Tv> chol_sddm() {
		return wrapInterface<Tv>(cholesky<Tv>);
	}

	// Applies a Laplacian `solver` that satisfies our interface to each connected component of the graph with adjacency matrix `a`.
	template <typename Tv>
	SubSolver<Tv> lapWrapConnected(SolverA<Tv> solver, const CompressedMatrix<Tv, blaze::columnMajor>& a, vector<size_t>& pcgIts,
		float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
		ApproxCholParams params = ApproxCholParams());

	template <typename Tv>
	inline SolverA<Tv> lapWrapConnected(const SolverA<Tv> solver) {
		return SolverA<Tv>([=](const CompressedMatrix<Tv, blaze::columnMajor> &a, vector<size_t>& pcgIts, float tol = 1e-6,
			double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
			const ApproxCholParams params = ApproxCholParams())
		{
			return lapWrapConnected(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
		});
	}

	template <typename Tv>
	SubSolver<Tv> lapWrapComponents(SolverA<Tv> solver, const CompressedMatrix<Tv, blaze::columnMajor>& a, vector<size_t>& pcgIts,
		float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
		ApproxCholParams params = ApproxCholParams());


	template <typename Tv>
	inline SolverA<Tv> lapWrapComponents(const SolverA<Tv> solver) {
		return SolverA<Tv>([=](const CompressedMatrix<Tv, blaze::columnMajor> &a, vector<size_t>& pcgIts, float tol = 1e-6,
			double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
			const ApproxCholParams params = ApproxCholParams())
		{
			return lapWrapComponents(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
		});
	}

	template <typename Tv>
	inline SolverA<Tv> lapWrapSDDM(SolverA<Tv> sddmSolver) {
		return lapWrapComponents(lapWrapConnected(sddmSolver));
	}

	template <typename Tv>
	SubSolver<Tv> sddmWrapLap(SolverA<Tv> lapSolver, const CompressedMatrix<Tv, blaze::columnMajor>& sddm, vector<size_t>& pcgIts,
		float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
		ApproxCholParams params = ApproxCholParams());

	template <typename Tv>
	inline SolverA<Tv> sddmWrapLap(const SolverA<Tv> solver) {

		return SolverA<Tv>([=](const CompressedMatrix<Tv, blaze::columnMajor> &a, vector<size_t>& pcgIts, float tol = 1e-6,
			double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
			const ApproxCholParams params = ApproxCholParams())
		{
			return sddmWrapLap(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
		});
	}
}

#include "lapwrappers.cpp"