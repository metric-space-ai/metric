// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "metric/engine.hpp"

namespace {

auto close_to(double lhs, double rhs, double tolerance = 1e-9) -> bool
{
	return std::abs(lhs - rhs) <= tolerance;
}

auto expect_invalid(const std::vector<std::vector<double>> &distances) -> void
{
	mtrc::redif_options options;
	options.neighbors = 1;
	options.iterations = 1;
	bool rejected = false;
	try {
		(void)mtrc::redif_operator_from_distance_matrix(distances, options);
	} catch (const std::invalid_argument &) {
		rejected = true;
	}
	assert(rejected);
}

auto exact_entries(const std::vector<std::vector<double>> &distances, std::size_t neighbors)
	-> std::vector<mtrc::RedifLocalRelationEntry>
{
	std::vector<mtrc::RedifLocalRelationEntry> entries;
	const auto n = distances.size();
	const auto k = std::min(neighbors, n > 0 ? n - 1 : 0);
	for (std::size_t i = 0; i < n; ++i) {
		std::vector<std::size_t> order;
		for (std::size_t j = 0; j < n; ++j) {
			if (i != j) {
				order.push_back(j);
			}
		}
		std::sort(order.begin(), order.end(), [&](std::size_t lhs, std::size_t rhs) {
			if (distances[i][lhs] != distances[i][rhs]) {
				return distances[i][lhs] < distances[i][rhs];
			}
			return lhs < rhs;
		});
		for (std::size_t t = 0; t < k; ++t) {
			entries.push_back(mtrc::RedifLocalRelationEntry{i, order[t], distances[i][order[t]]});
		}
	}
	return entries;
}

auto provider_from_entries(const std::vector<mtrc::RedifLocalRelationEntry> &entries, int *calls)
{
	return [&entries, calls](std::size_t source, std::size_t) {
		std::vector<std::pair<std::size_t, double>> row;
		for (const auto &entry : entries) {
			if (entry.source == source) {
				row.push_back({entry.target, entry.distance});
			}
		}
		*calls += static_cast<int>(row.size());
		return row;
	};
}

} // namespace

int main()
{
	const std::vector<std::vector<double>> line{{0.0, 1.0, 5.0},
											   {1.0, 0.0, 4.0},
											   {5.0, 4.0, 0.0}};

	for (const auto policy : {mtrc::redif_scale_policy::mean_local_distance,
							  mtrc::redif_scale_policy::median_local_distance,
							  mtrc::redif_scale_policy::kth_local_distance,
							  mtrc::redif_scale_policy::global_mean_distance}) {
		mtrc::redif_options options;
		options.neighbors = 2;
		options.iterations = 1;
		options.scale_policy = policy;
		const auto op = mtrc::redif_operator_from_distance_matrix(line, options);
		assert(op.size() == 3);
		assert(op.affinity_kernel == "redif_self_tuned_heat_kernel");
		assert(!op.local_scale_policy.empty());
		assert(op.diagnostics.reversible);
		assert(op.diagnostics.component_count == 1);
		assert(op.diagnostics.spectral_gap_proxy == "minimum_transition_escape_probability");
		assert(op.diagnostics.spectral_gap_proxy_value > 0.0);
		assert(op.diagnostics.minimum_transition_escape_probability > 0.0);
		assert(op.diagnostics.maximum_self_transition_probability < 1.0);
		for (std::size_t i = 0; i < op.size(); ++i) {
			assert(op.local_scale[i] > 0.0);
			assert(close_to(op.transition_row_sum(i), 1.0));
			for (std::size_t j = 0; j < op.size(); ++j) {
				assert(op.affinity[i][j] >= 0.0);
				assert(op.affinity[i][j] <= 1.0);
				assert(close_to(op.affinity[i][j], op.affinity[j][i]));
				assert(close_to(op.stationary[i] * op.transition[i][j],
								op.stationary[j] * op.transition[j][i]));
			}
		}
		assert(op.affinity[0][1] > op.affinity[1][2]);
		assert(op.affinity[1][2] > op.affinity[0][2]);
	}

	mtrc::redif_options sparse_options;
	sparse_options.neighbors = 2;
	sparse_options.iterations = 1;
	sparse_options.scale_policy = mtrc::redif_scale_policy::mean_local_distance;
	const auto directed_entries = exact_entries(line, sparse_options.neighbors);
	const auto sparse = mtrc::redif_sparse_operator_from_exact_local_relation(line.size(), directed_entries,
																			  sparse_options);
	const auto dense = mtrc::redif_operator_from_distance_matrix(line, sparse_options);
	const auto dense_from_sparse = mtrc::redif_operator_from_sparse_operator(sparse);
	assert(sparse.size() == dense.size());
	assert(sparse.local_relation_diagnostics.exact);
	assert(sparse.local_relation_diagnostics.representation == "exact_neighbor_provider");
	assert(sparse.diagnostics.local_relation_exact);
	assert(sparse.diagnostics.dense_distance_evaluations == 0);
	assert(dense.diagnostics.dense_distance_evaluations == (line.size() * (line.size() - 1)) / 2);
	for (std::size_t i = 0; i < dense.size(); ++i) {
		assert(close_to(sparse.transition_row_sum(i), dense.transition_row_sum(i)));
		assert(close_to(dense_from_sparse.transition_row_sum(i), dense.transition_row_sum(i)));
		assert(close_to(sparse.local_scale[i], dense.local_scale[i]));
		assert(close_to(sparse.stationary[i], dense.stationary[i]));
		for (std::size_t j = 0; j < dense.size(); ++j) {
			assert(dense_from_sparse.local_relation[i][j] == dense.local_relation[i][j]);
			assert(close_to(dense_from_sparse.affinity[i][j], dense.affinity[i][j]));
			assert(close_to(dense_from_sparse.transition[i][j], dense.transition[i][j]));
		}
	}

	int provider_calls = 0;
	const auto sparse_from_provider = mtrc::redif_sparse_operator_from_exact_neighbor_provider(
		line.size(), provider_from_entries(directed_entries, &provider_calls), sparse_options);
	assert(provider_calls == static_cast<int>(line.size() * sparse_options.neighbors));
	assert(sparse_from_provider.local_relation_diagnostics.distance_evaluations ==
		   line.size() * sparse_options.neighbors);
	assert(sparse_from_provider.diagnostics.local_relation_distance_evaluations ==
		   line.size() * sparse_options.neighbors);

	const std::vector<std::vector<double>> larger_line{
		{0.0, 1.0, 2.0, 3.0, 4.0, 5.0},
		{1.0, 0.0, 1.0, 2.0, 3.0, 4.0},
		{2.0, 1.0, 0.0, 1.0, 2.0, 3.0},
		{3.0, 2.0, 1.0, 0.0, 1.0, 2.0},
		{4.0, 3.0, 2.0, 1.0, 0.0, 1.0},
		{5.0, 4.0, 3.0, 2.0, 1.0, 0.0}};
	const auto larger_entries = exact_entries(larger_line, sparse_options.neighbors);
	int larger_provider_calls = 0;
	(void)mtrc::redif_sparse_operator_from_exact_neighbor_provider(
		larger_line.size(), provider_from_entries(larger_entries, &larger_provider_calls), sparse_options);
	assert(larger_provider_calls == static_cast<int>(larger_line.size() * sparse_options.neighbors));
	assert(static_cast<std::size_t>(larger_provider_calls) <
		   (larger_line.size() * (larger_line.size() - 1)) / 2);

	auto limited_sparse_options = sparse_options;
	limited_sparse_options.max_distance_evaluations = line.size() * sparse_options.neighbors - 1;
	int refused_provider_calls = 0;
	bool rejected_sparse_budget = false;
	try {
		(void)mtrc::redif_sparse_operator_from_exact_neighbor_provider(
			line.size(), provider_from_entries(directed_entries, &refused_provider_calls), limited_sparse_options);
	} catch (const mtrc::MetricInputError &) {
		rejected_sparse_budget = true;
	}
	assert(rejected_sparse_budget);
	assert(refused_provider_calls == 0);

	bool rejected_incomplete_relation = false;
	try {
		std::vector<mtrc::RedifLocalRelationEntry> incomplete = directed_entries;
		incomplete.pop_back();
		(void)mtrc::redif_sparse_operator_from_exact_local_relation(line.size(), incomplete, sparse_options);
	} catch (const std::invalid_argument &) {
		rejected_incomplete_relation = true;
	}
	assert(rejected_incomplete_relation);

	const auto refusal = mtrc::redif_local_relation_refusal(5, 2, "sampled_neighbor_provider",
														   "sampled relation cannot support exact Redif");
	assert(refusal.refused);
	assert(!refusal.exact);
	assert(refusal.exactness == "refused");
	assert(refusal.refusal_reason.find("exact") != std::string::npos);

	mtrc::redif_options zero_options;
	zero_options.neighbors = 1;
	zero_options.iterations = 1;
	const std::vector<std::vector<double>> duplicated{{0.0, 0.0, 2.0},
													  {0.0, 0.0, 2.0},
													  {2.0, 2.0, 0.0}};
	const auto duplicate_op = mtrc::redif_operator_from_distance_matrix(duplicated, zero_options);
	assert(duplicate_op.local_relation[0][1] != 0);
	assert(duplicate_op.local_relation[1][0] != 0);
	assert(close_to(duplicate_op.affinity[0][1], 1.0));
	assert(close_to(duplicate_op.affinity[1][0], 1.0));

	expect_invalid({{0.0, 1.0}, {1.0}});
	expect_invalid({{0.0, -1.0}, {-1.0, 0.0}});
	expect_invalid({{0.0, 1.0}, {2.0, 0.0}});
	expect_invalid({{1.0, 0.0}, {0.0, 0.0}});
	expect_invalid({});
	expect_invalid({{0.0, std::numeric_limits<double>::quiet_NaN()},
					{std::numeric_limits<double>::quiet_NaN(), 0.0}});
	expect_invalid({{0.0, std::numeric_limits<double>::infinity()},
					{std::numeric_limits<double>::infinity(), 0.0}});

	return 0;
}
