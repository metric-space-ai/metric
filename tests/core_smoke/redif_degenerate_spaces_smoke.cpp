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

auto path_by_id(const mtrc::RedifMeasureResult &result, std::size_t id_index) -> const mtrc::RedifMeasurePath &
{
	for (const auto &path : result.paths) {
		if (path.id.index() == id_index) {
			return path;
		}
	}
	throw std::logic_error("missing Redif path id");
}

} // namespace

int main()
{
	{
		mtrc::Wasserstein<double> two_point({{0.0, 1.0}, {1.0, 0.0}});
		assert(close_to(two_point(std::vector<double>{1.0, 0.0}, std::vector<double>{0.0, 1.0}), 1.0));
		mtrc::Wasserstein<double> three_point({{0.0, 1.0, 2.0},
											   {1.0, 0.0, 1.0},
											   {2.0, 1.0, 0.0}});
		assert(close_to(three_point(std::vector<double>{1.0, 0.0, 0.0},
									std::vector<double>{0.0, 0.0, 1.0}), 2.0));
	}

	auto duplicated = mtrc::make_space(std::vector<int>{0, 0, 2}, AbsoluteDistance{});
	mtrc::redif_options duplicate_options;
	duplicate_options.neighbors = 1;
	duplicate_options.iterations = 2;
	duplicate_options.euler_step = 0.25;
	duplicate_options.adaptive_geometry = false;
	const auto duplicate_op = mtrc::redif_operator(duplicated, duplicate_options);
	assert(duplicate_op.local_relation[0][1] != 0);
	assert(close_to(duplicate_op.affinity[0][1], 1.0));
	bool rejected_pseudometric_transport = false;
	try {
		(void)mtrc::redif_add_noise(duplicated, duplicate_options);
	} catch (const std::invalid_argument &) {
		rejected_pseudometric_transport = true;
	}
	assert(rejected_pseudometric_transport);

	auto two_components = mtrc::make_space(std::vector<int>{0, 1, 10, 11}, AbsoluteDistance{});
	mtrc::redif_options component_options;
	component_options.neighbors = 1;
	component_options.iterations = 1;
	component_options.euler_step = 0.25;
	component_options.adaptive_geometry = false;
	const auto component_op = mtrc::redif_operator(two_components, component_options);
	assert(component_op.diagnostics.component_count == 2);
	assert(component_op.local_relation[0][1] != 0);
	assert(component_op.local_relation[2][3] != 0);
	assert(component_op.local_relation[1][2] == 0);
	const auto component_noise = mtrc::redif_add_noise(two_components, component_options);
	for (const auto &path : component_noise.paths) {
		assert(close_to(mass(path.terminal_measure()), 1.0));
	}

	const std::vector<std::vector<double>> ultrametric{{0.0, 1.0, 2.0, 2.0},
													   {1.0, 0.0, 2.0, 2.0},
													   {2.0, 2.0, 0.0, 1.0},
													   {2.0, 2.0, 1.0, 0.0}};
	mtrc::redif_options ultra_options;
	ultra_options.neighbors = 3;
	ultra_options.iterations = 2;
	ultra_options.euler_step = 0.25;
	ultra_options.adaptive_geometry = false;
	const auto ultra_op = mtrc::redif_operator_from_distance_matrix(ultrametric, ultra_options);
	assert(ultra_op.diagnostics.component_count == 1);
	for (std::size_t row = 0; row < ultra_op.size(); ++row) {
		assert(close_to(ultra_op.transition_row_sum(row), 1.0));
	}

	const std::vector<std::vector<double>> permuted{{0.0, 2.0, 2.0, 1.0},
													{2.0, 0.0, 1.0, 2.0},
													{2.0, 1.0, 0.0, 2.0},
													{1.0, 2.0, 2.0, 0.0}};
	const auto permuted_op = mtrc::redif_operator_from_distance_matrix(permuted, ultra_options);
	assert(close_to(permuted_op.affinity[0][3], ultra_op.affinity[0][1]));
	assert(close_to(permuted_op.affinity[1][2], ultra_op.affinity[2][3]));

	const std::vector<std::vector<double>> star{{0.0, 1.0, 1.0, 1.0, 1.0},
												{1.0, 0.0, 2.0, 2.0, 2.0},
												{1.0, 2.0, 0.0, 2.0, 2.0},
												{1.0, 2.0, 2.0, 0.0, 2.0},
												{1.0, 2.0, 2.0, 2.0, 0.0}};
	mtrc::redif_options star_options;
	star_options.neighbors = 1;
	star_options.iterations = 1;
	const auto star_op = mtrc::redif_operator_from_distance_matrix(star, star_options);
	assert(star_op.diagnostics.component_count == 1);
	assert(star_op.degree[0] > star_op.degree[1]);
	for (std::size_t row = 0; row < star_op.size(); ++row) {
		assert(close_to(star_op.transition_row_sum(row), 1.0));
	}

	const std::vector<std::vector<double>> cycle{{0.0, 1.0, 2.0, 1.0},
												 {1.0, 0.0, 1.0, 2.0},
												 {2.0, 1.0, 0.0, 1.0},
												 {1.0, 2.0, 1.0, 0.0}};
	mtrc::redif_options cycle_options;
	cycle_options.neighbors = 2;
	cycle_options.iterations = 1;
	const auto cycle_op = mtrc::redif_operator_from_distance_matrix(cycle, cycle_options);
	assert(cycle_op.diagnostics.component_count == 1);
	assert(close_to(cycle_op.degree[0], cycle_op.degree[1]));
	assert(close_to(cycle_op.degree[1], cycle_op.degree[2]));
	assert(close_to(cycle_op.degree[2], cycle_op.degree[3]));
	assert(cycle_op.diagnostics.spectral_gap_proxy == "minimum_transition_escape_probability");
	assert(cycle_op.diagnostics.spectral_gap_proxy_value > 0.0);

	const std::vector<std::vector<double>> relabel_base{{0.0, 1.0, 3.0, 10.0},
														{1.0, 0.0, 2.0, 9.0},
														{3.0, 2.0, 0.0, 7.0},
														{10.0, 9.0, 7.0, 0.0}};
	const std::vector<std::size_t> permutation{2, 0, 3, 1};
	std::vector<std::vector<double>> relabelled(permutation.size(),
												std::vector<double>(permutation.size(), 0.0));
	std::vector<mtrc::RecordId> relabelled_ids;
	relabelled_ids.reserve(permutation.size());
	for (std::size_t row = 0; row < permutation.size(); ++row) {
		relabelled_ids.push_back(mtrc::RecordId::from_index(permutation[row]));
		for (std::size_t column = 0; column < permutation.size(); ++column) {
			relabelled[row][column] = relabel_base[permutation[row]][permutation[column]];
		}
	}
	mtrc::redif_options relabel_options;
	relabel_options.neighbors = 2;
	relabel_options.iterations = 3;
	relabel_options.euler_step = 0.2;
	relabel_options.adaptive_geometry = true;
	const auto base_result = mtrc::redif_remove_noise_from_distance_matrix(relabel_base, relabel_options);
	const auto relabelled_result =
		mtrc::redif_remove_noise_from_distance_matrix(relabelled, relabel_options, relabelled_ids);
	for (std::size_t original_id = 0; original_id < permutation.size(); ++original_id) {
		const auto &base_path = path_by_id(base_result, original_id);
		const auto &relabelled_path = path_by_id(relabelled_result, original_id);
		assert(close_to(base_path.transport_path_length, relabelled_path.transport_path_length, 1.0e-8));
		for (std::size_t frame = 0; frame < base_path.measures.size(); ++frame) {
			for (std::size_t relabelled_column = 0; relabelled_column < permutation.size(); ++relabelled_column) {
				const auto original_column = permutation[relabelled_column];
				assert(close_to(base_path.measures[frame][original_column],
								relabelled_path.measures[frame][relabelled_column], 1.0e-8));
			}
		}
	}

	mtrc::redif_options stress_options;
	stress_options.neighbors = 3;
	stress_options.iterations = 3;
	stress_options.euler_step = 0.2;
	stress_options.adaptive_geometry = true;
	const auto stress = mtrc::redif_add_noise(
		mtrc::make_space(std::vector<int>{0, 1, 2, 4, 8, 16}, AbsoluteDistance{}), stress_options);
	assert(stress.transport_diagnostics.exact);
	assert(stress.transport_diagnostics.measure_count == 6);
	assert(stress.transport_diagnostics.pair_count == 15);
	for (const auto &diagnostics : stress.operator_diagnostics) {
		assert(diagnostics.spectral_gap_proxy == "minimum_transition_escape_probability");
		assert(diagnostics.spectral_gap_proxy_value >= 0.0);
	}

	return 0;
}
