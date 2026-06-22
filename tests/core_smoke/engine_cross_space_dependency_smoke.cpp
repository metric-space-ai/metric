// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Core-gate smoke test for the Cross-Space Dependency hero application. Pins the
// statistical contract of the permutation-test dependence workflow on small,
// fast fixtures: a coupled pair is detected as dependent (and the naive scalar
// baseline misses it), while decoupled and permuted pairs are not significant.
// Shares the example's implementation in
// examples/engine/cross_space_dependency.hpp so the gate guards the exact code
// the hero ships.

#include <cassert>
#include <cmath>
#include <cstdint>

#include "cross_space_dependency.hpp"

namespace {

constexpr std::size_t kRecords = 24;
constexpr std::size_t kPermutations = 99;
constexpr std::size_t kBaselinePermutations = 499;
constexpr std::uint64_t kSeed = 20240607ULL;
constexpr double kAlpha = 0.05;

} // namespace

int main()
{
	using cross_space::Coupling;

	// --- Coupled: shared latent regime -> significant cross-space dependence ---
	const auto coupled = cross_space::generate_dataset(kRecords, Coupling::coupled, kSeed);
	const auto coupled_report = cross_space::permutation_test(coupled, kPermutations, kSeed);

	// The fast permutation path equals the public compare() statistic.
	assert(std::abs(coupled_report.statistic - cross_space::observed_statistic(coupled)) < 1e-9);

	// Valid sample statistic and add-one p-value bounds.
	assert(std::isfinite(coupled_report.statistic));
	assert(coupled_report.statistic >= -1.0 && coupled_report.statistic <= 1.0);
	assert(coupled_report.record_count == kRecords);
	assert(coupled_report.permutations == kPermutations);
	const double min_p = 1.0 / (1.0 + static_cast<double>(kPermutations));
	assert(coupled_report.p_value >= min_p - 1e-12 && coupled_report.p_value <= 1.0);

	// Strong, significant dependence under METRIC.
	assert(coupled_report.statistic > 0.3);
	assert(coupled_report.p_value <= kAlpha);
	assert(coupled_report.standardized > 2.0);

	// Naive baselines are blind to the structural dependence. Seed-robust claim:
	// native-metric MGC is dramatically stronger than both naive baselines.
	const auto coupled_baseline = cross_space::baseline_report(coupled, kBaselinePermutations, kSeed);
	assert(coupled_report.statistic - coupled_baseline.vectorized_mgc > 0.3);
	assert(coupled_report.statistic - std::abs(coupled_baseline.pearson_r) > 0.5);
	// Seed-specific calibration check (not a universal guarantee): the naive scalar
	// baseline does not reach significance for kSeed.
	assert(std::abs(coupled_baseline.pearson_r) < 0.3);
	assert(coupled_baseline.pearson_p_value > kAlpha);

	// --- Decoupled: independent latents -> dependence effect collapses ---
	const auto decoupled = cross_space::generate_dataset(kRecords, Coupling::decoupled, kSeed);
	const auto decoupled_report = cross_space::permutation_test(decoupled, kPermutations, kSeed);
	assert(coupled_report.statistic - decoupled_report.statistic > 0.5); // seed-robust
	assert(decoupled_report.p_value > kAlpha);                           // seed-specific calibration

	// --- Permuted: coupled data with the pairing shuffled -> null collapse ---
	const auto permuted = cross_space::permute_curves(coupled, kSeed);
	const auto permuted_report = cross_space::permutation_test(permuted, kPermutations, kSeed);
	assert(coupled_report.statistic - permuted_report.statistic > 0.5); // seed-robust
	assert(permuted_report.p_value > kAlpha);                           // seed-specific calibration

	// --- Determinism: identical seeds reproduce identical reports ---
	const auto coupled_again = cross_space::permutation_test(coupled, kPermutations, kSeed);
	assert(coupled_again.statistic == coupled_report.statistic);
	assert(coupled_again.p_value == coupled_report.p_value);

	return 0;
}
