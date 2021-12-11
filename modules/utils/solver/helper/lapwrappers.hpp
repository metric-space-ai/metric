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

#ifndef _METRIC_UTILS_SOLVER_HELPER_LAPWRAPPERS_HPP
#define _METRIC_UTILS_SOLVER_HELPER_LAPWRAPPERS_HPP
#include "blaze/Math.h"
#include <vector>
#include <chrono>
#include "graphalgs.hpp"
#include "solvertypes.hpp"

namespace metric {

/**
 * @brief 
 * 
 * @param a 
 * @param pcg 
 * @return
 */
template <typename Tv>
blaze::DynamicVector<Tv> nullSolver(const blaze::DynamicVector<Tv>& a, std::vector<size_t>& pcg);

/**
 * @brief Cholesky-based Substitution
 * 
 * @param Lower 
 * @param B 
 * @return
 */
template <typename Tv>
blaze::DynamicVector<Tv> chol_subst(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& Lower, const blaze::CompressedMatrix<Tv, blaze::columnMajor>& B);

/**
 * @brief 
 * 
 * @param Lower 
 * @param b 
 * @return
 */
template <typename Tv>
blaze::DynamicVector<Tv> chol_subst(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& Lower, const blaze::DynamicVector<Tv>& b);

/**
 * @brief Function: pass A matrix, return A matrix factorization
 * 
 * @param A 
 * @return
 */
template <typename Tv>
inline Factorization<Tv> cholesky(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A);

//	lapWrapComponents function

/**
 * @brief Apply the ith solver on the ith component
 * 
 * @param comps 
 * @param solvers 
 * @param pcgIts 
 * @param tol 
 * @param maxits 
 * @param maxtime 
 * @param verbose 
 * @return
 */
template <typename Tv>
SubSolver<Tv> BlockSolver(const std::vector<std::vector<size_t>>& comps, std::vector<SubSolver<Tv>>& solvers, std::vector<size_t>& pcgIts,
    float tol = 1e-6F, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false);

/**
 * @brief 
 * 
 * param solver 
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
SubSolverMat<Tv> wrapInterfaceMat(const FactorSolver<Tv> solver, const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a,
    std::vector<size_t>& pcgIts, float tol = 0, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
    ApproxCholParams params = ApproxCholParams());

/**
 * @brief 
 * 
 * @param solver 
 * @return
 */
template <typename Tv>
SolverAMat<Tv> wrapInterfaceMat(const FactorSolver<Tv> solver);

/**
 * @brief 
 * 
 * @param solver 
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
SubSolver<Tv> wrapInterface(const FactorSolver<Tv> solver, const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a,
    std::vector<size_t>& pcgIts, float tol = 0, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
    ApproxCholParams params = ApproxCholParams());

/**
 * @brief 
 * 
 * @param solver 
 * @return
 */
template <typename Tv>
SolverA<Tv> wrapInterface(const FactorSolver<Tv> solver);

/**
 * @brief This functions wraps cholfact so that it satisfies our interface.
 * 
 * @tparam Tv 
 * @return
 */
template <typename Tv>
SolverAMat<Tv> chol_sddm_mat();

/**
 * @brief 
 * 
 * @return
 */
template <typename Tv>
SolverA<Tv> chol_sddm();

/**
 * @brief Applies a Laplacian `solver` that satisfies our interface to each connected component of the graph with adjacency matrix `a`.
 * 
 * @param solver 
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
SubSolver<Tv> lapWrapConnected(SolverA<Tv> solver, const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a, std::vector<size_t>& pcgIts,
    float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
    ApproxCholParams params = ApproxCholParams());

/**
 * @brief 
 *
 * @param solver 
 * @return
 */
template <typename Tv>
inline SolverA<Tv> lapWrapConnected(const SolverA<Tv> solver);

/**
 * @brief 
 * 
 * @param solver 
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
SubSolver<Tv> lapWrapComponents(SolverA<Tv> solver, const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a, std::vector<size_t>& pcgIts,
    float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
    ApproxCholParams params = ApproxCholParams());

/**
 * @brief 
 * 
 * @param solver 
 * @return
 */
template <typename Tv>
inline SolverA<Tv> lapWrapComponents(const SolverA<Tv> solver);

/**
 * @brief 
 * 
 * @param sddmSolver 
 * @return
 */
template <typename Tv>
inline SolverA<Tv> lapWrapSDDM(SolverA<Tv> sddmSolver);

/**
 * @brief 
 * 
 * @param lapSolver 
 * @param sddm 
 * @param pcgIts 
 * @param tol 
 * @param maxits 
 * @param maxtime 
 * @param verbose 
 * @param params 
 * @return
 */
template <typename Tv>
SubSolver<Tv> sddmWrapLap(SolverA<Tv> lapSolver, const blaze::CompressedMatrix<Tv, blaze::columnMajor>& sddm, std::vector<size_t>& pcgIts,
    float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
    ApproxCholParams params = ApproxCholParams());

/**
 * @brief 
 * 
 * @param solver 
 * @return
 */
template <typename Tv>
inline SolverA<Tv> sddmWrapLap(const SolverA<Tv> solver);
}

#include "lapwrapper/lapwrappers.cpp"
#endif
