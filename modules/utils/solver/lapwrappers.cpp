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

namespace laplacians {
	template <typename Tv>
	SubSolver<Tv> sddmWrapLap(SolverA<Tv> lapSolver, const CompressedMatrix<Tv, blaze::columnMajor>& sddm, vector<size_t>& pcgIts,
		float tol, double maxits, double maxtime, bool verbose,
		ApproxCholParams params)
	{
		CompressedMatrix<Tv, blaze::columnMajor> a;
		DynamicVector<Tv> d;

		auto [a, d] = adj(sddm);

		CompressedMatrix<Tv, blaze::columnMajor>a1 = extendMatrix(a, d);

		SubSolver<Tv> F = lapSolver(a1, pcgIts, tol, maxits, maxtime, verbose, params);

		return [=](const DynamicVector<Tv>& b, vector<size_t>& pcgIts) mutable {
			DynamicVector<Tv> sb(b.size() + 1);
			subvector(sb, 0, b.size()) = b;
			sb[b.size()] = -blaze::sum(b);

			DynamicVector<Tv> xaug = F(sb, pcgIts, tol, maxits, maxtime, verbose, params);

			xaug = xaug - DynamicVector<Tv>(xaug.size(), xaug[xaug.size() - 1]);

			return subvector(xaug, 0, a.rows() - 1);

		};
	}

	template <typename Tv>
	SubSolver<Tv> lapWrapComponents(SolverA<Tv> solver, const CompressedMatrix<Tv, blaze::columnMajor>& a, vector<size_t>& pcgIts,
		float tol, double maxits, double maxtime, bool verbose, ApproxCholParams params)
	{
		auto t1 = high_resolution_clock::now();

		/*if (!testZeroDiag(a)) {
			a = ClearDiag(a);
		}*/

		vector<size_t> co = components(a);

		if (*max_element(co.begin(), co.end()) == 1) {

			SubSolver<Tv> s = solver(a, pcgIts, tol, maxits, maxtime, verbose, params);

			if (verbose) {
				auto t2 = high_resolution_clock::now();
				auto msec = duration_cast<milliseconds>(t2 - t1).count();
				std::cout << "Solver build time: " << msec << " ms.";
			}

			return s;
		}
		else {
			vector<vector<size_t>>comps = vecToComps(co);
			vector<SubSolver<Tv>>solvers;

			for (size_t i = 0; i < comps.size(); ++i) {

				vector<size_t> ind = comps[i];

				CompressedMatrix<Tv> asub = index<Tv>(a, ind, ind);
				SubSolver<Tv> subSolver;

				if (ind.size() == 1) {

					subSolver = SubSolver<Tv>(nullSolver<Tv>);
				}
				else
					if (ind.size() < 50) {

						vector<size_t>pcgits;
						subSolver = lapWrapConnected<Tv>(chol_sddm<Tv>(), asub, pcgits);
					}
					else {
						subSolver = solver(a, pcgIts);
					}

				solvers.push_back(subSolver);
			}

			if (verbose) {
				auto t2 = high_resolution_clock::now();
				auto msec = duration_cast<milliseconds>(t2 - t1).count();
				std::cout << "Solver build time: " << msec << " ms.";
			}

			return BlockSolver(comps, solvers, pcgIts, tol, maxits, maxtime, verbose);
		}
	}

	template <typename Tv>
	SubSolver<Tv> lapWrapConnected(SolverA<Tv> solver, const CompressedMatrix<Tv, blaze::columnMajor>& a, vector<size_t>& pcgIts,
		float tol, double maxits, double maxtime, bool verbose, ApproxCholParams params) {

		CompressedMatrix<Tv, blaze::columnMajor> la = forceLap(a);
		size_t N = la.rows();

		size_t ind = findmax(diag(la)).second;

		vector<size_t> leave;

		// Delete the row with the max value

		for (size_t i = 0; i < N; ++i) {
			if (i != ind)
				leave.push_back(i);
		}

		CompressedMatrix<Tv, blaze::columnMajor>lasub = index<Tv>(la, leave, leave);
		SubSolver<Tv> subSolver = solver(lasub, pcgIts, tol, maxits, maxtime, verbose, params);

		return SubSolver<Tv>([=](const DynamicVector<Tv>& b, vector<size_t>& pcgIts) mutable {

			DynamicVector<Tv> bs = index(b, leave) - DynamicVector<Tv>(leave.size(), mean(b));

			DynamicVector<Tv> xs = subSolver(bs, pcgIts);

			DynamicVector<Tv> x(b.size(), 0);
			index(x, leave, xs);
			x = x - DynamicVector<Tv>(x.size(), mean(x));

			return x;
			});
	}

	template <typename Tv>
	SubSolver<Tv> wrapInterface(const FactorSolver<Tv> solver, const CompressedMatrix<Tv, blaze::columnMajor>& a,
		vector<size_t>& pcgIts, float, double maxits, double maxtime, bool verbose, ApproxCholParams params)
	{
		auto t1 = high_resolution_clock::now();

		Factorization<Tv> sol = solver(a);

		if (verbose) {
			auto t2 = high_resolution_clock::now();
			auto msec = duration_cast<milliseconds>(t2 - t1).count();
			std::cout << "Solver build time: " << msec << " ms.";
		}

		return SubSolver<Tv>([=](const DynamicVector<Tv>& b, vector<size_t>& pcgIts)->DynamicVector<Tv> {

			if (pcgIts.size())
				pcgIts[0] = 0;

			auto t1 = high_resolution_clock::now();

			DynamicVector<Tv> x = chol_subst(sol.Lower, b);

			if (verbose) {
				auto t2 = high_resolution_clock::now();
				auto msec = duration_cast<milliseconds>(t2 - t1).count();
				std::cout << "Solver build time: " << msec << " ms.";
			}

			return x;
			});
	}

	template <typename Tv>
	SubSolver<Tv> BlockSolver(const vector<vector<size_t>>& comps, vector<SubSolver<Tv>>& solvers,
		vector<size_t>& pcgIts, float tol, double maxits, double maxtime,
		bool verbose) {

		return SubSolver<Tv>([=](const DynamicVector<Tv>& b, vector<size_t>& pcgIts) mutable {

			vector<size_t> pcgTmp;

			if (pcgIts.size()) {
				pcgIts[0] = 0;
				pcgTmp.push_back(0);
			}

			DynamicVector<Tv>x(b.size(), 0);

			for (size_t i = 0; i < comps.size(); ++i) {
				vector<size_t> ind = comps[i];
				DynamicVector<Tv> bi = index(b, ind);
				DynamicVector<Tv> solution = (solvers[i])(bi, pcgTmp);

				index(x, ind, solution);

				if (pcgIts.size())
					pcgIts[0] = pcgIts[0] > pcgTmp[0] ? pcgIts[0] : pcgTmp[0];

			}

			return x;
			});
	}

	template <typename Tv>
	SubSolverMat<Tv> wrapInterfaceMat(const FactorSolver<Tv> solver, const CompressedMatrix<Tv, blaze::columnMajor>& a,
		vector<size_t>& pcgIts, float tol, double maxits, double maxtime, bool verbose,
		ApproxCholParams params)
	{
		auto t1 = high_resolution_clock::now();

		Factorization<Tv> sol = solver(a);

		if (verbose) {
			auto t2 = high_resolution_clock::now();
			auto msec = duration_cast<milliseconds>(t2 - t1).count();
			std::cout << "Solver build time: " << msec << " ms.";
		}

		return SubSolverMat<Tv>([=](const CompressedMatrix<Tv, blaze::columnMajor>& b, vector<size_t>& pcgIts) {

			if (pcgIts.size())
				pcgIts[0] = 0;

			auto t1 = high_resolution_clock::now();

			DynamicVector<Tv> x = chol_subst(sol.Lower, b);

			if (verbose) {
				auto t2 = high_resolution_clock::now();
				auto msec = duration_cast<milliseconds>(t2 - t1).count();
				std::cout << "Solver build time: " << msec << " ms.";
			}

			return x;
			});
	}

}
