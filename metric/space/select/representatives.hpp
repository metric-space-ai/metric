// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_SELECT_REPRESENTATIVES_HPP
#define _METRIC_SPACE_SELECT_REPRESENTATIVES_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/space/select/options.hpp>

namespace mtrc::space::select {

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto find_representatives(const Provider &provider, std::size_t count, space::select::farthest_first strategy = {})
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

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, space::select::farthest_first strategy = {})
	-> RepresentativeSet<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = find_representatives(provider, count, strategy);
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
	result.representation = "records";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto find_representatives(const Space &space, std::size_t count, space::select::farthest_first strategy,
						  space::storage::policy runtime_policy) -> RepresentativeSet<typename Space::distance_type>
{
	space::storage::require_exact_representatives(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		space::storage::DistanceTable<Space> matrix(space);
		auto result = find_representatives(matrix, count, strategy);
		result.representation = space::storage::representative_representation(runtime_policy);
		return result;
	}

	auto result = find_representatives(space, count, strategy);
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
	result.representation = space::storage::representative_representation(runtime_policy);
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
