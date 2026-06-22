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
#ifndef _METRIC_UTILS_SOLVER_HELPER_LAPWRAPPERS_CPP
#define _METRIC_UTILS_SOLVER_HELPER_LAPWRAPPERS_CPP

namespace mtrc {

template <typename Tv>
inline mtrc::numeric::DynamicVector<Tv> nullSolver(const mtrc::numeric::DynamicVector<Tv> &a,
													 std::vector<size_t> &pcg)
{
	return mtrc::numeric::DynamicVector<Tv>(a.size(), Tv(0));
}

template <typename Tv>
inline mtrc::numeric::DynamicVector<Tv>
chol_subst(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &Lower,
		   const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &B)
{
	mtrc::numeric::DynamicVector<Tv> res(B.rows());
	mtrc::numeric::DynamicMatrix<Tv, mtrc::numeric::columnMajor> B1 = B, L = Lower;

	potrs(L, B1, 'L');

	res = column(B1, 0);

	return res;
}

template <typename Tv>
mtrc::numeric::DynamicVector<Tv>
chol_subst(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &Lower,
		   const mtrc::numeric::DynamicVector<Tv> &b)
{

	mtrc::numeric::DynamicMatrix<Tv, mtrc::numeric::columnMajor> L = Lower;
	mtrc::numeric::DynamicVector<Tv> b1 = b;

	potrs(L, b1, 'L');

	return b1;
}

template <typename Tv>
inline Factorization<Tv> cholesky(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &A)
{
	mtrc::numeric::DynamicMatrix<Tv, mtrc::numeric::columnMajor> A1(A), L;
	Factorization<Tv> F;

	mtrc::numeric::llh(A1, L);

	F.Lower = L;

	return F;
}

template <typename Tv>
SubSolver<Tv> sddmWrapLap(SolverA<Tv> lapSolver,
						  const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &sddm,
						  std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose,
						  ApproxCholParams params)
{
	auto [a, d] = adj(sddm);

	mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> a1 = extendMatrix(a, d);

	SubSolver<Tv> F = lapSolver(a1, pcgIts, tol, maxits, maxtime, verbose, params);

	return SubSolver<Tv>([a = a, F](const mtrc::numeric::DynamicVector<Tv> &b,
									std::vector<size_t> &pcgIts) mutable -> mtrc::numeric::DynamicVector<Tv> {
		mtrc::numeric::DynamicVector<Tv> sb(b.size() + 1);
		subvector(sb, 0, b.size()) = b;
		sb[b.size()] = -mtrc::numeric::sum(b);

		// F is the prebuilt Laplacian sub-solver; it is invoked as F(rhs, pcgIts). The solver
		// parameters (tol/maxits/...) were already bound when F was constructed above.
		mtrc::numeric::DynamicVector<Tv> xaug = F(sb, pcgIts);

		xaug = xaug - mtrc::numeric::DynamicVector<Tv>(xaug.size(), xaug[xaug.size() - 1]);

		// Return all n original unknowns (a.rows() == n); the augmented system has size n + 1,
		// so the last entry to drop is the appended grounding node, not the last real unknown.
		// Materialize an owning vector (not a view into the local xaug).
		mtrc::numeric::DynamicVector<Tv> result = subvector(xaug, 0, a.rows());
		return result;
	});
}

template <typename Tv>
SubSolver<Tv> lapWrapComponents(SolverA<Tv> solver,
								const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
								std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose,
								ApproxCholParams params)
{
	auto t1 = std::chrono::high_resolution_clock::now();

	std::vector<size_t> co = components(a);

	// An order-0 graph has no components; max_element would dereference end(). There is nothing
	// to solve, so hand back the identity (null) solver instead of reading past the empty range.
	if (co.empty())
		return SubSolver<Tv>(nullSolver<Tv>);

	// A single isolated vertex has a zero Laplacian and only the constant nullspace. The canonical
	// mean-zero solution is the zero vector; do not build ApproxCholPQ with degree 0, which would
	// underflow its degree-minus-one bucket key.
	if (a.rows() <= 1)
		return SubSolver<Tv>(nullSolver<Tv>);

	if (*max_element(co.begin(), co.end()) == 1) {

		SubSolver<Tv> s = solver(a, pcgIts, tol, maxits, maxtime, verbose, params);

		if (verbose) {
			auto t2 = std::chrono::high_resolution_clock::now();
			auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			std::cout << "Solver build time: " << msec << " ms.";
		}

		return s;
	} else {
		std::vector<std::vector<size_t>> comps = vecToComps(co);
		std::vector<SubSolver<Tv>> solvers;

		for (size_t i = 0; i < comps.size(); ++i) {

			std::vector<size_t> ind = comps[i];

			mtrc::numeric::CompressedMatrix<Tv> asub = index<Tv>(a, ind, ind);
			SubSolver<Tv> subSolver;

			if (ind.size() == 1) {

				subSolver = SubSolver<Tv>(nullSolver<Tv>);
			} else if (ind.size() < 50) {

				std::vector<size_t> pcgits;
				subSolver = lapWrapConnected<Tv>(chol_sddm<Tv>(), asub, pcgits);
			} else {
				// Factorize the per-component submatrix `asub`, not the full graph `a`:
				// BlockSolver drives this solver with a component-sized right-hand side, so
				// building it from `a` (size N) would mismatch dimensions. Forward the
				// caller's solver parameters as every other dispatch arm does.
				subSolver = solver(asub, pcgIts, tol, maxits, maxtime, verbose, params);
			}

			solvers.push_back(subSolver);
		}

		if (verbose) {
			auto t2 = std::chrono::high_resolution_clock::now();
			auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			std::cout << "Solver build time: " << msec << " ms.";
		}

		return BlockSolver(comps, solvers, pcgIts, tol, maxits, maxtime, verbose);
	}
}

template <typename Tv>
SubSolver<Tv> lapWrapConnected(SolverA<Tv> solver,
							   const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
							   std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose,
							   ApproxCholParams params)
{

	mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> la = forceLap(a);
	size_t N = la.rows();

	size_t ind = findmax(diag(la)).second;

	std::vector<size_t> leave;

	// Delete the row with the max value

	for (size_t i = 0; i < N; ++i) {
		if (i != ind)
			leave.push_back(i);
	}

	mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> lasub = index<Tv>(la, leave, leave);
	SubSolver<Tv> subSolver = solver(lasub, pcgIts, tol, maxits, maxtime, verbose, params);

	return SubSolver<Tv>([=](const mtrc::numeric::DynamicVector<Tv> &b, std::vector<size_t> &pcgIts) mutable {
		mtrc::numeric::DynamicVector<Tv> bs =
			index(b, leave) - mtrc::numeric::DynamicVector<Tv>(leave.size(), mean(b));

		mtrc::numeric::DynamicVector<Tv> xs = subSolver(bs, pcgIts);

		mtrc::numeric::DynamicVector<Tv> x(b.size(), 0);
		index(x, leave, xs);
		x = x - mtrc::numeric::DynamicVector<Tv>(x.size(), mean(x));

		return x;
	});
}

template <typename Tv>
SubSolver<Tv> wrapInterface(const FactorSolver<Tv> solver,
							const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
							std::vector<size_t> &pcgIts, float, double maxits, double maxtime, bool verbose,
							ApproxCholParams params)
{
	auto t1 = std::chrono::high_resolution_clock::now();

	Factorization<Tv> sol = solver(a);

	if (verbose) {
		auto t2 = std::chrono::high_resolution_clock::now();
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		std::cout << "Solver build time: " << msec << " ms.";
	}

	return SubSolver<Tv>([=](const mtrc::numeric::DynamicVector<Tv> &b,
							 std::vector<size_t> &pcgIts) -> mtrc::numeric::DynamicVector<Tv> {
		if (pcgIts.size())
			pcgIts[0] = 0;

		auto t1 = std::chrono::high_resolution_clock::now();

		mtrc::numeric::DynamicVector<Tv> x = chol_subst(sol.Lower, b);

		if (verbose) {
			auto t2 = std::chrono::high_resolution_clock::now();
			auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			std::cout << "Solver build time: " << msec << " ms.";
		}

		return x;
	});
}

template <typename Tv>
SubSolver<Tv> BlockSolver(const std::vector<std::vector<size_t>> &comps, std::vector<SubSolver<Tv>> &solvers,
						  std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose)
{

	return SubSolver<Tv>([=](const mtrc::numeric::DynamicVector<Tv> &b, std::vector<size_t> &pcgIts) mutable {
		std::vector<size_t> pcgTmp;

		if (pcgIts.size()) {
			pcgIts[0] = 0;
			pcgTmp.push_back(0);
		}

		mtrc::numeric::DynamicVector<Tv> x(b.size(), 0);

		for (size_t i = 0; i < comps.size(); ++i) {
			std::vector<size_t> ind = comps[i];
			mtrc::numeric::DynamicVector<Tv> bi = index(b, ind);
			mtrc::numeric::DynamicVector<Tv> solution = (solvers[i])(bi, pcgTmp);

			index(x, ind, solution);

			if (pcgIts.size())
				pcgIts[0] = pcgIts[0] > pcgTmp[0] ? pcgIts[0] : pcgTmp[0];
		}

		return x;
	});
}

template <typename Tv>
SubSolverMat<Tv> wrapInterfaceMat(const FactorSolver<Tv> solver,
								  const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
								  std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose,
								  ApproxCholParams params)
{
	auto t1 = std::chrono::high_resolution_clock::now();

	Factorization<Tv> sol = solver(a);

	if (verbose) {
		auto t2 = std::chrono::high_resolution_clock::now();
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		std::cout << "Solver build time: " << msec << " ms.";
	}

	return SubSolverMat<Tv>(
		[=](const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &b, std::vector<size_t> &pcgIts) {
			if (pcgIts.size())
				pcgIts[0] = 0;

			auto t1 = std::chrono::high_resolution_clock::now();

			mtrc::numeric::DynamicVector<Tv> x = chol_subst(sol.Lower, b);

			if (verbose) {
				auto t2 = std::chrono::high_resolution_clock::now();
				auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

				std::cout << "Solver build time: " << msec << " ms.";
			}

			return x;
		});
}

template <typename Tv> SolverAMat<Tv> wrapInterfaceMat(const FactorSolver<Tv> solver)
{
	return SolverAMat<Tv>([=](const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
							  std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = HUGE_VAL,
							  double maxtime = HUGE_VAL, bool verbose = false,
							  ApproxCholParams params = ApproxCholParams()) {
		return wrapInterfaceMat(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
	});
}

template <typename Tv> SolverA<Tv> wrapInterface(const FactorSolver<Tv> solver)
{
	return SolverA<Tv>([=](const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
						   std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = HUGE_VAL,
						   double maxtime = HUGE_VAL, bool verbose = false,
						   ApproxCholParams params = ApproxCholParams()) {
		return wrapInterface(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
	});
}

template <typename Tv> SolverAMat<Tv> chol_sddm_mat() { return wrapInterfaceMat<Tv>(cholesky<Tv>); }

template <typename Tv> SolverA<Tv> chol_sddm() { return wrapInterface<Tv>(cholesky<Tv>); }

template <typename Tv> inline SolverA<Tv> lapWrapConnected(const SolverA<Tv> solver)
{
	return SolverA<Tv>([=](const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
						   std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = HUGE_VAL,
						   double maxtime = HUGE_VAL, bool verbose = false,
						   const ApproxCholParams params = ApproxCholParams()) {
		return lapWrapConnected(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
	});
}

template <typename Tv> inline SolverA<Tv> lapWrapComponents(const SolverA<Tv> solver)
{
	return SolverA<Tv>([=](const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
						   std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = HUGE_VAL,
						   double maxtime = HUGE_VAL, bool verbose = false,
						   const ApproxCholParams params = ApproxCholParams()) {
		return lapWrapComponents(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
	});
}

template <typename Tv> inline SolverA<Tv> lapWrapSDDM(SolverA<Tv> sddmSolver)
{
	return lapWrapComponents(lapWrapConnected(sddmSolver));
}

template <typename Tv> inline SolverA<Tv> sddmWrapLap(const SolverA<Tv> solver)
{
	return SolverA<Tv>([=](const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &a,
						   std::vector<size_t> &pcgIts, float tol = 1e-6, double maxits = HUGE_VAL,
						   double maxtime = HUGE_VAL, bool verbose = false,
						   const ApproxCholParams params = ApproxCholParams()) {
		return sddmWrapLap(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
	});
}
} // namespace mtrc
#endif
