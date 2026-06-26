// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <cmath>
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

auto close_to(double lhs, double rhs, double tolerance = 1e-7) -> bool
{
	return std::abs(lhs - rhs) <= tolerance;
}

auto maximum_difference(const std::vector<std::vector<double>> &lhs,
						const std::vector<std::vector<double>> &rhs) -> double
{
	double maximum = 0.0;
	for (std::size_t row = 0; row < lhs.size(); ++row) {
		for (std::size_t column = 0; column < lhs[row].size(); ++column) {
			maximum = std::max(maximum, std::abs(lhs[row][column] - rhs[row][column]));
		}
	}
	return maximum;
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

	const std::vector<std::vector<double>> identity{{1.0, 0.0}, {0.0, 1.0}};
	const auto forward = mtrc::redif_forward_noise_step(op, identity, 0.25);
	mtrc::RedifStabilityDiagnostics stable;
	const auto recovered = mtrc::redif_inverse_noise_step(op, forward, 0.25, &stable);
	assert(stable.status == "stable");
	assert(stable.stable);
	assert(!stable.marginal);
	assert(stable.singularity_margin > options.marginal_stability_tolerance);
	assert(close_to(recovered[0][0], 1.0));
	assert(close_to(recovered[1][1], 1.0));

	const auto near_singular_forward = mtrc::redif_forward_noise_step(op, identity, 0.499);
	mtrc::RedifStabilityDiagnostics marginal;
	const auto near_recovered = mtrc::redif_inverse_noise_step(op, near_singular_forward, 0.499, &marginal,
															  1.0e-12, 1.0e-2);
	assert(marginal.status == "marginal");
	assert(marginal.stable);
	assert(marginal.marginal);
	assert(marginal.singularity_margin > 1.0e-12);
	assert(marginal.singularity_margin <= 1.0e-2);
	assert(close_to(near_recovered[0][0], 1.0));
	assert(close_to(near_recovered[1][1], 1.0));

	mtrc::RedifStabilityDiagnostics singular;
	bool rejected = false;
	try {
		(void)mtrc::redif_inverse_noise_step(op, forward, 0.5, &singular);
	} catch (const std::runtime_error &) {
		rejected = true;
	}
	assert(rejected);
	assert(singular.status == "singular");
	assert(!singular.stable);
	assert(singular.singularity_margin <= 1.0e-12);

	const auto removal = mtrc::redif_remove_noise(two_point, options);
	assert(removal.step_diagnostics.size() == options.iterations);
	assert(removal.step_diagnostics[0].stability.status == "stable");
	assert(removal.step_diagnostics[0].stability.stable);
	assert(removal.step_diagnostics[0].operator_diagnostics.reversible);

	const auto full_small = mtrc::redif_forward_noise_step(op, identity, 0.10);
	const auto half_small =
		mtrc::redif_forward_noise_step(op, mtrc::redif_forward_noise_step(op, identity, 0.05), 0.05);
	const auto full_larger = mtrc::redif_forward_noise_step(op, identity, 0.40);
	const auto half_larger =
		mtrc::redif_forward_noise_step(op, mtrc::redif_forward_noise_step(op, identity, 0.20), 0.20);
	assert(maximum_difference(full_small, half_small) < maximum_difference(full_larger, half_larger));

	return 0;
}
