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

namespace metric {

template <typename Tv>
inline blaze::DynamicVector<Tv> nullSolver(const blaze::DynamicVector<Tv> &a, std::vector<size_t> &pcg)
{
	return blaze::DynamicVector<Tv>(1, 0);
}

template <typename Tv>
inline blaze::DynamicVector<Tv> chol_subst(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &Lower,
										   const blaze::CompressedMatrix<Tv, blaze::columnMajor> &B)
{
	blaze::DynamicVector<Tv> res(B.rows());
	blaze::DynamicMatrix<Tv, blaze::columnMajor> B1 = B, L = Lower;

	potrs(L, B1, 'L');

	res = column(B1, 0);

	return res;
}

template <typename Tv>
blaze::DynamicVector<Tv> chol_subst(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &Lower,
									const blaze::DynamicVector<Tv> &b)
{

	blaze::DynamicMatrix<Tv, blaze::columnMajor> L = Lower;
	blaze::DynamicVector<Tv> b1 = b;

	potrs(L, b1, 'L');

	return b1;
}

template <typename Tv> inline Factorization<Tv> cholesky(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A)
{
	blaze::DynamicMatrix<Tv, blaze::columnMajor> A1(A), L;
	Factorization<Tv> F;

	blaze::llh(A1, L);

	F.Lower = L;

	return F;
}

template <typename Tv>
SubSolver<Tv> sddmWrapLap(SolverA<Tv> lapSolver, const blaze::CompressedMatrix<Tv, blaze::columnMajor> &sddm,
						  std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose,
						  ApproxCholParams params)
{
	auto [a, d] = adj(sddm);

	blaze::CompressedMatrix<Tv, blaze::columnMajor> a1 = extendMatrix(a, d);

	SubSolver<Tv> F = lapSolver(a1, pcgIts, tol, maxits, maxtime, verbose, params);

	return [a = a, F, tol, maxits, maxtime, verbose, params](const blaze::DynamicVector<Tv> &b,
															 std::vector<size_t> &pcgIts) mutable {
		blaze::DynamicVector<Tv> sb(b.size() + 1);
		subvector(sb, 0, b.size()) = b;
		sb[b.size()] = -blaze::sum(b);

		blaze::DynamicVector<Tv> xaug = F(sb, pcgIts, tol, maxits, maxtime, verbose, params);

		xaug = xaug - blaze::DynamicVector<Tv>(xaug.size(), xaug[xaug.size() - 1]);

		return subvector(xaug, 0, a.rows() - 1);
	};
}

template <typename Tv>
SubSolver<Tv> lapWrapComponents(SolverA<Tv> solver, const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a,
								std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose,
								ApproxCholParams params)
{
	auto t1 = std::chrono::high_resolution_clock::now();

	std::vector<size_t> co = components(a);

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

			blaze::CompressedMatrix<Tv> asub = index<Tv>(a, ind, ind);
			SubSolver<Tv> subSolver;

			if (ind.size() == 1) {

				subSolver = SubSolver<Tv>(nullSolver<Tv>);
			} else if (ind.size() < 50) {

				std::vector<size_t> pcgits;
				subSolver = lapWrapConnected<Tv>(chol_sddm<Tv>(), asub, pcgits);
			} else {
				subSolver = solver(a, pcgIts);
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
SubSolver<Tv> lapWrapConnected(SolverA<Tv> solver, const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a,
							   std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose,
							   ApproxCholParams params)
{

	blaze::CompressedMatrix<Tv, blaze::columnMajor> la = forceLap(a);
	size_t N = la.rows();

	size_t ind = findmax(diag(la)).second;

	std::vector<size_t> leave;

	// Delete the row with the max value

	for (size_t i = 0; i < N; ++i) {
		if (i != ind)
			leave.push_back(i);
	}

	blaze::CompressedMatrix<Tv, blaze::columnMajor> lasub = index<Tv>(la, leave, leave);
	SubSolver<Tv> subSolver = solver(lasub, pcgIts, tol, maxits, maxtime, verbose, params);

	return SubSolver<Tv>([=](const blaze::DynamicVector<Tv> &b, std::vector<size_t> &pcgIts) mutable {
		blaze::DynamicVector<Tv> bs = index(b, leave) - blaze::DynamicVector<Tv>(leave.size(), mean(b));

		blaze::DynamicVector<Tv> xs = subSolver(bs, pcgIts);

		blaze::DynamicVector<Tv> x(b.size(), 0);
		index(x, leave, xs);
		x = x - blaze::DynamicVector<Tv>(x.size(), mean(x));

		return x;
	});
}

template <typename Tv>
SubSolver<Tv> wrapInterface(const FactorSolver<Tv> solver, const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a,
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

	return SubSolver<Tv>(
		[=](const blaze::DynamicVector<Tv> &b, std::vector<size_t> &pcgIts) -> blaze::DynamicVector<Tv> {
			if (pcgIts.size())
				pcgIts[0] = 0;

			auto t1 = std::chrono::high_resolution_clock::now();

			blaze::DynamicVector<Tv> x = chol_subst(sol.Lower, b);

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

	return SubSolver<Tv>([=](const blaze::DynamicVector<Tv> &b, std::vector<size_t> &pcgIts) mutable {
		std::vector<size_t> pcgTmp;

		if (pcgIts.size()) {
			pcgIts[0] = 0;
			pcgTmp.push_back(0);
		}

		blaze::DynamicVector<Tv> x(b.size(), 0);

		for (size_t i = 0; i < comps.size(); ++i) {
			std::vector<size_t> ind = comps[i];
			blaze::DynamicVector<Tv> bi = index(b, ind);
			blaze::DynamicVector<Tv> solution = (solvers[i])(bi, pcgTmp);

			index(x, ind, solution);

			if (pcgIts.size())
				pcgIts[0] = pcgIts[0] > pcgTmp[0] ? pcgIts[0] : pcgTmp[0];
		}

		return x;
	});
}

template <typename Tv>
SubSolverMat<Tv> wrapInterfaceMat(const FactorSolver<Tv> solver,
								  const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a, std::vector<size_t> &pcgIts,
								  float tol, double maxits, double maxtime, bool verbose, ApproxCholParams params)
{
	auto t1 = std::chrono::high_resolution_clock::now();

	Factorization<Tv> sol = solver(a);

	if (verbose) {
		auto t2 = std::chrono::high_resolution_clock::now();
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		std::cout << "Solver build time: " << msec << " ms.";
	}

	return SubSolverMat<Tv>([=](const blaze::CompressedMatrix<Tv, blaze::columnMajor> &b, std::vector<size_t> &pcgIts) {
		if (pcgIts.size())
			pcgIts[0] = 0;

		auto t1 = std::chrono::high_resolution_clock::now();

		blaze::DynamicVector<Tv> x = chol_subst(sol.Lower, b);

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
	return SolverAMat<Tv>([=](const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a, std::vector<size_t> &pcgIts,
							  float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL,
							  bool verbose = false, ApproxCholParams params = ApproxCholParams()) {
		return wrapInterfaceMat(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
	});
}

template <typename Tv> SolverA<Tv> wrapInterface(const FactorSolver<Tv> solver)
{
	return SolverA<Tv>([=](const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a, std::vector<size_t> &pcgIts,
						   float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
						   ApproxCholParams params = ApproxCholParams()) {
		return wrapInterface(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
	});
}

template <typename Tv> SolverAMat<Tv> chol_sddm_mat() { return wrapInterfaceMat<Tv>(cholesky<Tv>); }

template <typename Tv> SolverA<Tv> chol_sddm() { return wrapInterface<Tv>(cholesky<Tv>); }

template <typename Tv> inline SolverA<Tv> lapWrapConnected(const SolverA<Tv> solver)
{
	return SolverA<Tv>([=](const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a, std::vector<size_t> &pcgIts,
						   float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
						   const ApproxCholParams params = ApproxCholParams()) {
		return lapWrapConnected(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
	});
}

template <typename Tv> inline SolverA<Tv> lapWrapComponents(const SolverA<Tv> solver)
{
	return SolverA<Tv>([=](const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a, std::vector<size_t> &pcgIts,
						   float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
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
	return SolverA<Tv>([=](const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a, std::vector<size_t> &pcgIts,
						   float tol = 1e-6, double maxits = HUGE_VAL, double maxtime = HUGE_VAL, bool verbose = false,
						   const ApproxCholParams params = ApproxCholParams()) {
		return sddmWrapLap(solver, a, pcgIts, tol, maxits, maxtime, verbose, params);
	});
}
} // namespace metric
#endif
