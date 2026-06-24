// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Property and degenerate-case coverage for coordinate metric-space entropy.
//
// Entropy is a PROPERTY of an embedded coordinate finite metric space (a kpN
// local-Gaussian differential entropy estimator), not a direct source records+metric
// operator. The direct estimator tests below pin platform-robust mathematical
// properties; the public stats wrapper is exercised only through a MetricSpace.

#include <array>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"
#include "metric/correlation/entropy.hpp"
#include "metric/stats/properties/entropy.hpp"

namespace {

using Rec = std::vector<double>;
using Cheb = mtrc::Chebyshev<double>;

auto close(double actual, double expected, double tolerance) -> bool
{
	return std::abs(actual - expected) < tolerance;
}

} // namespace

int main()
{
	const std::vector<Rec> base = {{5.0, 5.0}, {2.0, 2.0}, {3.0, 3.0}, {5.0, 1.0}};
	const mtrc::Entropy<void, Cheb> entropy(Cheb(), 3, 2);

	// Determinism: a pure function of the (records, metric, k, p) inputs.
	const double first = entropy(base);
	const double second = entropy(base);
	assert(first == second);

	// Differential entropy can be (and here is) negative.
	assert(first < 0.0);

	// Regression anchor shared with metric_entropy_smoke / metric_space_entropy.
	assert(close(first, -4.4489104772539489, 1e-6));

	// Translation invariance: distances and local covariances are unchanged by a
	// global shift, so the differential-entropy estimate is invariant.
	std::vector<Rec> shifted;
	for (const auto &record : base) {
		shifted.push_back({record[0] + 100.0, record[1] + 100.0});
	}
	assert(close(entropy(shifted), first, 1e-5));

	// Spread ordering: a widely spread space has larger differential entropy than a
	// tightly clustered one under identical (metric, k, p).
	std::vector<Rec> clustered, spread;
	for (int i = 0; i < 16; ++i) {
		const double a = static_cast<double>(i);
		clustered.push_back({a * 0.01, std::fmod(a * 0.013, 0.1)});
		spread.push_back({a * 10.0, std::fmod(a * 13.0, 100.0)});
	}
	const mtrc::Entropy<void, Cheb> entropy_kp(Cheb(), 3, 5);
	const double clustered_h = entropy_kp(clustered);
	const double spread_h = entropy_kp(spread);
	assert(std::isfinite(clustered_h) && std::isfinite(spread_h));
	assert(spread_h > clustered_h + 1.0);

	// Exponentiated flag: remaps sub-1 differential entropies onto a strictly
	// positive, order-preserving scale.
	const mtrc::Entropy<void, Cheb> entropy_exp(Cheb(), 3, 5, true);
	const double clustered_exp = entropy_exp(clustered);
	const double spread_exp = entropy_exp(spread);
	assert(clustered_exp > 0.0 && spread_exp > 0.0);
	assert(spread_exp > clustered_exp);

	// Degenerate finite spaces: fewer than four records cannot form local
	// neighborhoods -> NaN "estimation failed" sentinel (not a crash).
	const std::vector<Rec> too_few = {{1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}};
	assert(std::isnan(entropy(too_few)));

	// Empty space: must yield the same NaN sentinel rather than out-of-bounds UB.
	const std::vector<Rec> empty;
	assert(std::isnan(entropy(empty)));

	// Duplicate records (a zero-diameter space): every eps-box collapses, so no point
	// yields a valid local Gaussian (got_results == 0) and the estimator returns the NaN
	// "estimation failed" sentinel rather than a meaningless finite digamma-only value.
	// Earlier versions returned finite garbage because the failure guard was inert.
	const std::vector<Rec> duplicates(6, Rec{4.0, 4.0});
	assert(std::isnan(entropy_kp(duplicates)));

	// Public stats-property wrapper: representation metadata and parity with the
	// direct estimator on a finite metric space.
	auto space = mtrc::make_space(base, Cheb());
	const auto wrapped = mtrc::stats::properties::entropy(space, 3, 2);
	assert(wrapped.algorithm == "entropy");
	assert(wrapped.representation == "metric_space");
	assert(wrapped.record_count == space.size());
	assert(wrapped.neighbor_count == 3);
	assert(wrapped.approximation_order == 2);
	assert(close(wrapped.value, first, 1e-12));

	// The wrapper forwards the NaN sentinel for degenerate spaces (no exception, no
	// crash); callers detect failure via std::isnan.
	const auto wrapped_degenerate =
		mtrc::stats::properties::entropy(mtrc::make_space(too_few, Cheb()), 3, 2);
	assert(std::isnan(wrapped_degenerate.value));

	std::cout << std::setprecision(17) << "entropy base=" << first << " spread=" << spread_h
			  << " clustered=" << clustered_h << "\n";

	return 0;
}
