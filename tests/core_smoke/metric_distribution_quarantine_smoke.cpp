// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Smoke coverage for distribution-family metric admission.
//
// `mtrc::RandomEMD<Sample,D>`, `mtrc::CramervonMises<Sample,D>` and
// `mtrc::KolmogorovSmirnov<Sample,D>` are CDF-distance candidates built on the
// shared `mtrc::PMQ` CDF. None of them is admitted as a true metric. This file
// audits the EXACT shipped computation and pins the concrete reasons each is
// quarantined, so any future "fix" is forced to consciously re-classify it.
//
// Exact record model (audited):
//   Each metric constructs `mtrc::PMQ` from each `Sample` (a `std::vector<float>`
//   of raw observations). `PMQ::cdf(x)` is NOT the empirical step CDF: it is the
//   Akima spline interpolation through the knots `(sorted_sample[i], (i+0.5)/n)`,
//   clamped to 0 below the min sample and 1 above the max sample. The CDF is a
//   deterministic function of the sample (the PMQ RNG is only used by `rnd()`),
//   so symmetry and self-identity are exact. But:
//     * RandomEMD      = Riemann sum of |F1-F2| over the PAIR-dependent range
//                        [min, max] with step (max-min)*precision  (~L1-CDF).
//     * CramervonMises = sqrt of the Riemann sum of (F1-F2)^2        (~root-L2-CDF;
//                        the ROOT form, not the squared statistic).
//     * KolmogorovSmirnov = max |F1-F2| evaluated ONLY at the concatenated data
//                        points, not the true supremum                (~Linf-CDF).
//
// Disqualifying defects proven below (all reproduced by /tmp probes during the
// C8 audit and pinned here as regression guards):
//   * ROOT CAUSE: PMQ's Akima "CDF" is neither monotone nor bounded to [0,1] in
//     the interior (observed range [-252, +2.79] depending on the sample). A
//     function that escapes [0,1] is not a CDF, so the L1/L2/sup "distances"
//     built on it are not metrics -- this is what breaks the triangle inequality
//     for RandomEMD and CvM and makes KS exceed 1.0.
//   * The Akima CDF divides by (x[i]-x[i-1]); tied sample values make that zero,
//     so RandomEMD/CvM return NaN (even for d(a,a)) and KS silently returns 0
//     (false identity, including for fully disjoint distributions).
//   * Zero-range samples (all values equal) make RandomEMD/CvM step==0 and the
//     integration loop never terminates; empty samples SIGSEGV all three. NOT
//     executed here -- see check_unsafe_inputs_are_documented().
//   * KS additionally evaluates a finite-point supremum on a pair-dependent grid,
//     not the true supremum.

#include <array>
#include <cmath>
#include <cstdio>
#include <vector>

#include "metric/metric/admission.hpp"
#include "metric/metric/catalog.hpp"
#include "metric/utils/poor_mans_quantum.hpp"

namespace {

int g_failures = 0;

#define CHECK(cond)                                                                                                    \
	do {                                                                                                               \
		if (!(cond)) {                                                                                                 \
			std::fprintf(stderr, "CHECK failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__);                             \
			++g_failures;                                                                                              \
		}                                                                                                              \
	} while (0)

using Sample = std::vector<float>;

// Clean admitted-shape inputs: strictly increasing (no ties), nonzero range,
// at least three observations. On this domain the shipped CDF is finite.
const std::vector<Sample> &clean_fixtures()
{
	static const std::vector<Sample> fixtures = {
		{0.0f, 1.0f, 2.0f, 3.0f},		 {0.0f, 1.0f, 2.0f, 10.0f}, {1.0f, 2.0f, 3.0f, 100.0f},
		{0.0f, 2.0f, 4.0f, 6.0f, 8.0f}, {0.5f, 1.5f, 2.5f, 3.5f},
	};
	return fixtures;
}

// Self-identity, symmetry, non-negativity and finiteness hold EXACTLY for all
// three on the clean domain (the CDF is a deterministic function of the sample
// and every formula is symmetric in its two arguments). The triangle inequality
// is deliberately NOT asserted in this loop: it fails for RandomEMD and KS (see
// the dedicated checks below).
template <typename Metric> void check_shared_positive_properties(const char *name, Metric metric)
{
	const double tol = 1e-6;
	const auto &fixtures = clean_fixtures();
	for (std::size_t i = 0; i < fixtures.size(); ++i) {
		const double daa = metric(fixtures[i], fixtures[i]);
		CHECK(std::isfinite(daa));
		CHECK(std::abs(daa) <= tol); // identity: d(a,a) == 0
		if (std::abs(daa) > tol) {
			std::fprintf(stderr, "  [%s] d(a,a) = %.8g\n", name, daa);
		}
		for (std::size_t j = 0; j < fixtures.size(); ++j) {
			const double dij = metric(fixtures[i], fixtures[j]);
			const double dji = metric(fixtures[j], fixtures[i]);
			CHECK(std::isfinite(dij));					   // finite on the clean domain
			CHECK(dij >= -tol);							   // non-negativity
			CHECK(std::abs(dij - dji) <= tol * (1.0 + std::abs(dij))); // symmetry
		}
	}
}

// RandomEMD: a deterministic NaN on tied sample values. A true metric must
// return a finite distance on its admitted record domain; real samples have
// ties, so this is a blocker, not a corner case. (The triangle inequality also
// fails, but only by ~1e-3 relative margins from domain truncation + Akima, too
// tight to pin portably; the NaN below is the decisive disqualifier.)
void check_random_emd_is_not_admitted()
{
	mtrc::RandomEMD<Sample, double> emd;

	// Finite and symmetric on clean inputs.
	const double clean = emd(Sample{0.0f, 1.0f, 2.0f, 3.0f}, Sample{0.0f, 1.0f, 2.0f, 10.0f});
	CHECK(std::isfinite(clean));
	CHECK(clean > 0.0);

	// Tied values -> Akima divides by zero -> NaN (not finite). This also breaks
	// self-identity for any sample with repeats: d(a,a) is NaN, not 0.
	const Sample tied_a = {1.0f, 1.0f, 2.0f, 3.0f};
	const Sample tied_b = {1.0f, 2.0f, 2.0f, 3.0f};
	CHECK(!std::isfinite(emd(tied_a, tied_b)));
	CHECK(!std::isfinite(emd(tied_a, tied_a)));
}

// CramervonMises: the shipped computation takes the sqrt, i.e. it is the
// ROOT-CDF L2 *form* (the admissible candidate, NOT the squared CvM statistic),
// which is why the family stays "metric variant required". But the shipped
// computation is not admissible as a metric: it shares the same Akima CDF, which
// (1) returns NaN on tied values -- breaking even self-identity for real samples
// with repeats -- and (2) overshoots [0,1], so the triangle inequality also
// fails once near-duplicate knots appear (adversarially reproduced, e.g.
// d(a,c)=35.97 > d(a,b)+d(b,c)=35.84 for
// a={-96.38,-59.44,33.3413,33.3421,34.97}, b={-65.70,-39.33,18.61,26.43,84.12},
// c={-61.87,-1.59,17.87} -- relative margin too small to pin portably; the
// root-cause overshoot is pinned directly in check_cdf_engine_overshoot()).
void check_cramer_von_mises_root_form_but_not_admitted()
{
	mtrc::CramervonMises<Sample, double> cvm;

	// Finite on clean distinct inputs.
	CHECK(std::isfinite(cvm(Sample{0.0f, 1.0f, 2.0f, 3.0f}, Sample{0.0f, 1.0f, 2.0f, 10.0f})));

	// Tied values -> NaN, including self-distance (identity itself fails for any
	// sample with repeats), so the shipped computation is not admissible.
	const Sample tied_a = {1.0f, 1.0f, 2.0f, 3.0f};
	const Sample tied_b = {1.0f, 2.0f, 2.0f, 3.0f};
	CHECK(!std::isfinite(cvm(tied_a, tied_b)));
	CHECK(!std::isfinite(cvm(tied_a, tied_a))); // d(a,a) is NaN, not 0
}

// Root cause shared by all three: PMQ's "CDF" is an Akima cubic through the
// (sorted value, plotting-position) knots, which is neither monotone nor bounded
// to [0,1] in the interior. A function that escapes [0,1] is not a CDF, so the
// L1/L2/sup "distances" built on it cannot be metrics. This pins the invariant
// violation directly (large, deterministic margin), which is the mechanism
// behind the RandomEMD and KS triangle violations and the KS values > 1.
void check_cdf_engine_overshoot()
{
	const Sample sample = {0.0f, 1.0f, 1.1f, 1.2f, 10.0f, 20.0f};
	mtrc::PMQ<mtrc::Discrete<float>, float> pmq(sample);
	// Strictly inside [sample.front(), sample.back()] the interpolant dips well
	// below 0 (observed cdf(0.51) ~ -0.1147).
	const float c = pmq.cdf(0.51f);
	CHECK(std::isfinite(c));
	CHECK(c < -0.05f); // a valid CDF would be in [0, 1]
}

// KolmogorovSmirnov: NOT a metric as shipped.
//   (1) Triangle inequality is violated by a wide margin on a small fixture.
//   (2) The reported distance exceeds 1.0, which is impossible for a true
//       sup-CDF distance -- proof the Akima CDF overshoots [0,1].
//   (3) Tied values silently collapse two distinct distributions to distance 0,
//       breaking identity of indiscernibles.
void check_kolmogorov_smirnov_is_not_admitted()
{
	mtrc::KolmogorovSmirnov<Sample, double> ks;

	// (1)+(2) Triangle violation with d(a,c) > 1.
	const Sample a = {0.0f, 1.0f, 2.0f, 10.0f};
	const Sample b = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f};
	const Sample c = {1.0f, 2.0f, 3.0f, 100.0f};
	const double dab = ks(a, b);
	const double dbc = ks(b, c);
	const double dac = ks(a, c);
	CHECK(std::isfinite(dab) && std::isfinite(dbc) && std::isfinite(dac));
	CHECK(dac > dab + dbc + 0.5); // triangle violated (observed: 1.317 > 0.136 + 0.175)
	CHECK(dac > 1.0);			  // overshoot: a true sup-CDF distance lies in [0, 1]

	// (3) False zero on tied values: distinct distributions, distance 0. Unlike
	// RandomEMD/CvM, KS does not integrate the interior, so the per-knot NaN is
	// swallowed by the `NaN > max` comparison and the result collapses to 0.
	const Sample tied_a = {1.0f, 1.0f, 2.0f, 3.0f};
	const Sample tied_b = {1.0f, 2.0f, 2.0f, 3.0f};
	CHECK(ks(tied_a, tied_b) == 0.0);
	// Worse: two FULLY DISJOINT all-equal distributions also collapse to 0, even
	// though the true KS distance is 1 (the CDFs are 0/1-separated everywhere).
	CHECK(ks(Sample{2.0f, 2.0f, 2.0f}, Sample{5.0f, 5.0f, 5.0f}) == 0.0);
	CHECK(ks(Sample{1.0f, 1.0f, 1.0f, 1.0f}, Sample{2.0f, 2.0f, 2.0f, 2.0f}) == 0.0);
	// Control: a distinct-valued pair is correctly nonzero, so the zeros above
	// are false identities, not real coincidences.
	CHECK(ks(Sample{0.0f, 1.0f, 2.0f, 3.0f}, Sample{1.0f, 2.0f, 3.0f, 4.0f}) > 0.0);
}

// Two further hazards are real but cannot be exercised inside a single
// in-process smoke binary (one hangs, one crashes), so they are documented here:
//
//   * Zero-range / all-equal samples: RandomEMD/CvM compute
//       step = (max - min) * precision  == 0,
//     then loop  for (value = min; value <= max; value += step)  which never
//     advances. Reproduced out-of-process with
//       perl -e 'alarm 4; exec @ARGV' ./probe   (exit 142 == SIGALRM == hung).
//   * Empty samples: all three index concat_data.front()/back() (and build PMQ
//     on empty data) out of bounds -> SIGSEGV (audit reproduced exit 139 for
//     emd({},{1,2,3}), emd({},{}), cvm({},{}), ks({},{1,2,3})).
//
// Any future admission must add zero-range and empty-input guards.
void check_unsafe_inputs_are_documented() { CHECK(true); }

} // namespace

// Trait classification: none of the three is an admitted true metric. They keep
// the default `metric_law::distance` (no promoting specialization exists), so
// metric-only routing must not pick them up. If anyone promotes one of these,
// these assertions break and force a deliberate re-classification.
static_assert(mtrc::metric_traits<mtrc::RandomEMD<Sample, double>>::law == mtrc::metric_law::distance);
static_assert(mtrc::metric_traits<mtrc::CramervonMises<Sample, double>>::law == mtrc::metric_law::distance);
static_assert(mtrc::metric_traits<mtrc::KolmogorovSmirnov<Sample, double>>::law == mtrc::metric_law::distance);

static_assert(!mtrc::metric::is_admitted_metric_v<mtrc::RandomEMD<Sample, double>>);
static_assert(!mtrc::metric::is_admitted_metric_v<mtrc::CramervonMises<Sample, double>>);
static_assert(!mtrc::metric::is_admitted_metric_v<mtrc::KolmogorovSmirnov<Sample, double>>);

static_assert(mtrc::metric::is_quarantined_metric_v<mtrc::RandomEMD<Sample, double>>);
static_assert(mtrc::metric::is_quarantined_metric_v<mtrc::CramervonMises<Sample, double>>);
static_assert(mtrc::metric::is_quarantined_metric_v<mtrc::KolmogorovSmirnov<Sample, double>>);

// The admitted strict transport metric remains the promotion target for the
// distribution-transport intent that RandomEMD only approximates.
static_assert(mtrc::metric::is_admitted_metric_v<mtrc::Wasserstein<double>>);

int main()
{
	check_shared_positive_properties("RandomEMD", mtrc::RandomEMD<Sample, double>{});
	check_shared_positive_properties("CramervonMises", mtrc::CramervonMises<Sample, double>{});
	check_shared_positive_properties("KolmogorovSmirnov", mtrc::KolmogorovSmirnov<Sample, double>{});

	check_cdf_engine_overshoot();
	check_random_emd_is_not_admitted();
	check_cramer_von_mises_root_form_but_not_admitted();
	check_kolmogorov_smirnov_is_not_admitted();
	check_unsafe_inputs_are_documented();

	if (g_failures != 0) {
		std::fprintf(stderr, "metric_distribution_quarantine_smoke: %d check(s) failed\n", g_failures);
		return 1;
	}
	return 0;
}
