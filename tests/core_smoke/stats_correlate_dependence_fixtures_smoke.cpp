// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Dependence-testing fixtures for mtrc::stats::correlate (MGC), Level-1 stats.
//
// MGC is a DEPENDENCE/correlation statistic between two paired finite metric spaces, NEVER a
// metric. This test pins behaviours the existing smokes do not cover: (1) a strongly dependent
// pair scores far above an independent (fixed-permutation) pair; (2) a fixed permutation that
// destroys the pairing collapses the statistic well below the identity value of 1; (3) the raw
// MGC::estimate()/xcorr() entry points now reject mismatched paired inputs with a defined
// exception (previously NDEBUG-stripped asserts); and (4) a degenerate metric that returns a
// non-finite distance is rejected with std::invalid_argument instead of silently corrupting the
// internal rank sort.

#include <array>
#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "metric/correlation/mgc.hpp"
#include "metric/metric/catalog.hpp"
#include "metric/stats/correlate/correlation.hpp"

namespace {

using Rec = std::vector<double>;
using Eucl = mtrc::Euclidean<double>;

auto in_unit_range(double value) -> bool
{
	return std::isfinite(value) && value <= 1.0 + 1e-9 && value >= -1.0 - 1e-9;
}

// A degenerate "metric" returning NaN for one specific pair; used only to confirm MGC rejects
// non-finite distances. A real metric must return finite distances.
struct NaNAtFirstPair {
	auto operator()(const Rec &lhs, const Rec &rhs) const -> double
	{
		if (lhs == rhs) {
			return 0.0;
		}
		if ((lhs[0] == 0.0 && rhs[0] == 1.0) || (lhs[0] == 1.0 && rhs[0] == 0.0)) {
			return std::numeric_limits<double>::quiet_NaN();
		}
		const auto difference = lhs[0] - rhs[0];
		return difference < 0.0 ? -difference : difference;
	}
};

template <typename Callable> auto throws_invalid_argument(Callable &&call) -> bool
{
	try {
		call();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

void dependence_ranking_is_meaningful()
{
	// y depends strongly (monotonically) on x; z is a fixed permutation of x that breaks the
	// pairing. The dependent pair must score much higher than the permuted (≈independent) pair.
	std::vector<Rec> x, y;
	for (int i = 0; i < 16; ++i) {
		x.push_back({static_cast<double>(i)});
		y.push_back({static_cast<double>(i) * 2.0 + 1.0}); // affine -> perfectly dependent
	}
	// A fixed, deterministic permutation of x (no RNG): reverse the interior in blocks so the
	// pairing with x is scrambled but the marginal set of values is identical to x.
	std::vector<Rec> z = {{7}, {3}, {11}, {1}, {14}, {5}, {9}, {0},
						  {12}, {2}, {15}, {6}, {10}, {4}, {13}, {8}};

	const auto dependent = mtrc::stats::correlate::mgc(x, Eucl(), y, Eucl());
	const auto permuted = mtrc::stats::correlate::mgc(x, Eucl(), z, Eucl());

	assert(dependent.algorithm == "mgc");
	assert(in_unit_range(dependent.value) && in_unit_range(permuted.value));
	assert(std::abs(dependent.value - 1.0) < 1e-9);       // affine transform -> identity dependence
	assert(permuted.value < 0.5);                          // broken pairing -> weak dependence
	assert(dependent.value > permuted.value + 0.4);        // ranking is decisive
}

void raw_estimate_and_xcorr_reject_mismatched_pairs()
{
	mtrc::MGC<Rec, Eucl, Rec, Eucl> mgc;
	std::vector<Rec> a, b;
	for (int i = 0; i < 12; ++i) {
		a.push_back({static_cast<double>(i)});
	}
	for (int i = 0; i < 10; ++i) {
		b.push_back({static_cast<double>(i)});
	}

	// Previously NDEBUG-stripped asserts; now defined std::invalid_argument exceptions.
	assert(throws_invalid_argument([&] { (void)mgc.estimate(a, b); }));
	assert(throws_invalid_argument([&] { (void)mgc.xcorr(a, b, 1); }));
}

void non_finite_metric_is_rejected()
{
	std::vector<Rec> records = {{0.0}, {1.0}, {2.0}, {3.0}};
	// The (0,1) pair yields NaN under NaNAtFirstPair, which would make MGC's internal rank sort
	// an invalid comparator; computeDistanceMatrix must reject it up front.
	assert(throws_invalid_argument(
		[&] { (void)mtrc::stats::correlate::mgc(records, NaNAtFirstPair(), records, Eucl()); }));
}

} // namespace

int main()
{
	dependence_ranking_is_meaningful();
	raw_estimate_and_xcorr_reject_mismatched_pairs();
	non_finite_metric_is_rejected();
	return 0;
}
