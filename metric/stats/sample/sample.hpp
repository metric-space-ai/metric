// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_SAMPLE_SAMPLE_HPP
#define _METRIC_STATS_SAMPLE_SAMPLE_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/stats/sample/options.hpp>

namespace mtrc::stats::sample {

struct IndexSample {
	std::vector<std::size_t> positions;
	std::size_t record_count{};
	std::size_t requested_count{};
	bool exact{true};
	std::string algorithm{"regular_sample"};

	auto empty() const -> bool { return positions.empty(); }
	auto size() const -> std::size_t { return positions.size(); }
};

template <typename Distance> struct SampleResult {
	using distance_type = Distance;

	std::vector<RecordId> record_ids;
	std::vector<std::size_t> positions;
	std::vector<Distance> nearest_sample_distances;
	std::size_t record_count{};
	std::size_t requested_count{};
	Distance coverage_radius{};
	double average_nearest_distance{};
	bool exact{true};
	std::string algorithm;
	std::string representation;

	auto empty() const -> bool { return record_ids.empty(); }
	auto size() const -> std::size_t { return record_ids.size(); }
	auto begin() const -> typename std::vector<RecordId>::const_iterator { return record_ids.begin(); }
	auto end() const -> typename std::vector<RecordId>::const_iterator { return record_ids.end(); }
	auto operator[](std::size_t index) const -> RecordId { return record_ids[index]; }
};

inline auto regular_sample_positions(std::size_t record_count, std::size_t count, std::size_t offset = 0)
	-> IndexSample
{
	if (count == 0) {
		return IndexSample{{}, record_count, count};
	}
	if (record_count == 0) {
		throw std::invalid_argument("cannot sample a non-empty set from an empty record set");
	}
	if (count > record_count) {
		throw std::invalid_argument("sample count cannot exceed record_count");
	}
	if (offset >= record_count) {
		throw std::out_of_range("sample offset is outside the record set");
	}

	std::vector<std::size_t> positions;
	positions.reserve(count);
	for (std::size_t sample_index = 0; sample_index < count; ++sample_index) {
		positions.push_back(((sample_index * record_count) / count + offset) % record_count);
	}

	return IndexSample{std::move(positions), record_count, count};
}

namespace sample_detail {

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto make_sample_result(const Provider &provider, std::vector<RecordId> record_ids, std::size_t requested_count,
						std::string algorithm) -> SampleResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	SampleResult<distance_type> result;
	result.record_count = provider.record_count();
	result.requested_count = requested_count;
	result.algorithm = std::move(algorithm);
	result.representation = "pairwise_distances";
	result.record_ids = std::move(record_ids);
	result.positions.reserve(result.record_ids.size());
	for (const auto id : result.record_ids) {
		result.positions.push_back(provider.position_of(id));
	}

	if (result.record_ids.empty()) {
		return result;
	}

	const auto assignment = core::assign_records_to_representatives(
		provider, result.record_ids, "sample set must not be empty", "sample id is outside provider");
	result.nearest_sample_distances = std::move(assignment.nearest_distances);
	for (const auto distance : result.nearest_sample_distances) {
		if (result.coverage_radius < distance) {
			result.coverage_radius = distance;
		}
		result.average_nearest_distance += static_cast<double>(distance);
	}
	if (!result.nearest_sample_distances.empty()) {
		result.average_nearest_distance /=
			static_cast<double>(result.nearest_sample_distances.size());
	}

	return result;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto nearest_unselected_position(const Provider &provider, RecordId current_id, const std::vector<bool> &selected)
	-> std::size_t
{
	if (selected.size() != provider.record_count()) {
		throw std::invalid_argument("selected-record count does not match provider");
	}

	std::size_t best_position = selected.size();
	typename Provider::distance_type best_distance{};
	bool has_best = false;
	for (std::size_t position = 0; position < selected.size(); ++position) {
		if (selected[position]) {
			continue;
		}
		const auto candidate_id = provider.id(position);
		const auto distance = provider.distance(current_id, candidate_id);
		if (!has_best || distance < best_distance ||
			(distance == best_distance && candidate_id < provider.id(best_position))) {
			best_position = position;
			best_distance = distance;
			has_best = true;
		}
	}

	if (!has_best) {
		throw std::logic_error("failed to select the next metric-walk sample");
	}
	return best_position;
}

} // namespace sample_detail

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto farthest_first(const Provider &provider, std::size_t count, farthest_first_options options = {})
	-> SampleResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (count == 0) {
		return sample_detail::make_sample_result(provider, std::vector<RecordId>{}, count, "farthest_first");
	}
	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot sample from an empty distance provider");
	}
	if (count > provider.record_count()) {
		throw std::invalid_argument("sample count cannot exceed record_count");
	}
	if (options.seed_index >= provider.record_count()) {
		throw std::out_of_range("seed_index is outside the distance provider");
	}

	std::vector<RecordId> selected_ids;
	selected_ids.reserve(count);
	selected_ids.push_back(provider.id(options.seed_index));

	std::vector<bool> selected(provider.record_count(), false);
	selected[options.seed_index] = true;

	auto nearest_selected_distances = core::distances_to_record_id(
		provider, selected_ids.front(), "sample id is outside provider");

	while (selected_ids.size() < count) {
		const auto next_position = core::farthest_unselected_record_position(
			provider, nearest_selected_distances, selected,
			"selected-record count does not match distance count", "failed to select the next sample");
		const auto next_id = provider.id(next_position);
		selected_ids.push_back(next_id);
		selected[next_position] = true;
		core::update_min_distances_to_record_id(
			provider, nearest_selected_distances, next_id,
			"nearest sample distance count does not match provider", "sample id is outside provider");
	}

	(void)sizeof(distance_type);
	return sample_detail::make_sample_result(provider, std::move(selected_ids), count, "farthest_first");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto farthest_first(const Space &space, std::size_t count, farthest_first_options options = {})
	-> SampleResult<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = farthest_first(provider, count, options);
	result.representation = "metric_space";
	return result;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto metric_walk(const Provider &provider, std::size_t count, metric_walk_options options = {})
	-> SampleResult<typename Provider::distance_type>
{
	if (count == 0) {
		return sample_detail::make_sample_result(provider, std::vector<RecordId>{}, count, "metric_walk");
	}
	if (provider.record_count() == 0) {
		throw std::invalid_argument("cannot sample from an empty distance provider");
	}
	if (count > provider.record_count()) {
		throw std::invalid_argument("sample count cannot exceed record_count");
	}
	if (options.seed_index >= provider.record_count()) {
		throw std::out_of_range("seed_index is outside the distance provider");
	}

	std::vector<RecordId> selected_ids;
	selected_ids.reserve(count);
	std::vector<bool> selected(provider.record_count(), false);

	auto current_position = options.seed_index;
	while (selected_ids.size() < count) {
		const auto current_id = provider.id(current_position);
		selected_ids.push_back(current_id);
		selected[current_position] = true;
		if (selected_ids.size() == count) {
			break;
		}
		current_position = sample_detail::nearest_unselected_position(provider, current_id, selected);
	}

	return sample_detail::make_sample_result(provider, std::move(selected_ids), count, "metric_walk");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto metric_walk(const Space &space, std::size_t count, metric_walk_options options = {})
	-> SampleResult<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = metric_walk(provider, count, options);
	result.representation = "metric_space";
	return result;
}

} // namespace mtrc::stats::sample

#endif
