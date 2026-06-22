// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Cross-Space Dependency — production hero application.
//
// Demonstrates that METRIC can test dependence between two heterogeneous finite
// metric spaces (event-log strings under edit distance vs process curves under
// Time-Warp Edit Distance) WITHOUT a shared embedding, and that a permutation
// test built on the public `compare()` intent turns MGC's bare statistic into a
// calibrated p-value. See examples/engine/cross_space_dependency.hpp for the
// statistical contract and docs/examples/cross-space-dependency.md for the
// interpretation.

#include <cassert>
#include <iomanip>
#include <iostream>
#include <string>

#include "cross_space_dependency.hpp"

namespace {

using cross_space::BaselineReport;
using cross_space::Coupling;
using cross_space::Dataset;
using cross_space::DependenceReport;

constexpr std::size_t kRecords = 48;
constexpr std::size_t kPermutations = 199; // MGC null draws
constexpr std::size_t kBaselinePermutations = 1999; // cheap scalar null draws
constexpr std::uint64_t kSeed = 20240607ULL;
constexpr double kAlpha = 0.05; // significance level for the dependence decision

auto print_metric_block(const std::string &label, const DependenceReport &report) -> void
{
	std::cout << label << " MGC statistic = " << report.statistic << "\n";
	std::cout << label << " MGC null mean = " << report.null_mean << "\n";
	std::cout << label << " MGC null sd = " << report.null_sd << "\n";
	std::cout << label << " MGC standardized effect = " << report.standardized << "\n";
	std::cout << label << " MGC permutation p-value = " << report.p_value << "\n";
	std::cout << label << " MGC permutations = " << report.permutations << "\n";
	std::cout << label << " MGC decision (alpha=" << kAlpha
			  << ") = " << (report.p_value <= kAlpha ? "dependent" : "independent") << "\n";
}

auto print_baseline_block(const std::string &label, const BaselineReport &report) -> void
{
	std::cout << label << " baseline scalar pearson r = " << report.pearson_r << "\n";
	std::cout << label << " baseline scalar pearson p-value = " << report.pearson_p_value << "\n";
	std::cout << label << " baseline scalar decision (alpha=" << kAlpha
			  << ") = " << (report.pearson_p_value <= kAlpha ? "dependent" : "independent") << "\n";
	std::cout << label << " baseline forced-vector MGC = " << report.vectorized_mgc << "\n";
}

} // namespace

int main()
{
	std::cout << std::fixed << std::setprecision(6);

	// ---------------------------------------------------------------------
	// Scenario 1 — COUPLED: logs and curves share one hidden regime.
	// METRIC must report strong, significant dependence; the naive scalar
	// baseline must miss it.
	// ---------------------------------------------------------------------
	const Dataset coupled = cross_space::generate_dataset(kRecords, Coupling::coupled, kSeed);
	const DependenceReport coupled_mgc = cross_space::permutation_test(coupled, kPermutations, kSeed);
	const BaselineReport coupled_baseline = cross_space::baseline_report(coupled, kBaselinePermutations, kSeed);

	// The permutation null reuses precomputed distance matrices for speed; verify
	// it reports the exact statistic the public `compare()` intent produces.
	const double coupled_compare_value = cross_space::observed_statistic(coupled);

	// ---------------------------------------------------------------------
	// Scenario 2 — DECOUPLED: curves driven by an independent regime.
	// METRIC must report no significant dependence.
	// ---------------------------------------------------------------------
	const Dataset decoupled = cross_space::generate_dataset(kRecords, Coupling::decoupled, kSeed);
	const DependenceReport decoupled_mgc = cross_space::permutation_test(decoupled, kPermutations, kSeed);
	const BaselineReport decoupled_baseline = cross_space::baseline_report(decoupled, kBaselinePermutations, kSeed);

	// ---------------------------------------------------------------------
	// Scenario 3 — PERMUTED: take the coupled data, shuffle the pairing.
	// This is one explicit draw from the independence null; dependence must
	// collapse to the null range.
	// ---------------------------------------------------------------------
	const Dataset permuted = cross_space::permute_curves(coupled, kSeed);
	const DependenceReport permuted_mgc = cross_space::permutation_test(permuted, kPermutations, kSeed);

	std::cout << "cross-space records = " << kRecords << "\n";
	std::cout << "cross-space left space = event_logs/edit_distance\n";
	std::cout << "cross-space right space = process_curves/twed\n";
	std::cout << "coupled compare() statistic = " << coupled_compare_value << "\n";
	print_metric_block("coupled", coupled_mgc);
	print_baseline_block("coupled", coupled_baseline);
	print_metric_block("decoupled", decoupled_mgc);
	print_baseline_block("decoupled", decoupled_baseline);
	print_metric_block("permuted", permuted_mgc);
	std::cout << std::flush;

	// ---------------------------------------------------------------------
	// Assertions — the evidence contract for this hero application.
	// ---------------------------------------------------------------------

	// MGC statistic is always a valid sample statistic in [-1, 1].
	for (const auto *report : {&coupled_mgc, &decoupled_mgc, &permuted_mgc}) {
		assert(std::isfinite(report->statistic));
		assert(report->statistic >= -1.0 && report->statistic <= 1.0);
		assert(report->record_count == kRecords);
		assert(report->permutations == kPermutations);
		// Add-one estimator bounds: p in (0, 1].
		assert(report->p_value > 0.0 && report->p_value <= 1.0);
		const double min_p = 1.0 / (1.0 + static_cast<double>(kPermutations));
		assert(report->p_value >= min_p - 1e-12);
	}

	// The fast permutation path reports exactly the public `compare()` statistic.
	assert(std::abs(coupled_mgc.statistic - coupled_compare_value) < 1e-9);

	// Coupled: strong, significant dependence detected by METRIC.
	assert(coupled_mgc.statistic > 0.3);
	assert(coupled_mgc.p_value <= kAlpha);
	assert(coupled_mgc.statistic > coupled_mgc.null_mean);
	assert(coupled_mgc.standardized > 2.0);

	// Seed-robust invariant: the native-metric MGC is dramatically stronger than
	// both naive baselines. The dependence is unlocked by the metric spaces (not
	// the test alone) and is invisible to a naive scalar reduction. These large
	// effect-size gaps hold across seeds, not just this one.
	assert(coupled_mgc.statistic - coupled_baseline.vectorized_mgc > 0.3);
	assert(coupled_mgc.statistic - std::abs(coupled_baseline.pearson_r) > 0.5);

	// Significance readout for this fixed seed: the naive scalar baseline does not
	// reach significance — its structural blindness. NOTE: "baseline not
	// significant" is a stochastic outcome (a calibrated alpha test still crosses
	// on a few % of seeds), so the two asserts below are calibration checks pinned
	// to kSeed, not universal guarantees; the robust claim is the gap asserted above.
	assert(std::abs(coupled_baseline.pearson_r) < 0.3);
	assert(coupled_baseline.pearson_p_value > kAlpha);

	// Seed-robust invariant: removing the shared latent (decoupled) or destroying
	// the pairing (permuted) collapses the dependence effect far below the coupled
	// statistic — the pairing, not the marginals, carried the signal.
	assert(coupled_mgc.statistic - decoupled_mgc.statistic > 0.5);
	assert(coupled_mgc.statistic - permuted_mgc.statistic > 0.5);

	// Significance readout for this fixed seed: neither null scenario is
	// significant at alpha. As above, "not significant" is a seed-specific
	// calibration check (correct ~95% null behaviour), not a universal guarantee.
	assert(decoupled_mgc.p_value > kAlpha);
	assert(permuted_mgc.p_value > kAlpha);

	// Determinism: identical seeds reproduce identical reports.
	const DependenceReport coupled_again = cross_space::permutation_test(coupled, kPermutations, kSeed);
	assert(coupled_again.statistic == coupled_mgc.statistic);
	assert(coupled_again.p_value == coupled_mgc.p_value);

	std::cout << "cross-space verdict = metric_detects_dependence_baseline_misses_it\n";
	return 0;
}
