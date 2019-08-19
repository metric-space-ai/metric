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
	template<typename Tv>
	DynamicVector<Tv> pcg(const CompressedMatrix<Tv, blaze::columnMajor>& mat, const DynamicVector<Tv>& b, SolverB<Tv> pre,
		vector<size_t>& pcgIts, float tol, double maxits, double maxtime, bool verbose, size_t stag_test) {

		Tv al;

		size_t n = mat.rows();

		Tv nb = blaze::norm(b);

		// If input vector is zero, quit

		if (abs(nb) < 1e-6)
			return DynamicVector<Tv>(b.size(), 0);

		DynamicVector<Tv> x(n, 0), bestx(n, 0), r, z, p;

		double bestnr = 1.0;

		r = b;
		z = pre(b);
		p = z;

		Tv rho = dot(r, z);

		Tv best_rho = rho;
		size_t stag_count = 0;

		auto t1 = high_resolution_clock::now();

		size_t itcnt = 0;

		while (itcnt++ < maxits)
		{
			DynamicVector<Tv> q = mat * p;

			Tv pq = dot(p, q);

			if (pq < EPS || pq >= HUGE_VAL) {
				if (verbose)
					cout << endl << "PCG Stopped due to small or large pq";

				break;
			}

			al = rho / pq;

			// the following line could cause slowdown

			if (al * norm(p) < EPS * norm(x)) {
				if (verbose)
					cout << endl << "PCG: Stopped due to stagnation." << endl;

				break;
			}

			axpy2(al, p, x);
			axpy2(-al, q, r);

			Tv nr = norm(r) / nb;

			if (nr < bestnr) {
				bestnr = nr;
				bestx = x;
			}

			if (nr < tol)
				break;

			z = pre(r);

			Tv oldrho = rho;
			rho = dot(z, r); //this is gamma in hypre.

			if (stag_test != 0) // If stag_test=0 skip this check
				if (rho < best_rho * (1 - (Tv)(1 / stag_test))) {
					best_rho = rho;
					stag_count = 0;

				}
				else {
					if (stag_test > 0)
						if (best_rho > (1 - 1 / stag_test) * rho) {
							stag_count++;

							if (stag_count > stag_test)
							{
								if (verbose)
									cout << endl << "PCG Stopped by stagnation test.\n";

								break;
							}
						}
				}

			if (rho < EPS || rho >= HUGE_VAL) {
				if (verbose)
					cout << endl << "PCG Stopped due to small or large rho.\n";

				break;
			}

			Tv beta = rho / oldrho;

			if (beta < EPS || beta >= HUGE_VAL) {
				if (verbose)
					cout << endl << "PCG Stopped due to small or large beta.\n";

				break;
			}

			bzbeta(beta, p, z);

			auto t2 = high_resolution_clock::now();
			auto sec = duration_cast<milliseconds>(t2 - t1).count() * 1000;

			if (sec > maxtime)
			{
				if (verbose)
					cout << endl << "PCG stopped at maxtime.";

				break;
			}
		}

		auto t2 = high_resolution_clock::now();
		auto sec = duration_cast<milliseconds>(t2 - t1).count() * 1000;

		if (verbose) {
			cout << endl << "PCG stopped after: " << sec << " seconds and " << itcnt <<
				" iterations with relative error " << (norm(r) / norm(b)) << ".";
		}

		if (pcgIts.size())
			pcgIts[0] = itcnt;

		return bestx;
	}
}