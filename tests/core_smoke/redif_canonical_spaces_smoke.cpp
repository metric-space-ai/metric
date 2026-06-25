// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>

#include "metric/engine.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		const auto difference = lhs - rhs;
		return difference < 0 ? -difference : difference;
	}
};

auto close_to(double lhs, double rhs, double tolerance = 1e-9) -> bool
{
	return std::abs(lhs - rhs) <= tolerance;
}

auto mass(const std::vector<double> &measure) -> double
{
	return std::accumulate(measure.begin(), measure.end(), 0.0);
}

} // namespace

int main()
{
	auto two_point = mtrc::make_space(std::vector<int>{0, 1}, AbsoluteDistance{});

	mtrc::redif_options options;
	options.neighbors = 1;
	options.iterations = 1;
	options.euler_step = 0.25;
	options.adaptive_geometry = false;

	const auto op = mtrc::redif_operator(two_point, options);
	assert(op.size() == 2);
	assert(op.neighbors == 1);
	assert(close_to(op.local_distances[0][1], 1.0));
	assert(close_to(op.affinity[0][1], 0.5));
	assert(close_to(op.degree[0], 0.5));
	assert(close_to(op.laplacian[0][0], 0.5));
	assert(close_to(op.laplacian[0][1], -0.5));
	assert(close_to(op.transition[0][0], 0.0));
	assert(close_to(op.transition[0][1], 1.0));
	assert(close_to(op.stationary[0], 0.5));
	assert(close_to(op.transition_row_sum(0), 1.0));
	assert(op.diagnostics.reversible);

	const std::vector<std::vector<double>> identity{{1.0, 0.0}, {0.0, 1.0}};
	const auto forward_step = mtrc::redif_forward_noise_step(op, identity, options.euler_step);
	const auto recovered_step = mtrc::redif_inverse_noise_step(op, forward_step, options.euler_step);
	assert(close_to(recovered_step[0][0], 1.0));
	assert(close_to(recovered_step[0][1], 0.0));
	assert(close_to(recovered_step[1][0], 0.0));
	assert(close_to(recovered_step[1][1], 1.0));

	bool rejected_singular_inverse = false;
	try {
		(void)mtrc::redif_inverse_noise_step(op, forward_step, 0.5);
	} catch (const std::runtime_error &) {
		rejected_singular_inverse = true;
	}
	assert(rejected_singular_inverse);

	const auto removal = mtrc::redif_remove_noise(two_point, options);
	assert(removal.operator_name == "redif_remove_noise");
	assert(removal.strategy == "redif_inverse_implicit_euler_fixed");
	assert(removal.size() == 2);
	assert(removal.operator_diagnostics.size() == options.iterations);
	assert(removal.entropy_diagnostics.size() == removal.size());
	assert(close_to(removal.initial_stationary[0], 0.5));
	assert(close_to(mass(removal[0].terminal_measure()), 1.0));
	assert(close_to(removal[0].terminal_measure()[0], 5.0 / 6.0));
	assert(close_to(removal[0].terminal_measure()[1], 1.0 / 6.0));
	assert(close_to(removal[1].terminal_measure()[0], 1.0 / 6.0));
	assert(close_to(removal[1].terminal_measure()[1], 5.0 / 6.0));

	const auto noise = mtrc::redif_add_noise(two_point, options);
	assert(noise.operator_name == "redif_add_noise");
	assert(noise.strategy == "redif_forward_euler_fixed");
	assert(noise.operator_diagnostics.size() == options.iterations);
	assert(noise.entropy_diagnostics.size() == noise.size());
	assert(close_to(noise.initial_stationary[0], 0.5));
	assert(close_to(mass(noise[0].terminal_measure()), 1.0));
	assert(close_to(noise[0].terminal_measure()[0], 0.75));
	assert(close_to(noise[0].terminal_measure()[1], 0.25));
	assert(close_to(noise[1].terminal_measure()[0], 0.25));
	assert(close_to(noise[1].terminal_measure()[1], 0.75));

	const auto entropy = mtrc::entropy_trajectory(noise[0], noise.initial_stationary);
	assert(entropy.shannon_entropy.back() > entropy.shannon_entropy.front());
	assert(entropy.relative_entropy_to_stationary.back() < entropy.relative_entropy_to_stationary.front());
	assert(noise.entropy_diagnostics[0].terminal_shannon_entropy >
		   noise.entropy_diagnostics[0].initial_shannon_entropy);

	bool rejected_unstable_probability_step = false;
	try {
		auto bad = options;
		bad.euler_step = 1.25;
		(void)mtrc::redif_add_noise(two_point, bad);
	} catch (const std::invalid_argument &) {
		rejected_unstable_probability_step = true;
	}
	assert(rejected_unstable_probability_step);

	bool rejected_empty_neighborhood = false;
	try {
		auto bad = options;
		bad.neighbors = 0;
		(void)mtrc::redif_remove_noise(two_point, bad);
	} catch (const std::invalid_argument &) {
		rejected_empty_neighborhood = true;
	}
	assert(rejected_empty_neighborhood);

	return 0;
}
