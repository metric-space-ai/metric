// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Estimator-contract coverage for finite-metric-space entropy and VMixing.
//
// Entropy is a PROPERTY of a finite metric space (a kpN local-Gaussian, Kozachenko-
// Leonenko-style differential-entropy estimator); VMixing is a correlation/dependence
// quantity between two finite metric spaces. Neither is a metric.
//
// This file pins platform-ROBUST contracts only: determinism, the documented failure
// sentinels (NaN), container-shape invariance, throw conditions, and no-crash behaviour
// on degenerate / duplicate / zero-variance input. Exact regression magnitudes are
// estimator/platform sensitive and live in metric_entropy_smoke /
// metric_entropy_properties_smoke instead. Determinism is checked with a NaN-aware
// equality so a deterministic NaN result never looks like nondeterminism (NaN != NaN).

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "metric/correlation/entropy.hpp"
#include "metric/metric/catalog.hpp"

namespace {

using Rec = std::vector<double>;
using Cheb = mtrc::Chebyshev<double>;
using Eucl = mtrc::Euclidean<double>;

// Length-tolerant L2 over the common prefix: unlike the catalog metrics it does
// NOT throw on unequal-length records, so it lets ragged input reach the kpN
// estimator body. Metric is a free template argument of mtrc::Entropy, so this is
// a legitimate instantiation -- the equal-length guard must live in the estimator.
struct LenientL2 {
	template <typename A, typename B> double operator()(const A &a, const B &b) const
	{
		const std::size_t n = std::min(a.size(), b.size());
		double s = 0.0;
		for (std::size_t i = 0; i < n; ++i) {
			const double d = static_cast<double>(a[i]) - static_cast<double>(b[i]);
			s += d * d;
		}
		return std::sqrt(s);
	}
};

// Deterministic equality that also treats two NaNs as equal.
bool same(double a, double b) { return (a == b) || (std::isnan(a) && std::isnan(b)); }

} // namespace

int main()
{
	// ---- kpN Entropy, zero-diameter space -> NaN "estimation failed".
	// All records identical: every eps-box collapses, no point yields a valid local
	// Gaussian (got_results == 0). The estimator returns the NaN sentinel here instead of a
	// meaningless finite digamma-only value. Deterministic and platform-independent.
	{
		const std::vector<Rec> duplicates(6, Rec{4.0, 4.0});
		const mtrc::Entropy<void, Cheb> e(Cheb(), 3, 5);
		const double h = e(duplicates);
		assert(std::isnan(h));
		assert(same(h, e(duplicates))); // deterministic on the failure path
	}

	// ---- kpN Entropy, well-posed finite space: deterministic, finite, pure function of
	// (records, metric, k, p).
	{
		const std::vector<Rec> base = {{5.0, 5.0}, {2.0, 2.0}, {3.0, 3.0}, {5.0, 1.0}};
		const mtrc::Entropy<void, Cheb> e(Cheb(), 3, 2);
		const double a = e(base);
		assert(std::isfinite(a));
		assert(a == e(base));
	}

	// ---- kpN Entropy, near-degenerate space (nearly all identical, one slight outlier):
	// must not crash or invoke UB; the contract is simply "finite estimate or NaN
	// sentinel". This exercises the local-covariance regularization + EPMGP rejection
	// path without pinning a platform-sensitive magnitude.
	{
		std::vector<Rec> near_degenerate(8, Rec{1.0, 1.0});
		near_degenerate.push_back({1.0, 1.000001});
		const mtrc::Entropy<void, Cheb> e(Cheb(), 3, 5);
		const double h = e(near_degenerate);
		assert(std::isnan(h) || std::isfinite(h));
	}

	// ---- VMixing (kpN) and VMixing_simple: determinism, finiteness and container-shape
	// invariance on the historical reference pair. VMixing internally evaluates the kpN
	// entropy on X, Y and their concatenation; for this well-posed data all three sub-calls
	// run with got_results == n (no partial EPMGP failure).
	{
		const std::vector<Rec> X = {{5.0, 5.0}, {2.0, 2.0}, {3.0, 3.0}, {5.0, 0.0}};
		const std::vector<Rec> Y = {{5.0, 5.0}, {2.0, 2.0}, {3.0, 3.0}, {1.0, 0.0}};

		mtrc::VMixing<void, Eucl> vm;
		const double m = vm(X, Y);
		assert(std::isfinite(m));
		assert(m == vm(X, Y));

		mtrc::VMixing_simple<void, Eucl> vms;
		const double s = vms(X, Y);
		assert(std::isfinite(s));
		assert(s == vms(X, Y));

		// Shape invariance: deque records read the same metric values as vector records.
		const std::vector<std::deque<double>> Xd = {{5.0, 5.0}, {2.0, 2.0}, {3.0, 3.0}, {5.0, 0.0}};
		const std::vector<std::deque<double>> Yd = {{5.0, 5.0}, {2.0, 2.0}, {3.0, 3.0}, {1.0, 0.0}};
		assert(vm(Xd, Yd) == m);
		assert(vms(Xd, Yd) == s);
	}

	// ---- Throw contract: the non-kpN ("simple") estimators require strictly more than k
	// records and throw std::invalid_argument otherwise.
	{
		const std::vector<Rec> tiny = {{0.0, 0.0}, {1.0, 1.0}}; // n = 2 < k + 1 (default k = 3)
		mtrc::VMixing_simple<void, Eucl> vms;
		bool threw = false;
		try {
			vms(tiny, tiny);
		} catch (const std::invalid_argument &) {
			threw = true;
		}
		assert(threw);
	}

	// ---- kpN Entropy, ragged records: the estimator reads each record to
	// depth d = data[0].size(). With a length-tolerant metric (so tree.knn does not
	// throw first) unequal-length records would index past a short record's buffer
	// (heap-buffer-overflow) or silently truncate a longer one. The equal-length
	// guard must instead return the documented NaN "estimation failed" sentinel --
	// for BOTH a longer-than-rest record 0 and a shorter-than-rest record 0.
	{
		const mtrc::Entropy<void, LenientL2> e(LenientL2(), 3, 5);

		std::vector<Rec> longer_first = {{1.0, 2.0, 3.0, 4.0, 5.0}, {0.0, 0.0}, {1.0, 1.0}, {2.0, 0.0}, {0.0, 2.0}};
		const double h_long = e(longer_first);
		assert(std::isnan(h_long)); // no OOB read; documented failure sentinel

		std::vector<Rec> shorter_first = {{0.0, 0.0}, {1.0, 2.0, 3.0, 4.0, 5.0}, {1.0, 1.0}, {2.0, 0.0}, {0.0, 2.0}};
		const double h_short = e(shorter_first);
		assert(std::isnan(h_short)); // no silent truncation; documented failure sentinel
	}

	// ---- Monte-Carlo averaging estimate(): the default-seeded engine makes a single
	// build reproducible. Values are heuristic and not portable, so only the
	// same-input/same-output contract is pinned (NaN-aware). Well-posed, all-distinct,
	// well-spread data keeps every shuffled sample in general position so the estimate is
	// finite on any platform.
	{
		std::vector<Rec> A, B;
		for (int i = 0; i < 40; ++i) {
			A.push_back({i * 1.0, i * i * 0.5 + 0.25});
			B.push_back({i * 0.7 + 1.0, (40 - i) * 0.9});
		}
		mtrc::VMixing<void, Eucl> vm;
		const double e1 = vm.estimate(A, B, 8);
		const double e2 = vm.estimate(A, B, 8);
		assert(same(e1, e2));
		assert(std::isfinite(e1));

		mtrc::VMixing_simple<void, Eucl> vms;
		const double s1 = vms.estimate(A, B, 8);
		const double s2 = vms.estimate(A, B, 8);
		assert(same(s1, s2));
		assert(std::isfinite(s1));
	}

	// ---- estimate() on degenerate (duplicate-heavy) data must not crash. Such data makes
	// some shuffled subsamples zero-diameter, so the internal kpN entropy returns the NaN
	// sentinel; estimate() must skip those NaN samples rather than feed them into std::sort
	// (which would be undefined behavior). The contract here is "no crash + deterministic
	// finite-or-NaN result", not a magnitude.
	{
		std::vector<Rec> A, B;
		for (int i = 0; i < 30; ++i) {
			A.push_back(Rec{4.0, 4.0}); // all identical -> many degenerate subsamples
			B.push_back({i % 2 == 0 ? 1.0 : 2.0, 0.0});
		}
		mtrc::VMixing<void, Eucl> vm;
		const double d1 = vm.estimate(A, B, 6);
		const double d2 = vm.estimate(A, B, 6);
		assert(same(d1, d2));               // deterministic (NaN-aware)
		assert(std::isnan(d1) || std::isfinite(d1)); // never UB / never a crash
	}

	std::cout << "metric_entropy_estimator_smoke OK\n";
	return 0;
}
