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

#ifndef _METRIC_UTILS_SOLVER_HELPER_SOLVERTYPES_HPP
#define _METRIC_UTILS_SOLVER_HELPER_SOLVERTYPES_HPP
#include <functional>

namespace metric {
//  Solvers for A*x=b where A is a matrix, b is a vector

/*	Arguments:

		*`tol` is set to 1e-6 by default,
		* `maxits` defaults to MAX_VAL
		* `maxtime` defaults to MAX_VAL.It measures seconds.
		* `verbose` defaults to false
		* `pcgIts` is an array for returning the number of pcgIterations.Default is length 0, in which case nothing is
   returned.
		* `params` is additional parameters.

			params = ApproxCholParams(order, output)
			order can be one of
			Deg(by degree, adaptive),
			WDeg(by original wted degree, nonadaptive),
			Given



*/

// Functor: pass A matrix, return SubSolver

template <typename Tv> class SolverA;

// Result of solver
// Functor: pass vector b, returns x vector
template <typename Tv> class SubSolver;

template <typename Tv> using SolverB = std::function<blaze::DynamicVector<Tv>(const blaze::DynamicVector<Tv> &)>;

/**
 * @class Factorization
 *
 * @brief
 */
template <typename Tv> class Factorization {
  public:
	blaze::CompressedMatrix<Tv, blaze::columnMajor> Lower;
};

/*
params = ApproxCholParams(order, output)
order can be one of
Deg(by degree, adaptive),
WDeg(by original wted degree, nonadaptive),
Given
*/

enum class ApproxCholEnum { Deg, WDeg, Given };

/**
 * @class ApproxCholParams
 *
 * @brief
 *
 */

class ApproxCholParams {
  public:
	ApproxCholParams()
	{
		order = ApproxCholEnum::Deg;
		stag_test = 5;
	}

	ApproxCholParams(ApproxCholEnum symb)
	{
		order = symb;
		stag_test = 5;
	}
	ApproxCholEnum order;
	long stag_test;
};

//  Types of solvers for A*x=B where B is a matrix

template <typename Tv>
using FactorSolver = std::function<Factorization<Tv>(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &)>;

// Result of wrappers
// Function: pass B matrix, returns x vector

template <typename Tv>
using SolverBMat = std::function<blaze::DynamicVector<Tv>(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &)>;

// Result of SolverA functor
// Convert SolverA to a function with 1 paramater B - SolverB
template <typename Tv>
using SubSolverFuncMat = std::function<blaze::DynamicVector<Tv>(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &,
																std::vector<size_t> &)>;

/**
 * @class SubSolverMat
 *
 * @brief
 *
 */
template <typename Tv> class SubSolverMat {
  public:
	/**
	 * @brief Construct a new SubSolverMat object
	 *
	 * @param Asolver
	 */
	explicit SubSolverMat(SubSolverFuncMat<Tv> Asolver) : Solver(Asolver){};

	/**
	 * @brief Construct a new SubSolverMat object
	 *
	 * @param solver
	 */
	explicit SubSolverMat(SolverBMat<Tv> solver)
	{
		Solver = [=](const blaze::CompressedMatrix<Tv, blaze::columnMajor> &b, std::vector<size_t> &pcgIts) {
			return solver(b);
		};
	}

	/**
	 * @brief
	 *
	 * @param b
	 * @param pcgIts
	 * @return
	 */
	blaze::DynamicVector<Tv> operator()(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &b,
										std::vector<size_t> &pcgIts)
	{
		return Solver(b, pcgIts);
	}

	/**
	 * @brief
	 *
	 * @param b
	 */
	blaze::DynamicVector<Tv> operator()(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &b)
	{
		std::vector<size_t> pcgIts;
		return Solver(b, pcgIts);
	}

  private:
	SubSolverFuncMat<Tv> Solver;
};

// Function: pass A matrix, return SubSolver
template <typename Tv>
using SolverAFuncMat =
	std::function<SubSolverMat<Tv>(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &, std::vector<size_t> &)>;

/**
 * @class SolverAMat
 *
 * @brief
 */
template <typename Tv> class SolverAMat {
  public:
	/**
	 * @brief Construct a new SolverAMat object
	 *
	 * @param solver
	 */
	explicit SolverAMat(SolverAFuncMat<Tv> solver) : Solver(solver) {}

	/**
	 * @brief
	 *
	 * @param a
	 * @param pcgIts
	 * @return
	 */
	SubSolverMat<Tv> operator()(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a, std::vector<size_t> &pcgIts)
	{

		return Solver(a, pcgIts);
	}

	/**
	 * @brief
	 *
	 * @param a
	 * @return
	 */
	SubSolverMat<Tv> operator()(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a)
	{
		std::vector<size_t> pcgIts;
		return Solver(a, pcgIts);
	}

  private:
	SolverAFuncMat<Tv> Solver;
};

// Result of functor SolverA
// Convert SolverA to a function with 1 paramater B - SolverB

template <typename Tv>
using SubSolverFunc = std::function<blaze::DynamicVector<Tv>(const blaze::DynamicVector<Tv> &, std::vector<size_t> &)>;
/**
 * @class SubSolver
 *
 * @brief
 */
template <typename Tv> class SubSolver {
  public:
	/**
	 * @brief Construct a new SubSolver object
	 *
	 */
	SubSolver() {}

	/**
	 * @brief Construct a new SubSolver object
	 *
	 * @param Asolver
	 */
	explicit SubSolver(SubSolverFunc<Tv> Asolver) : Solver(Asolver){};

	/**
	 * @brief Construct a new SubSolver object
	 *
	 * @param solver
	 */
	explicit SubSolver(SolverB<Tv> solver)
	{
		Solver = [=](const blaze::DynamicVector<Tv> &b, std::vector<size_t> &pcgIts) { return solver(b); };
	}

	/**
	 * @brief
	 *
	 * @param b
	 * @param pcgIts
	 */
	blaze::DynamicVector<Tv> operator()(const blaze::DynamicVector<Tv> &b, std::vector<size_t> &pcgIts)
	{
		return Solver(b, pcgIts);
	}

	/**
	 * @brief
	 *
	 * @param b
	 */
	blaze::DynamicVector<Tv> operator()(const blaze::DynamicVector<Tv> &b)
	{
		std::vector<size_t> pcgIts;
		return Solver(b, pcgIts);
	}

  private:
	SubSolverFunc<Tv> Solver;
};

template <typename Tv>
using SolverAFunc = std::function<SubSolver<Tv>(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &,
												std::vector<size_t> &, float, double, double, bool, ApproxCholParams)>;
/**
 * @class SovlerA
 *
 * @brief
 */
template <typename Tv> class SolverA {
  public:
	/**
	 * @brief Construct a new SolverA object
	 *
	 * @param solver
	 */
	explicit SolverA(SolverAFunc<Tv> solver) : Solver(solver) {}

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
	 * @return SubSolver<Tv>
	 */
	SubSolver<Tv> operator()(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a, std::vector<size_t> &pcgIts,
							 float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL,
							 bool verbose = false, ApproxCholParams params = ApproxCholParams())
	{
		return Solver(a, pcgIts, tol, maxits, maxtime, verbose, params);
	}

	/**
	 * @brief
	 *
	 * @param a
	 * @return SubSolver<Tv>
	 */
	SubSolver<Tv> operator()(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a)
	{
		std::vector<size_t> pcgIts;
		return Solver(a, pcgIts, 1e-6F, HUGE_VAL, HUGE_VAL, false, ApproxCholParams());
	}

  private:
	SolverAFunc<Tv> Solver;
};
} // namespace metric

#endif