// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_SELECT_REPRESENTATIVES_HPP
#define _METRIC_SPACE_SELECT_REPRESENTATIVES_HPP

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/space/chunked.hpp>
#include <metric/space/sample_plan.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/space/select/options.hpp>

namespace mtrc::space::select {

struct exact_representative_work_options {
	// Maximum direct provider distance lookups for exact representative helpers.
	// Set to 0 only when the caller intentionally opts into unbounded exact work.
	std::size_t max_distance_evaluations{1'000'000};
};

namespace representative_detail {

constexpr std::size_t default_approximate_representative_sample_count = 512;
constexpr std::size_t max_default_exact_representative_records = 4096;

inline auto relabel_representative_strategy(std::string &strategy, const std::string &exact_name,
											const std::string &sampled_name,
											const std::string &chunked_name) -> void
{
	if (strategy == "farthest_first") {
		strategy = exact_name;
	} else if (strategy == "sampled_farthest_first" || strategy == "sampled_regular") {
		strategy = sampled_name;
	} else if (strategy == "chunked_farthest_first") {
		strategy = chunked_name;
	}
}

inline auto approximate_representative_sample_count(std::size_t record_count,
													storage::policy runtime_policy) -> std::size_t
{
	if (record_count == 0) {
		return 0;
	}
	if (runtime_policy.graph_neighbors() != 0) {
		return std::min(runtime_policy.graph_neighbors(), record_count);
	}
	return std::min(default_approximate_representative_sample_count, record_count);
}

inline auto contains_candidate_record_id(const std::vector<RecordId> &ids, RecordId id) -> bool
{
	return std::find(ids.begin(), ids.end(), id) != ids.end();
}

inline auto checked_representative_work_product(std::size_t lhs, std::size_t rhs, const char *message)
	-> std::size_t
{
	if (rhs != 0 && lhs > std::numeric_limits<std::size_t>::max() / rhs) {
		throw ::mtrc::RepresentationError(message);
	}
	return lhs * rhs;
}

inline auto checked_representative_work_sum(std::size_t lhs, std::size_t rhs, const char *message)
	-> std::size_t
{
	if (lhs > std::numeric_limits<std::size_t>::max() - rhs) {
		throw ::mtrc::RepresentationError(message);
	}
	return lhs + rhs;
}

inline auto require_exact_provider_representative_work(std::size_t estimated_distance_evaluations,
													   exact_representative_work_options options,
													   const char *operation) -> void
{
	if (options.max_distance_evaluations == 0 ||
		estimated_distance_evaluations <= options.max_distance_evaluations) {
		return;
	}
	throw ::mtrc::RepresentationError(
		std::string(operation) +
		" refused exact representative provider work before distance calls: estimated_distance_evaluations=" +
		std::to_string(estimated_distance_evaluations) +
		", max_distance_evaluations=" + std::to_string(options.max_distance_evaluations) +
		". Pass exact_representative_work_options{0} only when unbounded exact provider work is intentional.");
}

inline auto farthest_first_provider_work(std::size_t record_count, std::size_t requested_count,
										 const char *operation) -> std::size_t
{
	return checked_representative_work_product(
		record_count, requested_count,
		(std::string(operation) + " distance-evaluation estimate exceeds size_t capacity").c_str());
}

inline auto radius_coverage_provider_work(std::size_t record_count, const char *operation) -> std::size_t
{
	const auto dense_scan = checked_representative_work_product(
		record_count, record_count,
		(std::string(operation) + " distance-evaluation estimate exceeds size_t capacity").c_str());
	return checked_representative_work_sum(
		dense_scan, dense_scan,
		(std::string(operation) + " distance-evaluation estimate exceeds size_t capacity").c_str());
}

inline auto unbounded_exact_representative_work() -> exact_representative_work_options
{
	return exact_representative_work_options{0};
}

inline auto exact_representative_work_for_policy(storage::policy runtime_policy) -> exact_representative_work_options
{
	return exact_representative_work_options{runtime_policy.max_distance_evaluations()};
}

template <typename Provider>
auto sampled_regular_representatives(const Provider &provider, std::size_t count,
									 farthest_first strategy)
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
	std::unordered_set<RecordId> representative_ids;
	representative_ids.reserve(count);
	auto add_position = [&provider, &representatives, &representative_ids, count](std::size_t position) {
		const auto id = provider.id(position);
		if (representative_ids.insert(id).second) {
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
auto farthest_first_from_candidates(const Provider &provider, std::vector<RecordId> candidate_ids,
									std::size_t count, farthest_first strategy,
									std::string strategy_name, std::string representation)
	-> RepresentativeSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (count == 0) {
		return core::make_representative_set(std::vector<RecordId>{}, std::vector<distance_type>{},
											 provider.record_count(), count, std::move(strategy_name),
											 std::move(representation), false);
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
	const auto seed_id = provider.id(strategy.seed_index);
	std::unordered_set<RecordId> candidate_set(candidate_ids.begin(), candidate_ids.end());
	candidate_set.reserve(std::max(candidate_ids.size(), count));
	if (candidate_set.insert(seed_id).second) {
		candidate_ids.insert(candidate_ids.begin(), seed_id);
	}
	for (std::size_t position = 0; candidate_ids.size() < count && position < provider.record_count(); ++position) {
		const auto id = provider.id(position);
		if (candidate_set.insert(id).second) {
			candidate_ids.push_back(id);
		}
	}
	if (count > candidate_ids.size()) {
		return sampled_regular_representatives(provider, count, strategy);
	}

	std::vector<RecordId> representatives;
	representatives.reserve(count);
	representatives.push_back(seed_id);
	std::vector<bool> selected(candidate_ids.size(), false);
	const auto seed_position = static_cast<std::size_t>(
		std::find(candidate_ids.begin(), candidate_ids.end(), seed_id) - candidate_ids.begin());
	selected[seed_position] = true;

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

	return core::make_representative_set(std::move(representatives), std::vector<distance_type>{},
										 provider.record_count(), count, std::move(strategy_name),
										 std::move(representation), false);
}

template <typename Provider>
auto sampled_farthest_first_representatives(const Provider &provider, std::size_t count,
											farthest_first strategy, storage::policy runtime_policy)
	-> RepresentativeSet<typename Provider::distance_type>
{
	const auto sample_count = approximate_representative_sample_count(provider.record_count(), runtime_policy);
	if (count > sample_count) {
		return sampled_regular_representatives(provider, count, strategy);
	}
	auto candidate_ids = ::mtrc::space::record_ids_for_sample_plan(
		provider, ::mtrc::space::regular_sample_positions(provider.record_count(), sample_count));
	return farthest_first_from_candidates(
		provider, std::move(candidate_ids), count, strategy, "sampled_farthest_first", "sampled_metric_space");
}

template <typename Space>
auto chunked_farthest_first_representatives(const Space &space, std::size_t count,
											farthest_first strategy,
											const storage::execution_plan &plan)
	-> RepresentativeSet<typename Space::distance_type>
{
	storage::LiveDistances<Space> provider(space);
	const auto chunk_size = plan.chunk_size == 0 ? std::size_t{1} : plan.chunk_size;
	const auto chunks = ::mtrc::space::chunked_view(space, chunk_size);
	auto candidate_ids = chunks.representative_ids();
	const auto candidate_limit =
		std::min(space.size(), std::max(candidate_ids.size() + std::size_t{1},
										default_approximate_representative_sample_count));
	if (count > candidate_limit) {
		return sampled_regular_representatives(provider, count, strategy);
	}
	const auto candidate_plan = ::mtrc::space::regular_sample_positions(space.size(), candidate_limit);
	std::unordered_set<RecordId> candidate_set(candidate_ids.begin(), candidate_ids.end());
	candidate_set.reserve(candidate_limit);
	for (const auto position : candidate_plan.positions) {
		const auto id = space.id(position);
		if (candidate_set.insert(id).second) {
			candidate_ids.push_back(id);
		}
		if (candidate_ids.size() == candidate_limit) {
			break;
		}
	}
	return farthest_first_from_candidates(
		provider, std::move(candidate_ids), count, strategy, "chunked_farthest_first", "chunked_space_view");
}

template <typename Provider, typename Radius>
auto radius_coverage_from_candidates(const Provider &provider, const std::vector<RecordId> &candidate_ids,
									 Radius radius, std::string strategy, std::string representation)
	-> RepresentativeSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (radius < Radius{}) {
		throw std::invalid_argument("coverage radius must be non-negative");
	}
	if (candidate_ids.empty()) {
		return core::make_representative_set(std::vector<RecordId>{}, std::vector<distance_type>{},
											 provider.record_count(), 0, std::move(strategy),
											 std::move(representation), false);
	}

	const auto threshold = static_cast<distance_type>(radius);
	std::vector<RecordId> representatives;
	std::vector<bool> covered(candidate_ids.size(), false);
	std::size_t covered_count = 0;
	while (covered_count < candidate_ids.size()) {
		auto seed_index = std::size_t{0};
		while (seed_index < covered.size() && covered[seed_index]) {
			++seed_index;
		}
		if (seed_index == covered.size()) {
			break;
		}
		const auto seed_id = candidate_ids[seed_index];
		representatives.push_back(seed_id);
		for (std::size_t candidate_index = 0; candidate_index < candidate_ids.size(); ++candidate_index) {
			if (covered[candidate_index]) {
				continue;
			}
			if (provider.distance(seed_id, candidate_ids[candidate_index]) <= threshold) {
				covered[candidate_index] = true;
				++covered_count;
			}
		}
	}

	const auto representative_count = representatives.size();
	return core::make_representative_set(
		std::move(representatives), std::vector<distance_type>{}, provider.record_count(), representative_count,
		std::move(strategy), std::move(representation), false);
}

template <typename Space, typename Radius>
auto sampled_radius_coverage_representatives(const Space &space, radius_coverage<Radius> strategy,
											 storage::policy runtime_policy)
	-> RepresentativeSet<typename Space::distance_type>
{
	storage::LiveDistances<Space> provider(space);
	const auto sample_count = approximate_representative_sample_count(provider.record_count(), runtime_policy);
	const auto candidate_ids = ::mtrc::space::record_ids_for_sample_plan(
		provider, ::mtrc::space::regular_sample_positions(provider.record_count(), sample_count));
	return radius_coverage_from_candidates(
		provider, candidate_ids, strategy.radius, "sampled_radius_coverage", "sampled_metric_space");
}

template <typename Space, typename Radius>
auto chunked_radius_coverage_representatives(const Space &space, radius_coverage<Radius> strategy,
											 const storage::execution_plan &plan)
	-> RepresentativeSet<typename Space::distance_type>
{
	storage::LiveDistances<Space> provider(space);
	const auto chunk_size = plan.chunk_size == 0 ? std::size_t{1} : plan.chunk_size;
	const auto chunks = ::mtrc::space::chunked_view(space, chunk_size);
	return radius_coverage_from_candidates(
		provider, chunks.representative_ids(), strategy.radius, "chunked_radius_coverage", "chunked_space_view");
}

inline auto throw_representative_plan_refusal(const storage::execution_plan &plan) -> void
{
	auto message = plan.reason.empty() ? std::string("representative runtime policy refused by resource budget")
									   : plan.reason;
	if (!plan.fallback_hint.empty()) {
		message += "; fallback: " + plan.fallback_hint;
	}
	throw ::mtrc::RepresentationError(message);
}

} // namespace representative_detail

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_representatives(const Provider &provider, std::size_t count, space::select::farthest_first strategy = {},
						  exact_representative_work_options options = {})
	-> RepresentativeSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (count == 0) {
		return core::make_representative_set(std::vector<RecordId>{}, std::vector<distance_type>{},
											 provider.record_count(), count, "farthest_first", "pairwise_distances");
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
	representative_detail::require_exact_provider_representative_work(
		representative_detail::farthest_first_provider_work(provider.record_count(), count, "find_representatives"),
		options, "find_representatives");

	std::vector<RecordId> representatives;
	representatives.push_back(provider.id(strategy.seed_index));

	std::vector<bool> is_selected(provider.record_count(), false);
	is_selected[strategy.seed_index] = true;

	auto nearest_representative_distances =
		core::distances_to_record_id(provider, representatives.front(), "representative id is outside provider");

	while (representatives.size() < count) {
		const auto next_index = core::farthest_unselected_position(
			nearest_representative_distances, is_selected, "selected-record count does not match distance count",
			"failed to select the next representative");
		const auto next_id = provider.id(next_index);
		representatives.push_back(next_id);
		is_selected[next_index] = true;
		core::update_min_distances_to_record_id(provider, nearest_representative_distances, next_id,
												"nearest representative distance count does not match provider",
												"representative id is outside provider");
	}

	return core::make_representative_set(std::move(representatives), std::move(nearest_representative_distances),
										 provider.record_count(), count, "farthest_first", "pairwise_distances");
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_representatives(const Provider &provider, std::size_t count, space::select::coverage strategy,
						  exact_representative_work_options options = {})
	-> RepresentativeSet<typename Provider::distance_type>
{
	auto result = find_representatives(provider, count, space::select::farthest_first{strategy.seed_index}, options);
	result.strategy = "coverage";
	return result;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_representatives(const Provider &provider, std::size_t count, space::select::k_center strategy,
						  exact_representative_work_options options = {})
	-> RepresentativeSet<typename Provider::distance_type>
{
	auto result = find_representatives(provider, count, space::select::farthest_first{strategy.seed_index}, options);
	result.strategy = "k_center";
	return result;
}

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_representatives(const Provider &provider, space::select::radius_coverage<Radius> strategy,
						  exact_representative_work_options options = {})
	-> RepresentativeSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (strategy.radius < Radius{}) {
		throw std::invalid_argument("coverage radius must be non-negative");
	}
	if (provider.record_count() == 0) {
		return core::make_representative_set(std::vector<RecordId>{}, std::vector<distance_type>{}, 0, 0,
											 "radius_coverage", "pairwise_distances");
	}
	representative_detail::require_exact_provider_representative_work(
		representative_detail::radius_coverage_provider_work(provider.record_count(), "radius_coverage"),
		options, "radius_coverage");

	std::vector<RecordId> representatives;
	std::vector<bool> covered(provider.record_count(), false);
	std::size_t covered_count = 0;
	const auto radius = static_cast<distance_type>(strategy.radius);

	while (covered_count < provider.record_count()) {
		const auto seed_index = core::first_unmarked_position(covered, "failed to select the next coverage seed");
		const auto seed_id = provider.id(seed_index);
		representatives.push_back(seed_id);
		covered_count += core::mark_records_within_radius(
			provider, seed_id, radius, covered, "coverage state count does not match provider",
			"coverage representative id is outside provider");
	}

	auto nearest_representative_distances =
		core::distances_to_record_id(provider, representatives.front(), "representative id is outside provider");
	for (std::size_t representative_index = 1; representative_index < representatives.size(); ++representative_index) {
		core::update_min_distances_to_record_id(provider, nearest_representative_distances,
												representatives[representative_index],
												"nearest representative distance count does not match provider",
												"representative id is outside provider");
	}

	const auto representative_count = representatives.size();
	return core::make_representative_set(std::move(representatives), std::move(nearest_representative_distances),
										 provider.record_count(), representative_count, "radius_coverage",
										 "pairwise_distances");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, space::select::farthest_first strategy = {})
	-> RepresentativeSet<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	if (space.size() > representative_detail::max_default_exact_representative_records && count > 0) {
		return representative_detail::sampled_farthest_first_representatives(
			provider, count, strategy, space::storage::approximate());
	}
	auto result = find_representatives(provider, count, strategy);
	result.representation = "metric_space";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, space::select::coverage strategy)
	-> RepresentativeSet<typename Space::distance_type>
{
	auto result = find_representatives(space, count, space::select::farthest_first{strategy.seed_index});
	representative_detail::relabel_representative_strategy(
		result.strategy, "coverage", "sampled_coverage", "chunked_coverage");
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, space::select::k_center strategy)
	-> RepresentativeSet<typename Space::distance_type>
{
	auto result = find_representatives(space, count, space::select::farthest_first{strategy.seed_index});
	representative_detail::relabel_representative_strategy(
		result.strategy, "k_center", "sampled_k_center", "chunked_k_center");
	return result;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, space::select::radius_coverage<Radius> strategy)
	-> RepresentativeSet<typename Space::distance_type>
{
	if (space.size() > representative_detail::max_default_exact_representative_records) {
		return representative_detail::sampled_radius_coverage_representatives(
			space, strategy, space::storage::approximate());
	}
	space::storage::LiveDistances<Space> provider(space);
	auto result = find_representatives(provider, strategy);
	result.representation = "metric_space";
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_representatives(const Container &records, const Metric &metric, std::size_t count,
						  space::select::farthest_first strategy = {})
	-> RepresentativeSet<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = find_representatives(space, count, strategy);
	if (result.exact) {
		result.representation = "records";
	}
	return result;
}

template <typename Container, typename Metric, typename Radius,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_representatives(const Container &records, const Metric &metric, space::select::radius_coverage<Radius> strategy)
	-> RepresentativeSet<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = find_representatives(space, strategy);
	if (result.exact) {
		result.representation = "records";
	}
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, space::select::farthest_first strategy,
						  space::storage::policy runtime_policy) -> RepresentativeSet<typename Space::distance_type>
{
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.is_approximate()) {
		space::storage::LiveDistances<Space> provider(space);
		return representative_detail::sampled_farthest_first_representatives(provider, count, strategy, runtime_policy);
	}

		auto plan = space::storage::estimate_cost(space, "representatives", runtime_policy, count);
		if (!plan.refused && runtime_policy.uses_materialization() &&
			!space::storage::uses_distance_table_materialization(runtime_policy)) {
			plan = space::storage::estimate_cost(
				space, "representatives", space::storage::using_distance_table(runtime_policy), count);
		}
	if (plan.refused) {
		representative_detail::throw_representative_plan_refusal(plan);
	}
	if (plan.allowed && plan.downgraded && !plan.exact && plan.representation == "chunked_space_view") {
		return representative_detail::chunked_farthest_first_representatives(space, count, strategy, plan);
	}
	if (plan.allowed && plan.downgraded && !plan.exact && plan.representation == "sampled_metric_space") {
		space::storage::LiveDistances<Space> provider(space);
		return representative_detail::sampled_farthest_first_representatives(provider, count, strategy, runtime_policy);
	}

		if (runtime_policy.uses_materialization()) {
			return space::storage::with_materialized_distance_provider(
				space, runtime_policy, "representatives", [count, strategy](const auto &provider, const auto &plan) {
					auto result = find_representatives(
						provider, count, strategy, representative_detail::unbounded_exact_representative_work());
					result.representation = space::storage::materialized_operator_representation(plan);
					return result;
				});
		}

		space::storage::LiveDistances<Space> provider(space);
		auto result = find_representatives(
			provider, count, strategy, representative_detail::exact_representative_work_for_policy(runtime_policy));
		result.representation = space::storage::representative_representation(runtime_policy);
		return result;
	}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, space::select::coverage strategy,
						  space::storage::policy runtime_policy) -> RepresentativeSet<typename Space::distance_type>
{
	auto result =
		find_representatives(space, count, space::select::farthest_first{strategy.seed_index}, runtime_policy);
	representative_detail::relabel_representative_strategy(
		result.strategy, "coverage", "sampled_coverage", "chunked_coverage");
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, space::select::k_center strategy,
						  space::storage::policy runtime_policy) -> RepresentativeSet<typename Space::distance_type>
{
	auto result =
		find_representatives(space, count, space::select::farthest_first{strategy.seed_index}, runtime_policy);
	representative_detail::relabel_representative_strategy(
		result.strategy, "k_center", "sampled_k_center", "chunked_k_center");
	return result;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, space::select::radius_coverage<Radius> strategy,
						  space::storage::policy runtime_policy) -> RepresentativeSet<typename Space::distance_type>
{
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.is_approximate()) {
		return representative_detail::sampled_radius_coverage_representatives(space, strategy, runtime_policy);
	}

		auto plan = space::storage::estimate_cost(space, "representatives", runtime_policy, space.size());
		if (!plan.refused && runtime_policy.uses_materialization() &&
			!space::storage::uses_distance_table_materialization(runtime_policy)) {
			plan = space::storage::estimate_cost(
				space, "representatives", space::storage::using_distance_table(runtime_policy), space.size());
		}
	if (plan.refused) {
		representative_detail::throw_representative_plan_refusal(plan);
	}
	if (plan.allowed && plan.downgraded && !plan.exact && plan.representation == "chunked_space_view") {
		return representative_detail::chunked_radius_coverage_representatives(space, strategy, plan);
	}
	if (plan.allowed && plan.downgraded && !plan.exact && plan.representation == "sampled_metric_space") {
		return representative_detail::sampled_radius_coverage_representatives(space, strategy, runtime_policy);
	}

		if (runtime_policy.uses_materialization()) {
			return space::storage::with_materialized_distance_provider(
				space, runtime_policy, "representatives", [strategy](const auto &provider, const auto &plan) {
					auto result = find_representatives(
						provider, strategy, representative_detail::unbounded_exact_representative_work());
					result.representation = space::storage::materialized_operator_representation(plan);
					return result;
				});
		}

		space::storage::LiveDistances<Space> provider(space);
		auto result = find_representatives(
			provider, strategy, representative_detail::exact_representative_work_for_policy(runtime_policy));
		result.representation = space::storage::representative_representation(runtime_policy);
		return result;
	}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, space::storage::policy runtime_policy)
	-> RepresentativeSet<typename Space::distance_type>
{
	return find_representatives(space, count, space::select::farthest_first{}, runtime_policy);
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_representatives(const Container &records, const Metric &metric, std::size_t count,
						  space::select::farthest_first strategy, space::storage::policy runtime_policy)
	-> RepresentativeSet<metric_result_t<Metric, Record>>
{
	auto space = core::make_space(records, metric);
	auto result = find_representatives(space, count, strategy, runtime_policy);
	if (result.exact) {
		result.representation = space::storage::representative_representation(runtime_policy);
	}
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto find_representatives(const Container &records, const Metric &metric, std::size_t count,
						  space::storage::policy runtime_policy)
	-> RepresentativeSet<metric_result_t<Metric, Record>>
{
	return find_representatives(records, metric, count, space::select::farthest_first{}, runtime_policy);
}

} // namespace mtrc::space::select

namespace mtrc {
using space::select::find_representatives;
} // namespace mtrc

#endif
