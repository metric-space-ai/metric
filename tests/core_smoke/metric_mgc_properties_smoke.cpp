// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Statistic-policy and degenerate-case coverage for MGC.
//
// MGC is a DEPENDENCE statistic, not a metric: this test pins the documented
// policy (sample statistic in [-1, 1], identity -> 1, constant space -> 0,
// monotone-transform invariance, observed symmetry) and the degenerate-input
// contract of the public stats wrapper (mismatched / too-small inputs are
// rejected). No p-value is produced anywhere; CorrelationResult carries only the
// statistic. The statistic is rank-based and reproducible, so the deterministic
// partial-dependence fixtures below are pinned with a small tolerance.

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "metric/correlation/mgc.hpp"
#include "metric/metric/catalog.hpp"
#include "metric/stats/correlate/correlation.hpp"

namespace {

using Rec = std::vector<double>;
using Eucl = mtrc::Euclidean<double>;

auto close(double actual, double expected, double tolerance) -> bool
{
	return std::abs(actual - expected) < tolerance;
}

auto in_unit_range(double value) -> bool
{
	// Documented MGC range is [-1, 1]; allow a tiny floating-point slack.
	return std::isfinite(value) && value <= 1.0 + 1e-9 && value >= -1.0 - 1e-9;
}

auto ramp(int n, double scale, double offset) -> std::vector<Rec>
{
	std::vector<Rec> out;
	for (int i = 0; i < n; ++i) {
		out.push_back({static_cast<double>(i) * scale + offset});
	}
	return out;
}

} // namespace

int main()
{
	mtrc::MGC<Rec, Eucl, Rec, Eucl> mgc;

	const auto mono10 = ramp(10, 1.0, 0.0);

	// Identity: a space perfectly correlated with itself -> statistic 1.
	const double identity = mgc(mono10, mono10);
	assert(close(identity, 1.0, 1e-9));
	assert(in_unit_range(identity));

	// Monotone-transform invariance: affine, (positive-domain) quadratic, and
	// order-reversing transforms preserve the dependence structure -> 1.
	const auto affine = ramp(10, 2.0, 1.0);
	std::vector<Rec> quadratic, antitone;
	for (int i = 0; i < 10; ++i) {
		quadratic.push_back({static_cast<double>(i) * static_cast<double>(i)});
		antitone.push_back({static_cast<double>(9 - i)});
	}
	assert(close(mgc(mono10, affine), 1.0, 1e-9));
	assert(close(mgc(mono10, quadratic), 1.0, 1e-9));
	assert(close(mgc(mono10, antitone), 1.0, 1e-9));

	// Deterministic partial dependence: monotone trend + bounded fixed perturbation
	// yields a reproducible statistic strictly inside (0, 1), and is (observably)
	// symmetric.
	std::vector<Rec> x, y;
	for (int i = 0; i < 24; ++i) {
		x.push_back({static_cast<double>(i)});
		y.push_back({static_cast<double>(i) + ((i % 5) - 2) * 3.0});
	}
	const double partial_xy = mgc(x, y);
	const double partial_yx = mgc(y, x);
	assert(in_unit_range(partial_xy));
	assert(partial_xy > 0.0 && partial_xy < 1.0);
	assert(close(partial_xy, partial_yx, 1e-9));               // observed symmetry
	assert(close(partial_xy, 0.68339610064249701, 1e-6));      // deterministic regression anchor

	// Constant (zero-distance) space -> no detectable dependence -> 0.
	const std::vector<Rec> constant(10, Rec{7.0});
	const double constant_corr = mgc(mono10, constant);
	assert(close(constant_corr, 0.0, 1e-12));

	// Small but admissible samples stay finite and in range (the significant-region
	// heuristic degenerates for n <= 4 and falls back to the maximal-scale value).
	const auto mono4 = ramp(4, 1.0, 0.0);
	const auto mono5 = ramp(5, 1.0, 0.0);
	assert(close(mgc(mono4, mono4), 1.0, 1e-9));
	assert(close(mgc(mono5, mono5), 1.0, 1e-9));

	// Raw operator() now rejects mismatched paired containers with a defined
	// exception instead of an NDEBUG-stripped assert.
	bool raw_rejected_mismatch = false;
	try {
		(void)mgc(mono10, mono5);
	} catch (const std::invalid_argument &) {
		raw_rejected_mismatch = true;
	}
	assert(raw_rejected_mismatch);

	// Public stats wrapper: mismatched record counts are rejected.
	bool wrapper_rejected_mismatch = false;
	try {
		(void)mtrc::stats::correlate::mgc(mono10, Eucl(), mono5, Eucl());
	} catch (const std::invalid_argument &) {
		wrapper_rejected_mismatch = true;
	}
	assert(wrapper_rejected_mismatch);

	// Public stats wrapper: degenerate sizes (empty, single record) are rejected so
	// the divide-by-zero / size_t-underflow paths in the raw estimator are never hit.
	bool wrapper_rejected_empty = false;
	try {
		const std::vector<Rec> empty;
		(void)mtrc::stats::correlate::mgc(empty, Eucl(), empty, Eucl());
	} catch (const std::invalid_argument &) {
		wrapper_rejected_empty = true;
	}
	assert(wrapper_rejected_empty);

	bool wrapper_rejected_single = false;
	try {
		const std::vector<Rec> single = {{0.0}};
		(void)mtrc::stats::correlate::mgc(single, Eucl(), single, Eucl());
	} catch (const std::invalid_argument &) {
		wrapper_rejected_single = true;
	}
	assert(wrapper_rejected_single);

	// Two paired records is the minimal admissible size and must not throw.
	const auto pair = ramp(2, 1.0, 0.0);
	const auto wrapper_pair = mtrc::stats::correlate::mgc(pair, Eucl(), pair, Eucl());
	assert(wrapper_pair.algorithm == "mgc");
	assert(in_unit_range(wrapper_pair.value));

	std::cout << std::setprecision(17) << "MGC identity=" << identity << " partial=" << partial_xy
			  << " constant=" << constant_corr << "\n";

	return 0;
}
