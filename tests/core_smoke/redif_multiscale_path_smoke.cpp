// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
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

auto score_by_id(const mtrc::RedifMultiscaleResult &result, std::size_t id_index) -> const mtrc::RedifMultiscaleScore &
{
	for (const auto &score : result.scores) {
		if (score.id.index() == id_index) {
			return score;
		}
	}
	throw std::logic_error("missing Redif multiscale score id");
}

} // namespace

int main()
try
{
	auto line = mtrc::make_space(std::vector<int>{0, 1, 2, 100}, AbsoluteDistance{});

	mtrc::redif_options local;
	local.neighbors = 1;
	local.iterations = 3;
	local.euler_step = 0.25;
	local.adaptive_geometry = false;

	mtrc::redif_options wider = local;
	wider.neighbors = 2;
	wider.adaptive_geometry = false;

	mtrc::redif_options kth = local;
	kth.neighbors = 2;
	kth.iterations = 4;
	kth.euler_step = 0.15;
	kth.adaptive_geometry = false;
	kth.scale_policy = mtrc::redif_scale_policy::kth_local_distance;

	mtrc::redif_options global = local;
	global.neighbors = 3;
	global.iterations = 2;
	global.euler_step = 0.35;
	global.adaptive_geometry = false;
	global.scale_policy = mtrc::redif_scale_policy::global_mean_distance;

	const auto multiscale = mtrc::redif_multiscale_transport_paths(
		line, std::vector<mtrc::RedifScaleConfiguration>{{local, "local_fixed"},
														 {wider, "wider_fixed"},
														 {kth, "kth_fixed"},
														 {global, "global_fixed"}});
	assert(multiscale.exact);
	assert(multiscale.aggregate == "median_transport_path_length");
	assert(multiscale.scale_labels.size() == 4);
	assert(multiscale.scale_labels[0] == "local_fixed");
	assert(multiscale.scale_labels[1] == "wider_fixed");
	assert(multiscale.scale_labels[2] == "kth_fixed");
	assert(multiscale.scale_labels[3] == "global_fixed");
	assert(multiscale.scores.size() == line.size());
	assert(multiscale.scores[0].id == line.id(3));
	for (const auto &score : multiscale.scores) {
		assert(score.scale_path_lengths.size() == multiscale.scale_labels.size());
		assert(score.scale_stability_weights.size() == multiscale.scale_labels.size());
		assert(score.maximum_path_length >= score.median_path_length);
		assert(score.stability_weighted_path_length >= 0.0);
		for (const auto weight : score.scale_stability_weights) {
			assert(weight > 0.0);
			assert(weight <= 1.0);
		}
	}
	assert(multiscale.scores[0].median_path_length >= multiscale.scores[1].median_path_length);
	const auto &isolated = score_by_id(multiscale, line.id(3).index());
	for (std::size_t scale = 0; scale < multiscale.scale_labels.size(); ++scale) {
		for (const auto &score : multiscale.scores) {
			if (score.id != isolated.id) {
				assert(isolated.scale_path_lengths[scale] >= score.scale_path_lengths[scale]);
			}
		}
	}
	auto median_sensitive = kth;
	median_sensitive.scale_policy = mtrc::redif_scale_policy::median_local_distance;
	bool rejected_parameter_sensitive_scale = false;
	try {
		(void)mtrc::redif_remove_noise(line, median_sensitive);
	} catch (const std::runtime_error &) {
		rejected_parameter_sensitive_scale = true;
	}
	assert(rejected_parameter_sensitive_scale);

	auto bridge = mtrc::make_space(std::vector<int>{0, 1, 2, 6, 10, 11, 12}, AbsoluteDistance{});
	mtrc::redif_options bridge_local;
	bridge_local.neighbors = 2;
	bridge_local.iterations = 4;
	bridge_local.euler_step = 0.25;
	bridge_local.adaptive_geometry = false;
	mtrc::redif_options bridge_wide = bridge_local;
	bridge_wide.neighbors = 3;
	bridge_wide.euler_step = 0.18;
	mtrc::redif_options bridge_global = bridge_wide;
	bridge_global.scale_policy = mtrc::redif_scale_policy::global_mean_distance;
	const auto bridge_multiscale = mtrc::redif_multiscale_transport_paths(
		bridge, std::vector<mtrc::RedifScaleConfiguration>{{bridge_local, "bridge_fixed_k2"},
															{bridge_wide, "bridge_fixed_k3"},
															{bridge_global, "bridge_global_k3"}});
	assert(bridge_multiscale.scores[0].id == bridge.id(3));
	const auto &bridge_atom = score_by_id(bridge_multiscale, bridge.id(3).index());
	for (std::size_t scale = 0; scale < bridge_multiscale.scale_labels.size(); ++scale) {
		for (const auto &score : bridge_multiscale.scores) {
			if (score.id != bridge_atom.id) {
				assert(bridge_atom.scale_path_lengths[scale] >= score.scale_path_lengths[scale]);
			}
		}
	}

	bool rejected_empty_grid = false;
	try {
		(void)mtrc::redif_multiscale_transport_paths(line, {});
	} catch (const std::invalid_argument &) {
		rejected_empty_grid = true;
	}
	assert(rejected_empty_grid);

	return 0;
} catch (const std::exception &error) {
	std::cerr << error.what() << '\n';
	return 1;
}
