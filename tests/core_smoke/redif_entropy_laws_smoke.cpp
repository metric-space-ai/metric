// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <cmath>
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

} // namespace

int main()
{
	auto two_point = mtrc::make_space(std::vector<int>{0, 1}, AbsoluteDistance{});
	mtrc::redif_options uniform_options;
	uniform_options.neighbors = 1;
	uniform_options.iterations = 3;
	uniform_options.euler_step = 0.25;
	uniform_options.adaptive_geometry = false;
	const auto uniform_noise = mtrc::redif_add_noise(two_point, uniform_options);
	assert(close_to(uniform_noise.initial_stationary[0], 0.5));
	assert(close_to(uniform_noise.initial_stationary[1], 0.5));
	const auto uniform_entropy = mtrc::entropy_trajectory(uniform_noise[0], uniform_noise.initial_stationary);
	assert(uniform_entropy.shannon_entropy.back() > uniform_entropy.shannon_entropy.front());
	assert(uniform_entropy.relative_entropy_to_stationary.back() <
		   uniform_entropy.relative_entropy_to_stationary.front());

	auto nonuniform_space = mtrc::make_space(std::vector<int>{0, 1, 10}, AbsoluteDistance{});
	mtrc::redif_options nonuniform_options;
	nonuniform_options.neighbors = 2;
	nonuniform_options.iterations = 2;
	nonuniform_options.euler_step = 0.25;
	nonuniform_options.adaptive_geometry = false;
	const auto nonuniform_noise = mtrc::redif_add_noise(nonuniform_space, nonuniform_options);
	assert(nonuniform_noise.initial_stationary.size() == 3);
	assert(!close_to(nonuniform_noise.initial_stationary[0], nonuniform_noise.initial_stationary[1]) ||
		   !close_to(nonuniform_noise.initial_stationary[1], nonuniform_noise.initial_stationary[2]));
	for (const auto &path : nonuniform_noise.paths) {
		const auto trajectory = mtrc::entropy_trajectory(path, nonuniform_noise.initial_stationary);
		assert(trajectory.relative_entropy_to_stationary.back() <=
			   trajectory.relative_entropy_to_stationary.front() + 1.0e-12);
	}
	assert(nonuniform_noise.step_diagnostics.size() == nonuniform_options.iterations);
	for (const auto &step : nonuniform_noise.step_diagnostics) {
		assert(step.stationary.size() == nonuniform_space.size());
		assert(step.maximum_relative_entropy_to_current_stationary >= 0.0);
	}

	return 0;
}
