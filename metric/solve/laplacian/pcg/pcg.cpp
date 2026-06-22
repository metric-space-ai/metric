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
#ifndef _METRIC_UTILS_SOLVER_PCG_CPP
#define _METRIC_UTILS_SOLVER_PCG_CPP
namespace mtrc {

template <typename Tv>
void axpy2(const Tv al, const mtrc::numeric::DynamicVector<Tv> &p, mtrc::numeric::DynamicVector<Tv> &x)
{
	x += al * p;
}

template <typename Tv>
void bzbeta(const Tv beta, mtrc::numeric::DynamicVector<Tv> &p, const mtrc::numeric::DynamicVector<Tv> &z)
{
	p = z + beta * p;
}

template <typename Tv>
mtrc::numeric::DynamicVector<Tv> pcg(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &mat,
									   const mtrc::numeric::DynamicVector<Tv> &b,
									   const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &pre,
									   std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime,
									   bool verbose, size_t stag_test)
{
	if (pre.rows() != pre.columns() || pre.rows() != mat.rows())
		throw std::invalid_argument("pcg preconditioner dimensions must match the system matrix");

	Factorization<Tv> fact = cholesky(pre);

	SolverB<Tv> F = [=](const mtrc::numeric::DynamicVector<Tv> &b) {
		mtrc::numeric::DynamicVector<Tv> x = chol_subst(fact.Lower, b);
		return x;
	};

	return pcg(mat, b, F, pcgIts, tol, maxits, maxtime, verbose, stag_test);
}

template <typename Tv>
SubSolver<Tv> pcgSolver(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &mat, SolverB<Tv> pre,
						std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose)
{

	return [=, &mat](const mtrc::numeric::DynamicVector<Tv> &b, std::vector<size_t> &pcgIts) {
		return pcg(mat, b, pre, pcgIts, tol, maxits, maxtime, verbose);
	};
}

template <typename Tv>
SubSolver<Tv> pcgSolver(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &mat,
						const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &pre,
						std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose)
{

	return [=, &mat](const mtrc::numeric::DynamicVector<Tv> &b, std::vector<size_t> &pcgIts) {
		return pcg(mat, b, pre, pcgIts, tol, maxits, maxtime, verbose);
	};
}

template <typename Tv>
mtrc::numeric::DynamicVector<Tv> pcg(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &mat,
									   const mtrc::numeric::DynamicVector<Tv> &b, SolverB<Tv> pre,
									   std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime,
									   bool verbose, size_t stag_test)
{
	if (mat.rows() != mat.columns())
		throw std::invalid_argument("pcg requires a square system matrix");

	if (b.size() != mat.rows())
		throw std::invalid_argument("pcg right-hand side dimension must match the system matrix");

	Tv al;

	size_t n = mat.rows();

	Tv nb = mtrc::numeric::norm(b);

	if (!std::isfinite(static_cast<double>(nb)))
		throw std::invalid_argument("pcg right-hand side must be finite");

	// Effective iteration cap. The public default maxits == HUGE_VAL would otherwise leave the
	// loop unbounded (an infinite hang on a non-converging operator); fall back to a generous
	// finite bound derived from the system size while preserving any caller-supplied value.
	double eff_maxits = maxits;
	if (std::isinf(eff_maxits)) {
		size_t cap = 10 * n;
		if (cap < 1000)
			cap = 1000;
		eff_maxits = static_cast<double>(cap);
	}

	// If the input vector is exactly zero, the unique solution is zero. Use an exact test
	// (faithful to the upstream norm(b) == 0) so legitimately small-scale systems are solved
	// rather than silently flattened to the zero vector by an absolute 1e-6 threshold.
	if (nb == Tv(0))
		return mtrc::numeric::DynamicVector<Tv>(b.size(), 0);

	mtrc::numeric::DynamicVector<Tv> x(n, 0), bestx(n, 0), r, z, p;

	double bestnr = 1.0;

	r = b;
	z = pre(b);

	if (z.size() != n)
		throw std::invalid_argument("pcg preconditioner output dimension must match the system matrix");

	p = z;

	Tv rho = dot(r, z);
	// Breakdown condition is positive-definiteness, not an absolute magnitude floor: dot(r, pre(r))
	// must be finite and strictly positive. An absolute `rho < EPS` test would spuriously reject a
	// well-posed but small-scale system (rho scales with ||b||^2), so test the sign instead.
	if (!std::isfinite(static_cast<double>(rho)) || rho <= Tv(0))
		throw std::invalid_argument("pcg requires a positive definite preconditioned residual");

	Tv best_rho = rho;
	size_t stag_count = 0;

	auto t1 = std::chrono::high_resolution_clock::now();

	size_t itcnt = 0;

	while (itcnt < eff_maxits) {
		++itcnt;

		mtrc::numeric::DynamicVector<Tv> q = mat * p;

		Tv pq = dot(p, q);

		// Curvature p^T (mat p) must be finite and strictly positive for an SPD operator. Use a
		// sign test rather than an absolute `pq < EPS` floor so a small-scale system is not cut
		// off prematurely (pq scales with the squared problem magnitude).
		if (!std::isfinite(static_cast<double>(pq)) || pq <= Tv(0)) {
			if (verbose)
				std::cout << std::endl << "PCG Stopped due to non-positive curvature";

			if (itcnt == 1)
				throw std::invalid_argument("pcg requires positive curvature in the initial search direction");

			break;
		}

		al = rho / pq;

		// the following line could cause slowdown

		if (al * norm(p) < EPS * norm(x)) {
			if (verbose)
				std::cout << std::endl << "PCG: Stopped due to stagnation." << std::endl;

			break;
		}

		axpy2(al, p, x);
		axpy2(-al, q, r);

		Tv nr = norm(r) / nb;

		// A non-finite residual means the iteration has been poisoned (e.g. by a NaN from an
		// indefinite operator/preconditioner). Relational guards alone cannot catch this
		// because every comparison with NaN is false, so stop explicitly and return bestx.
		if (!std::isfinite(static_cast<double>(nr))) {
			if (verbose)
				std::cout << std::endl << "PCG stopped due to a non-finite residual.";

			break;
		}

		if (nr < bestnr) {
			bestnr = nr;
			bestx = x;
		}

		if (nr < tol)
			break;

		z = pre(r);
		if (z.size() != n)
			throw std::invalid_argument("pcg preconditioner output dimension must match the system matrix");

		Tv oldrho = rho;
		rho = dot(z, r); // this is gamma in hypre.

		// Stagnation heuristic (stag_test == 0 disables it). NOTE: this is a running-minimum guard,
		// not the fixed k-step lookback `rho[it] > (1 - 1/k) rho[it-k]` once sketched in the upstream
		// comment. With factor = 1 - 1/stag_test: whenever rho drops below factor*best_rho we record a
		// new best and reset the counter; otherwise (rho failed to improve by the required factor) we
		// increment a stall counter and stop after stag_test consecutive non-improving iterations.
		// `stag_test` is unsigned, so the outer `!= 0` already implies `> 0` (the old extra guard was
		// dead).
		if (stag_test != 0) {
			const Tv stagnation_factor = Tv(1) - Tv(1) / Tv(stag_test);
			if (rho < best_rho * stagnation_factor) {
				best_rho = rho;
				stag_count = 0;
			} else if (best_rho > stagnation_factor * rho) {
				stag_count++;

				if (stag_count > stag_test) {
					if (verbose)
						std::cout << std::endl << "PCG Stopped by stagnation test.\n";

					break;
				}
			}
		}
		if (!std::isfinite(static_cast<double>(rho)) || rho <= Tv(0)) {
			if (verbose)
				std::cout << std::endl << "PCG Stopped due to non-positive rho.\n";

			break;
		}

		Tv beta = rho / oldrho;

		if (!std::isfinite(static_cast<double>(beta)) || beta <= Tv(0)) {
			if (verbose)
				std::cout << std::endl << "PCG Stopped due to small or large beta.\n";

			break;
		}

		bzbeta(beta, p, z);

		auto t2 = std::chrono::high_resolution_clock::now();
		const auto sec = std::chrono::duration<double>(t2 - t1).count();

		if (sec > maxtime) {
			if (verbose)
				std::cout << std::endl << "PCG stopped at maxtime.";

			break;
		}
	}

	auto t2 = std::chrono::high_resolution_clock::now();
	const auto sec = std::chrono::duration<double>(t2 - t1).count();

	if (verbose) {
		std::cout << std::endl
				  << "PCG stopped after: " << sec << " seconds and " << itcnt << " iterations with relative error "
				  << (norm(r) / norm(b)) << ".";
	}

	if (pcgIts.size())
		pcgIts[0] = itcnt;

	return bestx;
}
} // namespace mtrc
#endif
