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
#include "metric/core/errors.hpp"
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

struct CountingMetric {
	std::size_t *calls{};

	auto operator()(const Rec &lhs, const Rec &rhs) const -> double
	{
		++(*calls);
		return std::abs(lhs[0] - rhs[0]);
	}
};

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

	// Legacy raw MGC is still available, but exact dense work now has a safe default
	// and explicit budgets. Refusal must happen before distance matrices call the metric.
	const auto triple = ramp(3, 1.0, 0.0);
	std::size_t calls = 0;
	CountingMetric counting{&calls};
	mtrc::MGC<Rec, CountingMetric, Rec, CountingMetric> low_record_budget(
		counting, counting, mtrc::mgc_resource_options{2, 0, 0});
	bool raw_refused_by_record_budget = false;
	try {
		(void)low_record_budget(triple, triple);
	} catch (const mtrc::RepresentationError &) {
		raw_refused_by_record_budget = true;
	}
	assert(raw_refused_by_record_budget);
	assert(calls == 0);

	mtrc::MGC<Rec, CountingMetric, Rec, CountingMetric> low_distance_budget(
		counting, counting, mtrc::mgc_resource_options{0, 2, 0});
	bool raw_refused_by_distance_budget = false;
	try {
		(void)low_distance_budget(triple, triple);
	} catch (const mtrc::RepresentationError &) {
		raw_refused_by_distance_budget = true;
	}
	assert(raw_refused_by_distance_budget);
	assert(calls == 0);

	const auto scratch_large = ramp(2887, 1.0, 0.0);
	bool wrapper_refused_by_scratch_budget = false;
	try {
		(void)mtrc::stats::correlate::mgc(scratch_large, counting, scratch_large, counting);
	} catch (const mtrc::RepresentationError &) {
		wrapper_refused_by_scratch_budget = true;
	}
	assert(wrapper_refused_by_scratch_budget);
	assert(calls == 0);

	mtrc::DistanceMatrix<double> direct_a(3);
	mtrc::DistanceMatrix<double> direct_b(3);
	for (std::size_t i = 0; i < 3; ++i) {
		direct_a(i, i) = 0.0;
		direct_b(i, i) = 0.0;
		for (std::size_t j = i + 1; j < 3; ++j) {
			direct_a(i, j) = static_cast<double>(j - i);
			direct_b(i, j) = static_cast<double>((j - i) * 2);
		}
	}
	bool direct_refused_by_matrix_budget = false;
	try {
		(void)mtrc::MGC_direct(mtrc::mgc_resource_options{0, 0, 8})(direct_a, direct_b);
	} catch (const mtrc::RepresentationError &) {
		direct_refused_by_matrix_budget = true;
	}
	assert(direct_refused_by_matrix_budget);

	bool direct_helper_refused_by_matrix_budget = false;
	try {
		(void)mtrc::MGC_direct(mtrc::mgc_resource_options{0, 0, 8}).center_distance_matrix(direct_a);
	} catch (const mtrc::RepresentationError &) {
		direct_helper_refused_by_matrix_budget = true;
	}
	assert(direct_helper_refused_by_matrix_budget);

	bool xcorr_rejected_overshift = false;
	try {
		(void)mgc.xcorr(triple, triple, 3);
	} catch (const std::invalid_argument &) {
		xcorr_rejected_overshift = true;
	}
	assert(xcorr_rejected_overshift);

	bool free_distance_matrix_refused_default_large = false;
	try {
		const auto too_large_for_default = ramp(4097, 1.0, 0.0);
		(void)mtrc::distance_matrix(too_large_for_default);
	} catch (const mtrc::RepresentationError &) {
		free_distance_matrix_refused_default_large = true;
	}
	assert(free_distance_matrix_refused_default_large);

	std::cout << std::setprecision(17) << "MGC identity=" << identity << " partial=" << partial_xy
			  << " constant=" << constant_corr << "\n";

	return 0;
}
