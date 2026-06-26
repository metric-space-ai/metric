// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <cmath>
#include <numeric>
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

struct CountingAbsoluteDistance {
	int *calls{};

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		const auto difference = lhs - rhs;
		return difference < 0 ? -difference : difference;
	}
};

struct StringLengthDistance {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> double
	{
		const auto left = static_cast<double>(lhs.size());
		const auto right = static_cast<double>(rhs.size());
		return std::abs(left - right);
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
	mtrc::redif_options options;
	options.neighbors = 1;
	options.iterations = 4;
	options.euler_step = 0.25;
	options.adaptive_geometry = true;

	auto line = mtrc::make_space(std::vector<int>{0, 1, 2, 100}, AbsoluteDistance{});
	const auto removal_paths = mtrc::redif_remove_noise(line, options);
	assert(removal_paths.operator_name == "redif_remove_noise");
	assert(removal_paths.strategy == "redif_inverse_implicit_euler_adaptive");
	assert(removal_paths.representation == "transport_measure_path");
	assert(removal_paths.size() == line.size());
	assert(removal_paths.iterations == options.iterations);
	assert(removal_paths.operator_diagnostics.size() == options.iterations);
	assert(removal_paths.entropy_diagnostics.size() == removal_paths.size());
	for (const auto &path : removal_paths.paths) {
		assert(path.measures.size() == options.iterations + 1);
		assert(path.step_transport.size() == options.iterations);
		assert(close_to(mass(path.measures.front()), 1.0));
		assert(close_to(mass(path.terminal_measure()), 1.0));
		assert(path.transport_path_length >= 0.0);
	}

	const auto redif_outliers = mtrc::redif_outliers(line, options);
	assert(redif_outliers.operator_name == "find_outliers");
	assert(redif_outliers.strategy == "redif_transport_path_length");
	assert(redif_outliers.size() == line.size());
	assert(redif_outliers[0].id == line.id(3));
	assert(redif_outliers[0].score > redif_outliers[1].score);

	int budget_calls = 0;
	auto budget_space = mtrc::make_space(std::vector<int>{0, 1, 2}, CountingAbsoluteDistance{&budget_calls});
	auto budget_options = options;
	budget_options.max_distance_evaluations = 2;
	bool rejected_budget = false;
	try {
		(void)mtrc::redif_remove_noise(budget_space, budget_options);
	} catch (const mtrc::MetricInputError &) {
		rejected_budget = true;
	}
	assert(rejected_budget);
	assert(budget_calls == 0);

	int transport_budget_calls = 0;
	auto transport_budget_space =
		mtrc::make_space(std::vector<int>{0, 1, 2, 3}, CountingAbsoluteDistance{&transport_budget_calls});
	auto transport_budget_options = options;
	transport_budget_options.max_transport_problems = 1;
	bool rejected_transport_budget = false;
	try {
		(void)mtrc::redif_remove_noise(transport_budget_space, transport_budget_options);
	} catch (const mtrc::MetricInputError &) {
		rejected_transport_budget = true;
	}
	assert(rejected_transport_budget);
	assert(transport_budget_calls == 0);

	// The same Redif measure dynamics works on non-vector records because the
	// state lives on the finite metric space's atoms, not in a coordinate chart.
	auto words = mtrc::make_space(
		std::vector<std::string>{"a", "aaa", "aaaaaa", "aaaaaaaaaa"}, StringLengthDistance{});
	const auto word_paths = mtrc::redif_remove_noise(words, options);
	assert(word_paths.size() == words.size());
	assert(close_to(mass(word_paths[0].terminal_measure()), 1.0));

	mtrc::redif_options noise_options;
	noise_options.neighbors = 2;
	noise_options.iterations = 3;
	noise_options.adaptive_geometry = false;
	const auto noise_paths = mtrc::redif_add_noise(words, noise_options);
	assert(noise_paths.operator_name == "redif_add_noise");
	assert(noise_paths.strategy == "redif_forward_euler_fixed");
	assert(noise_paths.transport_diagnostics.exact);
	assert(noise_paths.transport_diagnostics.transport_problem_count ==
		   noise_paths.record_count * noise_options.iterations);
	assert(noise_paths.size() == words.size());
	assert(noise_paths[0].step_transport.size() == noise_options.iterations);
	assert(noise_paths.operator_diagnostics.size() == noise_options.iterations);
	assert(noise_paths.entropy_diagnostics.size() == noise_paths.size());
	const auto entropy_path = mtrc::entropy_trajectory(noise_paths[0], noise_paths.initial_stationary);
	assert(entropy_path.shannon_entropy.back() > entropy_path.shannon_entropy.front());
	assert(entropy_path.relative_entropy_to_stationary.back() < entropy_path.relative_entropy_to_stationary.front());
	assert(noise_paths[0].transport_path_length > 0.0);

	const std::vector<std::vector<double>> four_point{{0.0, 1.0, 2.0, 3.0},
													  {1.0, 0.0, 1.0, 2.0},
													  {2.0, 1.0, 0.0, 1.0},
													  {3.0, 2.0, 1.0, 0.0}};
	auto exact_transport_options = noise_options;
	exact_transport_options.max_transport_support_atoms = 2;
	bool rejected_support_budget = false;
	try {
		(void)mtrc::redif_add_noise_from_distance_matrix(four_point, exact_transport_options);
	} catch (const mtrc::MetricInputError &) {
		rejected_support_budget = true;
	}
	assert(rejected_support_budget);

	auto truncated_transport_options = exact_transport_options;
	truncated_transport_options.allow_transport_support_truncation = true;
	const auto truncated_paths =
		mtrc::redif_add_noise_from_distance_matrix(four_point, truncated_transport_options);
	assert(!truncated_paths.exact);
	assert(!truncated_paths.transport_diagnostics.exact);
	assert(truncated_paths.transport_diagnostics.support_truncated);
	assert(truncated_paths.transport_diagnostics.truncated_measure_count > 0);
	assert(truncated_paths.transport_diagnostics.maximum_discarded_mass > 0.0);
	assert(truncated_paths.transport_diagnostics.exactness == "non_exact_support_truncated_transport");

	const std::vector<std::vector<double>> eight_point{{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0},
													   {1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0},
													   {2.0, 1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0},
													   {3.0, 2.0, 1.0, 0.0, 1.0, 2.0, 3.0, 4.0},
													   {4.0, 3.0, 2.0, 1.0, 0.0, 1.0, 2.0, 3.0},
													   {5.0, 4.0, 3.0, 2.0, 1.0, 0.0, 1.0, 2.0},
													   {6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0, 1.0},
													   {7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0}};
	mtrc::redif_options sampled_redif_options;
	sampled_redif_options.neighbors = 2;
	sampled_redif_options.iterations = 2;
	sampled_redif_options.adaptive_geometry = false;
	mtrc::redif_sampled_relation_options sampled_relation;
	sampled_relation.candidate_count = 2;
	sampled_relation.chunk_size = 4;
	const auto sampled_paths =
		mtrc::redif_remove_noise_from_sampled_distance_matrix(eight_point, sampled_redif_options, sampled_relation);
	assert(!sampled_paths.exact);
	assert(sampled_paths.representation == "sampled_chunked_transport_measure_path");
	assert(sampled_paths.operator_diagnostics.size() == sampled_redif_options.iterations);
	assert(sampled_paths.operator_diagnostics.front().local_relation_representation ==
		   "sampled_chunked_local_relation");
	assert(!sampled_paths.operator_diagnostics.front().local_relation_exact);
	assert(sampled_paths.operator_diagnostics.front().local_relation_exactness ==
		   "non_exact_sampled_local_relation");
	assert(sampled_paths.operator_diagnostics.front().local_relation_candidate_count == 2);
	assert(sampled_paths.operator_diagnostics.front().local_relation_chunk_size == 4);
	assert(sampled_paths.operator_diagnostics.front().local_relation_chunk_count == 2);
	assert(sampled_paths.operator_diagnostics.front().local_relation_distance_evaluations ==
		   eight_point.size() * sampled_relation.candidate_count);
	assert(sampled_paths.operator_diagnostics.front().local_relation_distance_evaluations <
		   (eight_point.size() * (eight_point.size() - 1)) / 2);

	return 0;
}
