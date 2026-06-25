// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_CLUSTERING_HPP
#define _METRIC_OPERATORS_CLUSTERING_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/record/id.hpp>
#include <metric/core/result.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/numeric/Math.h>

namespace mtrc::stats::structural_analysis {

namespace engine_detail {

constexpr std::size_t max_default_exact_metric_space_clustering_records = 4096;

inline auto require_default_exact_metric_space_clustering(std::size_t record_count) -> void
{
	if (record_count > max_default_exact_metric_space_clustering_records) {
		throw std::invalid_argument(
			"direct metric-space clustering overload refuses exact execution above 4096 records; "
			"use find_groups(...) for default large-space behavior or pass a pairwise distance provider for explicit "
			"exact execution");
	}
}

template <typename Provider>
auto initialize_medoids(const Provider &provider, std::size_t cluster_count) -> std::vector<RecordId>
{
	using distance_type = typename Provider::distance_type;

	std::vector<RecordId> medoids;
	medoids.reserve(cluster_count);

	RecordId first_medoid;
	distance_type best_total{};
	bool has_best = false;
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		const auto candidate = provider.id(index);
		const auto total = core::total_distance_to_provider_records(provider, candidate);
		if (!has_best || total < best_total) {
			first_medoid = candidate;
			best_total = total;
			has_best = true;
		}
	}
	medoids.push_back(first_medoid);

	std::vector<bool> is_selected(provider.record_count(), false);
	is_selected[provider.position_of(first_medoid)] = true;
	auto nearest_medoid_distances =
		core::distances_to_record_id(provider, first_medoid, "k-medoids medoid id is outside provider");

	while (medoids.size() < cluster_count) {
		const auto next_position = core::farthest_unselected_record_position(
			provider, nearest_medoid_distances, is_selected,
			"selected medoid count does not match provider distance count", "failed to initialize k-medoids");
		const auto next_medoid = provider.id(next_position);
		medoids.push_back(next_medoid);
		is_selected[next_position] = true;
		core::update_min_distances_to_record_id(provider, nearest_medoid_distances, next_medoid,
												"nearest medoid distance count does not match provider",
												"k-medoids medoid id is outside provider");
	}

	std::sort(medoids.begin(), medoids.end());
	return medoids;
}

template <typename Provider>
auto assign_to_medoids(const Provider &provider, const std::vector<RecordId> &medoids)
	-> std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
{
	using distance_type = typename Provider::distance_type;

	auto assignments = numeric::column_argmin_indices<distance_type>(
		medoids.size(), provider.record_count(), [&provider, &medoids](std::size_t cluster, std::size_t index) {
			return provider.distance(provider.id(index), medoids[cluster]);
		});
	auto cluster_sizes = numeric::index_counts(assignments, medoids.size());
	return {assignments, cluster_sizes};
}

template <typename Provider>
auto recompute_medoids(const Provider &provider, const std::vector<std::size_t> &assignments,
					   const std::vector<RecordId> &current_medoids)
	-> std::vector<RecordId>
{
	std::vector<RecordId> medoids;
	medoids.reserve(current_medoids.size());

	const auto ids_by_cluster = core::record_id_buckets_for_assignments(
		provider, assignments, current_medoids.size(), "k-medoids assignment references an unknown cluster");
	for (std::size_t cluster = 0; cluster < ids_by_cluster.size(); ++cluster) {
		if (ids_by_cluster[cluster].empty()) {
			medoids.push_back(current_medoids[cluster]);
			continue;
		}
		const auto &cluster_ids = ids_by_cluster[cluster];
		medoids.push_back(core::minimum_total_distance_record_id(provider, cluster_ids,
																 "k-medoids cluster must not be empty",
																 "k-medoids cluster id is outside provider"));
	}

	std::sort(medoids.begin(), medoids.end());
	return medoids;
}

template <typename Provider>
auto compute_cluster_medoids(const Provider &provider, const std::vector<std::size_t> &assignments,
							 std::size_t cluster_count) -> std::vector<RecordId>
{
	using distance_type = typename Provider::distance_type;
	const auto unassigned_label = ClusteringResult<distance_type>::unassigned_label;

	std::vector<RecordId> medoids;
	medoids.reserve(cluster_count);

	const auto ids_by_cluster = core::record_id_buckets_excluding_assignment(
		provider, assignments, cluster_count, unassigned_label, "clustering assignment references an unknown cluster");
	for (const auto &cluster_ids : ids_by_cluster) {
		if (!cluster_ids.empty()) {
			medoids.push_back(core::minimum_total_distance_record_id(provider, cluster_ids,
																	 "cluster must not be empty",
																	 "cluster id is outside provider"));
		}
	}

	return medoids;
}

template <typename Radius> auto validate_dbscan_parameters(Radius radius, std::size_t min_points) -> void
{
	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}
	if (min_points == 0) {
		throw std::invalid_argument("min_points must be positive");
	}
}

inline auto validate_affinity_propagation_parameters(double preference, int max_iterations, double tolerance,
													 double damping) -> void
{
	if (preference < 0.0 || preference >= 1.0) {
		throw std::invalid_argument("preference must be in the range [0, 1)");
	}
	if (max_iterations <= 0) {
		throw std::invalid_argument("max_iterations must be positive");
	}
	if (tolerance <= 0.0) {
		throw std::invalid_argument("tolerance must be positive");
	}
	if (damping < 0.0 || damping >= 1.0) {
		throw std::invalid_argument("damping must be in the range [0, 1)");
	}
}

template <typename Provider>
auto affinity_similarity_matrix(const Provider &provider, typename Provider::distance_type preference)
	-> numeric::DynamicMatrix<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	numeric::DynamicMatrix<distance_type> similarity(provider.record_count(), provider.record_count());
	distance_type minimum_similarity{};
	distance_type maximum_similarity{};
	bool has_similarity = false;

	for (std::size_t lhs = 0; lhs < provider.record_count(); ++lhs) {
		for (std::size_t rhs = lhs; rhs < provider.record_count(); ++rhs) {
			const auto distance = provider.distance(provider.id(lhs), provider.id(rhs));
			const auto current_similarity = -distance;
			if (!has_similarity || current_similarity < minimum_similarity) {
				minimum_similarity = current_similarity;
			}
			if (!has_similarity || current_similarity > maximum_similarity) {
				maximum_similarity = current_similarity;
			}
			has_similarity = true;
			similarity(lhs, rhs) = current_similarity;
			similarity(rhs, lhs) = current_similarity;
		}
	}

	numeric::set_diagonal(similarity,
						  preference * maximum_similarity + (distance_type{1} - preference) * minimum_similarity);

	return similarity;
}

template <typename Matrix, typename Distance>
auto update_affinity_responsibilities(Matrix &responsibilities, const Matrix &similarity, const Matrix &availabilities,
									  Distance damping) -> Distance
{
	const auto count = similarity.rows();
	Distance max_abs{};

	const auto row_maxima = numeric::row_top_two_values<Distance>(
		count, count, [&availabilities, &similarity](std::size_t row, std::size_t column) {
			return availabilities(row, column) + similarity(row, column);
		});

	for (std::size_t column = 0; column < count; ++column) {
		for (std::size_t row = 0; row < count; ++row) {
			const auto old_value = responsibilities(row, column);
			const auto &maxima = row_maxima[row];
			const auto competing_max =
				column == maxima.first_index ? maxima.second_value : maxima.first_value;
			const auto new_value = similarity(row, column) - competing_max;
			responsibilities(row, column) = damping * old_value + (Distance{1} - damping) * new_value;
			max_abs = std::max(max_abs, static_cast<Distance>(std::abs(old_value - new_value)));
		}
	}

	return max_abs;
}

template <typename Matrix, typename Distance>
auto update_affinity_availabilities(Matrix &availabilities, const Matrix &responsibilities, Distance damping)
	-> Distance
{
	const auto count = responsibilities.rows();
	Distance max_abs{};
	const auto positive_sums = numeric::positive_column_sums_excluding_diagonal(responsibilities);

	for (std::size_t column = 0; column < count; ++column) {
		const auto self_responsibility = responsibilities(column, column);
		const auto positive_sum = positive_sums[column];

		for (std::size_t row = 0; row < count; ++row) {
			const auto old_value = availabilities(row, column);
			Distance new_value{};
			if (row == column) {
				new_value = positive_sum;
			} else {
				auto value = self_responsibility + positive_sum;
				const auto responsibility = responsibilities(row, column);
				if (responsibility > Distance{}) {
					value -= responsibility;
				}
				new_value = value < Distance{} ? value : Distance{};
			}
			availabilities(row, column) = damping * old_value + (Distance{1} - damping) * new_value;
			max_abs = std::max(max_abs, static_cast<Distance>(std::abs(old_value - new_value)));
		}
	}

	return max_abs;
}

template <typename Matrix, typename Distance>
auto extract_affinity_exemplars(const Matrix &availabilities, const Matrix &responsibilities)
	-> std::vector<std::size_t>
{
	return numeric::positive_diagonal_indices<Distance>(
		availabilities.rows(), [&availabilities, &responsibilities](std::size_t index) {
			return availabilities(index, index) + responsibilities(index, index);
		});
}

template <typename Matrix, typename Distance>
auto assign_affinity_exemplars(const Matrix &similarity, const std::vector<std::size_t> &exemplars)
	-> std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
{
	const auto count = similarity.rows();
	const auto cluster_count = exemplars.size();
	auto assignments = numeric::column_argmax_indices<Distance>(
		cluster_count, count, [&similarity, &exemplars](std::size_t cluster, std::size_t record) {
			return similarity(exemplars[cluster], record);
		});

	for (std::size_t cluster = 0; cluster < cluster_count; ++cluster) {
		assignments[exemplars[cluster]] = cluster;
	}

	auto cluster_sizes = numeric::index_counts(assignments, cluster_count);

	return {assignments, cluster_sizes};
}

template <typename Provider, typename Radius>
auto dbscan_region_query(const Provider &provider, RecordId id, Radius radius) -> std::vector<RecordId>
{
	auto candidates = core::neighbor_candidates_within<typename Provider::distance_type>(
		provider.record_count(), [&provider](std::size_t index) { return provider.id(index); },
		[&provider, id](RecordId candidate, std::size_t) { return provider.distance(id, candidate); }, radius);
	return core::neighbor_ids(candidates);
}

template <typename Provider, typename Radius>
auto expand_dbscan_cluster(const Provider &provider, const std::vector<RecordId> &seed_neighbors, Radius radius,
						   std::size_t min_points, std::size_t cluster, std::vector<bool> &visited,
						   std::vector<bool> &assigned, std::vector<std::size_t> &assignments,
						   std::vector<bool> &core_records) -> void
{
	std::vector<RecordId> queue;
	std::vector<bool> queued(provider.record_count(), false);
	queue.reserve(seed_neighbors.size());

	for (const auto neighbor : seed_neighbors) {
		queue.push_back(neighbor);
		queued[provider.position_of(neighbor)] = true;
	}

	for (std::size_t cursor = 0; cursor < queue.size(); ++cursor) {
		const auto candidate = queue[cursor];
		const auto candidate_index = provider.position_of(candidate);

		if (!visited[candidate_index]) {
			visited[candidate_index] = true;
			auto neighbors = dbscan_region_query(provider, candidate, radius);
			if (neighbors.size() >= min_points) {
				core_records[candidate_index] = true;
				for (const auto neighbor : neighbors) {
					const auto neighbor_position = provider.position_of(neighbor);
					if (!queued[neighbor_position]) {
						queue.push_back(neighbor);
						queued[neighbor_position] = true;
					}
				}
			}
		}

		if (!assigned[candidate_index]) {
			assignments[candidate_index] = cluster;
			assigned[candidate_index] = true;
		}
	}
}

} // namespace engine_detail

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto kmedoids(const Provider &provider, std::size_t cluster_count, std::size_t max_iterations = 100)
	-> ClusteringResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot cluster an empty distance provider");
	}
	if (cluster_count == 0) {
		throw std::invalid_argument("cluster_count must be positive");
	}
	if (cluster_count > provider.record_count()) {
		throw std::invalid_argument("cluster_count cannot exceed record_count");
	}

	auto medoids = engine_detail::initialize_medoids(provider, cluster_count);
	auto assignment_state = engine_detail::assign_to_medoids(provider, medoids);
	auto assignments = std::move(assignment_state.first);
	auto cluster_sizes = std::move(assignment_state.second);

	std::size_t iterations = 0;
	bool converged = false;
	for (; iterations < max_iterations; ++iterations) {
		auto updated_medoids = engine_detail::recompute_medoids(provider, assignments, medoids);
		auto updated_assignment_state = engine_detail::assign_to_medoids(provider, updated_medoids);
		auto updated_assignments = std::move(updated_assignment_state.first);
		auto updated_cluster_sizes = std::move(updated_assignment_state.second);

		if (updated_medoids == medoids && updated_assignments == assignments) {
			converged = true;
			medoids = std::move(updated_medoids);
			assignments = std::move(updated_assignments);
			cluster_sizes = std::move(updated_cluster_sizes);
			++iterations;
			break;
		}

		medoids = std::move(updated_medoids);
		assignments = std::move(updated_assignments);
		cluster_sizes = std::move(updated_cluster_sizes);
	}

	return core::make_clustering_result<distance_type>(std::move(assignments), std::move(medoids), {}, {},
													   std::move(cluster_sizes), iterations, converged, "kmedoids",
													   "pairwise_distances");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto kmedoids(const Space &space, std::size_t cluster_count, std::size_t max_iterations = 100)
	-> ClusteringResult<typename Space::distance_type>
{
	engine_detail::require_default_exact_metric_space_clustering(space.size());
	space::storage::LiveDistances<Space> provider(space);
	auto result = kmedoids(provider, cluster_count, max_iterations);
	result.representation = "metric_space";
	return result;
}

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto dbscan(const Provider &provider, Radius radius, std::size_t min_points)
	-> ClusteringResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	const auto unassigned_label = ClusteringResult<distance_type>::unassigned_label;

	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot cluster an empty distance provider");
	}
	engine_detail::validate_dbscan_parameters(radius, min_points);

	std::vector<std::size_t> assignments(provider.record_count(), unassigned_label);
	std::vector<bool> assigned(provider.record_count(), false);
	std::vector<bool> visited(provider.record_count(), false);
	std::vector<bool> core_record_flags(provider.record_count(), false);

	std::size_t cluster_count = 0;
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		if (visited[index]) {
			continue;
		}

		const auto id = provider.id(index);
		visited[index] = true;
		auto neighbors = engine_detail::dbscan_region_query(provider, id, radius);
		if (neighbors.size() < min_points) {
			continue;
		}

		core_record_flags[index] = true;
		engine_detail::expand_dbscan_cluster(provider, neighbors, radius, min_points, cluster_count, visited, assigned,
											 assignments, core_record_flags);
		++cluster_count;
	}

	auto unassigned_records = core::record_ids_matching_value(provider, assignments, unassigned_label);
	auto core_records = core::record_ids_matching_value(provider, core_record_flags, true);
	auto cluster_sizes = numeric::index_counts_excluding(assignments, cluster_count, unassigned_label,
														"dbscan assignment references an unknown cluster");
	auto medoids = engine_detail::compute_cluster_medoids(provider, assignments, cluster_count);

	return core::make_clustering_result<distance_type>(std::move(assignments), std::move(medoids),
													   std::move(core_records), std::move(unassigned_records),
													   std::move(cluster_sizes), 1, true, "dbscan",
													   "pairwise_distances");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto dbscan(const Space &space, Radius radius, std::size_t min_points)
	-> ClusteringResult<typename Space::distance_type>
{
	engine_detail::require_default_exact_metric_space_clustering(space.size());
	space::storage::LiveDistances<Space> provider(space);
	auto result = dbscan(provider, radius, min_points);
	result.representation = "metric_space";
	return result;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto affinity_propagation(const Provider &provider, double preference = 0.5, int max_iterations = 200,
						  double tolerance = 1.0e-6, double damping = 0.5)
	-> ClusteringResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	static_assert(std::is_floating_point<distance_type>::value,
				  "mtrc::stats::structural_analysis::affinity_propagation requires a floating-point distance type");

	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot cluster an empty distance provider");
	}
	engine_detail::validate_affinity_propagation_parameters(preference, max_iterations, tolerance, damping);
	if (provider.record_count() == 1) {
		return core::make_clustering_result<distance_type>(
			std::vector<std::size_t>{0}, std::vector<RecordId>{provider.id(0)}, {}, {}, std::vector<std::size_t>{1}, 0,
			true, "affinity_propagation", "pairwise_distances");
	}

	const auto preference_value = static_cast<distance_type>(preference);
	const auto tolerance_value = static_cast<distance_type>(tolerance);
	const auto damping_value = static_cast<distance_type>(damping);

	auto similarity = engine_detail::affinity_similarity_matrix(provider, preference_value);
	numeric::DynamicMatrix<distance_type> responsibilities(provider.record_count(), provider.record_count());
	numeric::DynamicMatrix<distance_type> availabilities(provider.record_count(), provider.record_count());

	int iterations = 0;
	bool converged = false;
	while (!converged && iterations < max_iterations) {
		++iterations;
		const auto responsibility_delta = engine_detail::update_affinity_responsibilities(
			responsibilities, similarity, availabilities, damping_value);
		const auto availability_delta =
			engine_detail::update_affinity_availabilities(availabilities, responsibilities, damping_value);
		const auto change = std::max(availability_delta, responsibility_delta) / (distance_type{1} - damping_value);
		converged = change < tolerance_value;
	}

	auto exemplars = engine_detail::extract_affinity_exemplars<decltype(availabilities), distance_type>(
		availabilities, responsibilities);
	if (exemplars.empty()) {
		// Degenerate space (e.g. every pairwise distance equal or zero): the message-passing
		// fixed point exposes no positive self-availability, so no exemplar emerges. Rather
		// than failing a valid investigation with an exception, fall back deterministically to
		// the single global medoid (the record with minimum total distance to all others) as
		// the sole exemplar; every record is then assigned to that one cluster. record_count()
		// is >= 2 here (the empty and singleton cases returned earlier).
		std::vector<RecordId> all_ids;
		all_ids.reserve(provider.record_count());
		for (std::size_t position = 0; position < provider.record_count(); ++position) {
			all_ids.push_back(provider.id(position));
		}
		const auto medoid_id = core::minimum_total_distance_record_id(
			provider, all_ids, "affinity propagation fallback requires a non-empty space",
			"affinity propagation fallback id is outside provider");
		exemplars = std::vector<std::size_t>{provider.position_of(medoid_id)};
	}

	auto [assignments, cluster_sizes] =
		engine_detail::assign_affinity_exemplars<decltype(similarity), distance_type>(similarity, exemplars);

	auto medoids = core::record_ids_at_positions(provider, exemplars);

	return core::make_clustering_result<distance_type>(std::move(assignments), std::move(medoids), {}, {},
													   std::move(cluster_sizes), static_cast<std::size_t>(iterations),
													   converged, "affinity_propagation", "pairwise_distances");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto affinity_propagation(const Space &space, double preference = 0.5, int max_iterations = 200,
						  double tolerance = 1.0e-6, double damping = 0.5)
	-> ClusteringResult<typename Space::distance_type>
{
	engine_detail::require_default_exact_metric_space_clustering(space.size());
	space::storage::LiveDistances<Space> provider(space);
	auto result = affinity_propagation(provider, preference, max_iterations, tolerance, damping);
	result.representation = "metric_space";
	return result;
}

} // namespace mtrc::stats::structural_analysis

#endif
