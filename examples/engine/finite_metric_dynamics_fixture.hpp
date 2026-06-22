// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Small, deterministic fixture for the Finite Metric Dynamics hero example.
//
// The fixture is a 1-D manifold (an S-curve, y = sin x) sampled at evenly spaced
// arc parameters and embedded in R^2. Two properties make it a good substrate
// for demonstrating dynamics over metric structure:
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

/// Number of nodes (records) in the fixture.
inline constexpr std::size_t node_count = 28;

/// Dimension of each record (the manifold is embedded in R^2).
inline constexpr std::size_t dimension = 2;

/// Build the clean fixture: `node_count` points along y = sin(x), x in [0, 2*pi].
/// The x-axis is scaled so neighbouring samples sit at a comparable Euclidean
/// distance to the vertical excursions, giving a well-conditioned path graph.
inline auto make_records() -> std::vector<std::vector<double>>
{
	constexpr double pi = 3.14159265358979323846;
	constexpr double x_scale = 1.0 / pi; // compress x so dx ~ dy in magnitude

	std::vector<std::vector<double>> records;
	records.reserve(node_count);
	for (std::size_t i = 0; i < node_count; ++i) {
		const double t = (2.0 * pi) * static_cast<double>(i) / static_cast<double>(node_count - 1);
		records.push_back({t * x_scale, std::sin(t)});
	}
	return records;
}

} // namespace finite_metric_dynamics_fixture

#endif
