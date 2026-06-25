// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MODIFY_COMPRESS_HPP
#define _METRIC_MODIFY_COMPRESS_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/space/sample_plan.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/space/select/options.hpp>
#include <metric/space/select/representatives.hpp>
#include <metric/stats/structural_analysis/groups.hpp>
#include <metric/stats/structural_analysis/options.hpp>

namespace mtrc::modify::reduce {
namespace detail {

constexpr std::size_t default_approximate_compression_sample_count = 512;
constexpr std::size_t max_default_exact_compression_records = 4096;

inline auto approximate_compression_sample_count(std::size_t record_count, std::size_t requested_count,
												 space::storage::policy runtime_policy) -> std::size_t
{
	if (record_count == 0) {
		return 0;
	}
	auto sample_count = runtime_policy.graph_neighbors() == 0
							? std::min(default_approximate_compression_sample_count, record_count)
							: std::min(runtime_policy.graph_neighbors(), record_count);
	(void)requested_count;
	return sample_count;
}

inline auto contains_candidate_record_id(const std::vector<RecordId> &ids, RecordId id) -> bool
{
	return std::find(ids.begin(), ids.end(), id) != ids.end();
}

template <typename Provider>
auto sampled_regular_representatives(const Provider &provider, std::size_t count,
									 space::select::farthest_first strategy)
	-> RepresentativeSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (count == 0) {
		return core::make_representative_set(std::vector<RecordId>{}, std::vector<distance_type>{},
											 provider.record_count(), count, "sampled_regular",
											 "sampled_metric_space", false);
	}
	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot select representatives from an empty distance provider");
	}
	if (count > provider.record_count()) {
		throw std::invalid_argument("representative count cannot exceed the number of records");
	}
	if (strategy.seed_index >= provider.record_count()) {
		throw std::out_of_range("seed_index is outside the metric space");
	}

	std::vector<RecordId> representatives;
	representatives.reserve(count);
	auto add_position = [&provider, &representatives, count](std::size_t position) {
		const auto id = provider.id(position);
		if (!contains_candidate_record_id(representatives, id)) {
			representatives.push_back(id);
		}
		return representatives.size() == count;
	};

	(void)add_position(strategy.seed_index);
	const auto candidate_plan = ::mtrc::space::regular_sample_positions(provider.record_count(), count);
	for (const auto position : candidate_plan.positions) {
		if (add_position(position)) {
			break;
		}
	}
	for (std::size_t position = 0; representatives.size() < count && position < provider.record_count(); ++position) {
		(void)add_position(position);
	}

	return core::make_representative_set(std::move(representatives), std::vector<distance_type>{},
										 provider.record_count(), count, "sampled_regular",
										 "sampled_metric_space", false);
}

template <typename Provider>
auto sampled_farthest_first_representatives(const Provider &provider, std::size_t count,
											space::select::farthest_first strategy,
											space::storage::policy runtime_policy)
	-> RepresentativeSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (count == 0) {
		return core::make_representative_set(std::vector<RecordId>{}, std::vector<distance_type>{},
											 provider.record_count(), count, "sampled_farthest_first",
											 "sampled_metric_space", false);
	}
	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot select representatives from an empty distance provider");
	}
	if (count > provider.record_count()) {
		throw std::invalid_argument("representative count cannot exceed the number of records");
	}
	if (strategy.seed_index >= provider.record_count()) {
		throw std::out_of_range("seed_index is outside the metric space");
	}

	const auto sample_count = approximate_compression_sample_count(provider.record_count(), count, runtime_policy);
	if (count > sample_count) {
		return sampled_regular_representatives(provider, count, strategy);
	}

	const auto seed_id = provider.id(strategy.seed_index);
	std::vector<RecordId> candidate_ids;
	candidate_ids.reserve(sample_count);
	candidate_ids.push_back(seed_id);
	const auto candidate_plan = ::mtrc::space::regular_sample_positions(provider.record_count(), sample_count);
	for (const auto position : candidate_plan.positions) {
		const auto candidate_id = provider.id(position);
		if (!contains_candidate_record_id(candidate_ids, candidate_id)) {
			candidate_ids.push_back(candidate_id);
		}
		if (candidate_ids.size() == sample_count) {
			break;
		}
	}

	std::vector<RecordId> representatives;
	representatives.reserve(count);
	representatives.push_back(seed_id);
	std::vector<bool> selected(candidate_ids.size(), false);
	selected[0] = true;

	std::vector<distance_type> nearest_candidate_distances;
	nearest_candidate_distances.reserve(candidate_ids.size());
	for (const auto candidate_id : candidate_ids) {
		nearest_candidate_distances.push_back(provider.distance(candidate_id, seed_id));
	}

	while (representatives.size() < count) {
		const auto next_index = core::farthest_unselected_position(
			nearest_candidate_distances, selected, "candidate selection count does not match distance count",
			"failed to select the next sampled representative");
		const auto next_id = candidate_ids[next_index];
		representatives.push_back(next_id);
		selected[next_index] = true;
		for (std::size_t candidate_index = 0; candidate_index < candidate_ids.size(); ++candidate_index) {
			const auto distance = provider.distance(candidate_ids[candidate_index], next_id);
			if (distance < nearest_candidate_distances[candidate_index]) {
				nearest_candidate_distances[candidate_index] = distance;
			}
		}
	}

	auto nearest_representative_distances =
		core::distances_to_record_id(provider, representatives.front(), "representative id is outside provider");
	for (std::size_t representative_index = 1; representative_index < representatives.size(); ++representative_index) {
		core::update_min_distances_to_record_id(provider, nearest_representative_distances,
												representatives[representative_index],
												"nearest representative distance count does not match provider",
												"representative id is outside provider");
	}

	return core::make_representative_set(std::move(representatives), std::move(nearest_representative_distances),
										 provider.record_count(), count, "sampled_farthest_first",
										 "sampled_metric_space", false);
}

template <typename Provider>
auto assign_records_to_sampled_representatives(const Provider &provider, const std::vector<RecordId> &representatives,
											   std::size_t candidate_count, const char *empty_message,
											   const char *not_found_message)
	-> core::RepresentativeAssignment<typename Provider::distance_type>
{
	if (representatives.empty()) {
		throw std::invalid_argument(empty_message);
	}
	for (const auto id : representatives) {
		if (!provider.contains(id)) {
			throw std::invalid_argument(not_found_message);
		}
	}

	const auto sampled_count = std::min(candidate_count, representatives.size());
	auto candidate_positions = ::mtrc::space::regular_sample_positions(representatives.size(), sampled_count).positions;
	if (candidate_positions.empty()) {
		candidate_positions.push_back(0);
	}

	core::RepresentativeAssignment<typename Provider::distance_type> result;
	result.assignments.reserve(provider.record_count());
	result.nearest_distances.reserve(provider.record_count());
	for (std::size_t record_index = 0; record_index < provider.record_count(); ++record_index) {
		const auto source_id = provider.id(record_index);
		auto best_representative_index = candidate_positions.front();
		auto best_distance = provider.distance(source_id, representatives[best_representative_index]);
		for (std::size_t candidate_index = 1; candidate_index < candidate_positions.size(); ++candidate_index) {
			const auto representative_index = candidate_positions[candidate_index];
			const auto representative_id = representatives[representative_index];
			const auto distance = provider.distance(source_id, representative_id);
			if (distance < best_distance ||
				(distance == best_distance &&
				 representative_id.index() < representatives[best_representative_index].index())) {
				best_representative_index = representative_index;
				best_distance = distance;
			}
		}
		result.assignments.push_back(best_representative_index);
		result.nearest_distances.push_back(best_distance);
	}
	return result;
}

template <typename Space, typename Provider>
auto compress_from_representatives(const Space &space, const Provider &provider,
								   const RepresentativeSet<typename Space::distance_type> &representatives)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using target_space_type = MetricSpace<typename Space::record_type, typename Space::metric_type>;

	auto records = core::records_for_record_ids(space, representatives.representatives);

	const auto use_sampled_assignment =
		!representatives.exact &&
		representatives.representatives.size() > default_approximate_compression_sample_count;
	auto representative_assignment = use_sampled_assignment
		? assign_records_to_sampled_representatives(
			  provider, representatives.representatives, default_approximate_compression_sample_count,
			  "representative set must not be empty", "representative id is outside compression provider")
		: core::assign_records_to_representatives(
			  provider, representatives.representatives, "representative set must not be empty",
			  "representative id is outside compression provider");

	target_space_type compressed_space(std::move(records), space.metric());
	// compress is record-set CARDINALITY reduction: it keeps a coverage-based
	// subset of the original records under the source metric (so the metric law is
	// preserved) and maps every source record to its nearest representative. It is
	// not coordinate/dimension reduction.
	return core::make_compression_result(
		std::move(compressed_space), representatives.representatives,
		std::move(representative_assignment.assignments), std::move(representative_assignment.nearest_distances),
		space.size(), "representatives", representatives.strategy, representatives.representation, representatives.exact,
		true, false, core::metric_traits<typename Space::metric_type>::law,
		use_sampled_assignment
			? "record-set cardinality reduction with bounded sampled representative assignment; kept records are an "
			  "unmodified subset under the source metric; assignment is approximate; not dimension reduction"
			: "record-set cardinality reduction (coverage-based coarsening); kept records are an unmodified subset under "
			  "the source metric; each source record maps to its nearest representative; not dimension reduction");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto identity_compression(const Space &space)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using target_space_type = MetricSpace<typename Space::record_type, typename Space::metric_type>;
	using distance_type = typename Space::distance_type;

	auto ids = std::vector<RecordId>{};
	auto records = std::vector<typename Space::record_type>{};
	auto assignments = std::vector<std::size_t>{};
	auto nearest_distances = std::vector<distance_type>{};
	ids.reserve(space.size());
	records.reserve(space.size());
	assignments.reserve(space.size());
	nearest_distances.reserve(space.size());
	for (std::size_t position = 0; position < space.size(); ++position) {
		const auto id = space.id(position);
		ids.push_back(id);
		records.push_back(space.record(id));
		assignments.push_back(position);
		nearest_distances.push_back(distance_type{});
	}
	target_space_type compressed_space(std::move(records), space.metric());
	return core::make_compression_result(
		std::move(compressed_space), std::move(ids), std::move(assignments), std::move(nearest_distances),
		space.size(), "representatives", "identity", "metric_space", true, false, true,
		core::metric_traits<typename Space::metric_type>::law,
		"identity compression because requested count equals source record count");
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, space::select::farthest_first strategy = {})
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (count == 0) {
		throw std::invalid_argument("compression count must be positive");
	}
	if (count == space.size()) {
		return detail::identity_compression(space);
	}
	if (space.size() > detail::max_default_exact_compression_records) {
		space::storage::LiveDistances<Space> provider(space);
		auto representatives =
			detail::sampled_farthest_first_representatives(provider, count, strategy, space::storage::approximate());
		return detail::compress_from_representatives(space, provider, representatives);
	}

	space::storage::LiveDistances<Space> provider(space);
	const auto representatives = find_representatives(space, count, strategy);
	return detail::compress_from_representatives(space, provider, representatives);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, space::select::coverage strategy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	auto result = compress(space, count, space::select::farthest_first{strategy.seed_index});
	if (result.strategy == "farthest_first") {
		result.strategy = "coverage";
	} else if (result.strategy == "sampled_farthest_first") {
		result.strategy = "sampled_coverage";
	}
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, space::select::k_center strategy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	auto result = compress(space, count, space::select::farthest_first{strategy.seed_index});
	if (result.strategy == "farthest_first") {
		result.strategy = "k_center";
	} else if (result.strategy == "sampled_farthest_first") {
		result.strategy = "sampled_k_center";
	}
	return result;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, space::select::radius_coverage<Radius> strategy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (space.empty()) {
		throw std::invalid_argument("cannot compress an empty metric space");
	}
	space::storage::LiveDistances<Space> provider(space);
	const auto representatives = find_representatives(provider, strategy);
	auto result = detail::compress_from_representatives(space, provider, representatives);
	result.representation = "metric_space";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, stats::structural_analysis::k_medoids_options strategy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (space.empty()) {
		throw std::invalid_argument("cannot compress an empty metric space");
	}
	const auto groups = stats::structural_analysis::find_groups(space, strategy);
	space::storage::LiveDistances<Space> provider(space);
	auto assignment = core::assign_records_to_representatives(provider, groups.medoids,
															  "k-medoids compression requires medoids",
															  "k-medoids medoid id is outside provider");
	auto representatives = core::make_representative_set(
		groups.medoids, std::move(assignment.nearest_distances), space.size(), groups.medoids.size(), "k_medoids",
		groups.representation, groups.algorithm == "kmedoids");
	return detail::compress_from_representatives(space, provider, representatives);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, space::select::farthest_first strategy,
			  space::storage::policy runtime_policy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (count == 0) {
		throw std::invalid_argument("compression count must be positive");
	}

	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.is_approximate()) {
		space::storage::LiveDistances<Space> provider(space);
		auto representatives = detail::sampled_farthest_first_representatives(provider, count, strategy, runtime_policy);
		return detail::compress_from_representatives(space, provider, representatives);
	}

	space::storage::require_exact_compress(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		return space::storage::with_materialized_distance_provider(
			space, runtime_policy, "compress", [&](const auto &provider, const auto &plan) {
				auto representatives = find_representatives(provider, count, strategy);
				representatives.representation = space::storage::materialized_operator_representation(plan);
				return detail::compress_from_representatives(space, provider, representatives);
			});
	}

	auto result = compress(space, count, strategy);
	result.representation = space::storage::compression_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, space::select::coverage strategy,
			  space::storage::policy runtime_policy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	auto result = compress(space, count, space::select::farthest_first{strategy.seed_index}, runtime_policy);
	if (result.strategy == "farthest_first") {
		result.strategy = "coverage";
	} else if (result.strategy == "sampled_farthest_first") {
		result.strategy = "sampled_coverage";
	}
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, space::select::k_center strategy,
			  space::storage::policy runtime_policy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	auto result = compress(space, count, space::select::farthest_first{strategy.seed_index}, runtime_policy);
	if (result.strategy == "farthest_first") {
		result.strategy = "k_center";
	} else if (result.strategy == "sampled_farthest_first") {
		result.strategy = "sampled_k_center";
	}
	return result;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, space::select::radius_coverage<Radius> strategy,
			  space::storage::policy runtime_policy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (space.empty()) {
		throw std::invalid_argument("cannot compress an empty metric space");
	}
	space::storage::require_exact_compress(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		return space::storage::with_materialized_distance_provider(
			space, runtime_policy, "compress", [&](const auto &provider, const auto &plan) {
				auto representatives = find_representatives(provider, strategy);
				representatives.representation = space::storage::materialized_operator_representation(plan);
				return detail::compress_from_representatives(space, provider, representatives);
			});
	}

	auto result = compress(space, strategy);
	result.representation = space::storage::compression_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, stats::structural_analysis::k_medoids_options strategy,
			  space::storage::policy runtime_policy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (space.empty()) {
		throw std::invalid_argument("cannot compress an empty metric space");
	}
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	const auto groups = stats::structural_analysis::find_groups(space, strategy, runtime_policy);
	if (runtime_policy.uses_materialization()) {
		return space::storage::with_materialized_distance_provider(
			space, runtime_policy, "compress", [&](const auto &provider, const auto &plan) {
				auto assignment = core::assign_records_to_representatives(provider, groups.medoids,
																		  "k-medoids compression requires medoids",
																		  "k-medoids medoid id is outside provider");
				auto representatives = core::make_representative_set(
					groups.medoids, std::move(assignment.nearest_distances), space.size(), groups.medoids.size(),
					"k_medoids", space::storage::materialized_operator_representation(plan),
					groups.algorithm == "kmedoids");
				return detail::compress_from_representatives(space, provider, representatives);
			});
	}

	space::storage::LiveDistances<Space> provider(space);
	auto assignment = core::assign_records_to_representatives(provider, groups.medoids,
															  "k-medoids compression requires medoids",
															  "k-medoids medoid id is outside provider");
	auto representatives = core::make_representative_set(
		groups.medoids, std::move(assignment.nearest_distances), space.size(), groups.medoids.size(), "k_medoids",
		space::storage::compression_representation(runtime_policy), groups.algorithm == "kmedoids");
	return detail::compress_from_representatives(space, provider, representatives);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, space::storage::policy runtime_policy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	return compress(space, count, space::select::farthest_first{}, runtime_policy);
}

} // namespace mtrc::modify::reduce

namespace mtrc {
using modify::reduce::compress;
} // namespace mtrc

#endif
