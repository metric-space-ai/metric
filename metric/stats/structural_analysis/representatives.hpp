// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_STRUCTURAL_ANALYSIS_REPRESENTATIVES_HPP
#define _METRIC_STATS_STRUCTURAL_ANALYSIS_REPRESENTATIVES_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/stats/sample/sample.hpp>

namespace mtrc::stats::structural_analysis {

namespace representatives_detail {

template <typename Container> using record_type_t = typename std::decay<typename Container::value_type>::type;

} // namespace representatives_detail

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto medoid_id(const Provider &provider) -> RecordId
{
	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot select a medoid from an empty distance provider");
	}

	std::vector<RecordId> ids;
	ids.reserve(provider.record_count());
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		ids.push_back(provider.id(index));
	}

	return core::minimum_total_distance_record_id(provider, ids,
												  "cannot select a medoid from an empty distance provider",
												  "candidate id is outside the distance provider");
}

template <typename Container, typename Metric,
		  typename Record = typename representatives_detail::record_type_t<Container>,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto representative_indices(const Container &records, Metric metric, std::size_t count, std::size_t seed_index = 0)
	-> std::vector<std::size_t>
{
	auto space = mtrc::make_space(records, std::move(metric));
	const auto sample =
		stats::sample::farthest_first(space, count, stats::sample::farthest_first_options(seed_index));
	return sample.positions;
}

template <typename Container, typename Metric,
		  typename Record = typename representatives_detail::record_type_t<Container>,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto representatives(const Container &records, Metric metric, std::size_t count, std::size_t seed_index = 0)
	-> std::vector<Record>
{
	const auto selected = representative_indices(records, std::move(metric), count, seed_index);
	std::vector<Record> result;
	result.reserve(selected.size());
	for (const auto index : selected) {
		result.push_back(records[index]);
	}
	return result;
}

template <typename Container, typename Metric,
		  typename Record = typename representatives_detail::record_type_t<Container>,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto medoid_index(const Container &records, Metric metric) -> std::size_t
{
	auto space = mtrc::make_space(records, std::move(metric));
	space::storage::LiveDistances<decltype(space)> provider(space);
	return provider.position_of(medoid_id(provider));
}

template <typename Container, typename Metric,
		  typename Record = typename representatives_detail::record_type_t<Container>,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto medoid(const Container &records, Metric metric) -> Record
{
	return records[medoid_index(records, std::move(metric))];
}

template <typename Container, typename Metric, typename MinimumDistance,
		  typename Record = typename representatives_detail::record_type_t<Container>,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto separated_representative_indices(const Container &records, Metric metric, MinimumDistance minimum_distance)
	-> std::vector<std::size_t>
{
	using distance_type = metric_result_t<Metric, Record>;
	using comparison_type = typename std::common_type<distance_type, MinimumDistance>::type;

	if (minimum_distance < MinimumDistance{}) {
		throw std::invalid_argument("minimum_distance must be non-negative");
	}
	if (records.empty()) {
		return {};
	}

	auto space = mtrc::make_space(records, std::move(metric));
	space::storage::LiveDistances<decltype(space)> provider(space);
	const auto threshold = static_cast<comparison_type>(minimum_distance);

	std::vector<std::size_t> selected;
	std::vector<RecordId> selected_ids;
	for (std::size_t candidate_index = 0; candidate_index < provider.record_count(); ++candidate_index) {
		const auto candidate_id = provider.id(candidate_index);
		if (core::record_is_separated_from_record_ids(
				provider, candidate_id, selected_ids, threshold, "candidate id is outside the record set",
				"selected representative id is outside the record set")) {
			selected.push_back(candidate_index);
			selected_ids.push_back(candidate_id);
		}
	}

	return selected;
}

template <typename Container, typename Metric, typename MinimumDistance,
		  typename Record = typename representatives_detail::record_type_t<Container>,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto separated_representatives(const Container &records, Metric metric, MinimumDistance minimum_distance)
	-> std::vector<Record>
{
	const auto selected = separated_representative_indices(records, std::move(metric), minimum_distance);
	std::vector<Record> result;
	result.reserve(selected.size());
	for (const auto index : selected) {
		result.push_back(records[index]);
	}
	return result;
}

template <typename Container, typename Metric, typename Radius,
		  typename Record = typename representatives_detail::record_type_t<Container>,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto coverage_representative_indices(const Container &records, Metric metric, Radius radius)
	-> std::vector<std::size_t>
{
	using distance_type = metric_result_t<Metric, Record>;

	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}
	if (records.empty()) {
		return {};
	}

	auto space = mtrc::make_space(records, std::move(metric));
	space::storage::LiveDistances<decltype(space)> provider(space);
	const auto cover_radius = static_cast<distance_type>(radius);

	std::vector<std::size_t> selected;
	std::vector<bool> covered(records.size(), false);
	std::size_t covered_count = 0;

	while (covered_count < records.size()) {
		const auto seed_index =
			core::first_unmarked_position(covered, "failed to select the next coverage representative");
		selected.push_back(seed_index);
		covered_count += core::mark_records_within_radius(
			provider, provider.id(seed_index), cover_radius, covered,
			"coverage state count does not match record count", "coverage representative id is outside the record set");
	}

	return selected;
}

template <typename Container, typename Metric, typename Radius,
		  typename Record = typename representatives_detail::record_type_t<Container>,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto coverage_representatives(const Container &records, Metric metric, Radius radius) -> std::vector<Record>
{
	const auto selected = coverage_representative_indices(records, std::move(metric), radius);
	std::vector<Record> result;
	result.reserve(selected.size());
	for (const auto index : selected) {
		result.push_back(records[index]);
	}
	return result;
}

} // namespace mtrc::stats::structural_analysis

#endif
