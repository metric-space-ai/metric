// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Deterministic fixture for the Finite Metric Dynamics hero example.
//
// The fixture is a 1-D manifold (a multi-cycle S-curve with deterministic
// micro-variation) sampled at evenly spaced arc parameters and embedded in R^2.
// Two properties make it a good substrate for demonstrating dynamics over
// metric structure:
//
//   * It is a genuinely low-dimensional structure living in a higher-dimensional
//     space, so a k-NN graph built from the Euclidean metric recovers the curve
//     as a near-path graph -- the "metric structure" the dynamics evolve over.
//   * The clean coordinates are smooth along the manifold (low graph Dirichlet
//     energy), so injected noise is high graph-frequency and the reverse graph
//     heat flow can contract it back onto the curve.
//
// The generator is closed-form and uses no randomness, so the fixture is fixed
// bit-for-bit across runs and platforms.

#ifndef METRIC_EXAMPLE_FINITE_METRIC_DYNAMICS_FIXTURE_HPP
#define METRIC_EXAMPLE_FINITE_METRIC_DYNAMICS_FIXTURE_HPP

#include <cmath>
#include <cstddef>
#include <vector>

namespace finite_metric_dynamics_fixture {

/// Number of nodes (records) in the console fixture.
inline constexpr std::size_t node_count = 28;

/// Number of nodes (records) in the scaled visual evidence fixture.
inline constexpr std::size_t visual_node_count = 512;

/// Dimension of each record (the manifold is embedded in R^2).
inline constexpr std::size_t dimension = 2;

/// Build the clean fixture. The default `node_count` path preserves the
/// original single-cycle console example; the scaled visual path uses a
/// deterministic multi-cycle S-curve. The x-axis is scaled so neighbouring
/// samples sit at a comparable Euclidean distance to the vertical excursions.
inline auto make_records(std::size_t count = node_count) -> std::vector<std::vector<double>>
{
	constexpr double pi = 3.14159265358979323846;
	const double cycles = count == node_count ? 1.0 : 4.0;
	const double x_scale = 1.0 / (cycles * pi); // compress x so dx ~ dy in magnitude

	std::vector<std::vector<double>> records;
	records.reserve(count);
	for (std::size_t i = 0; i < count; ++i) {
		const double u = count > 1 ? static_cast<double>(i) / static_cast<double>(count - 1) : 0.0;
		const double t = (2.0 * pi * cycles) * u;
		const double deterministic_variation =
			count == node_count ? 0.0 : 0.055 * std::sin(7.0 * t + 0.35) + 0.025 * std::cos(13.0 * t);
		records.push_back({t * x_scale, std::sin(t) + deterministic_variation});
	}
	return records;
}

inline auto make_visual_records() -> std::vector<std::vector<double>> { return make_records(visual_node_count); }

} // namespace finite_metric_dynamics_fixture

#endif
