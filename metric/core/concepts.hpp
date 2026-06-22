// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_CONCEPTS_HPP
#define _METRIC_CORE_CONCEPTS_HPP

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/record/ids.hpp>

#include "metric_traits.hpp"

namespace mtrc::core {

template <typename Metric, typename Record, typename = void> struct MetricCallable : std::false_type {};

template <typename Metric, typename Record>
struct MetricCallable<Metric, Record,
					  std::void_t<decltype(std::declval<const Metric &>()(
						  std::declval<const Record &>(), std::declval<const Record &>()))>> : std::true_type {};

template <typename Metric, typename Record> constexpr bool MetricCallable_v = MetricCallable<Metric, Record>::value;

template <typename Metric, typename Record>
using metric_result_t =
	decltype(std::declval<const Metric &>()(std::declval<const Record &>(), std::declval<const Record &>()));

template <typename Space, typename = void> struct MetricSpaceLike : std::false_type {};

template <typename Space>
struct MetricSpaceLike<Space, std::void_t<typename Space::record_type, typename Space::metric_type,
										  typename Space::distance_type, decltype(std::declval<const Space &>().size()),
										  decltype(std::declval<const Space &>().version())>> : std::true_type {};

template <typename Space> constexpr bool MetricSpaceLike_v = MetricSpaceLike<Space>::value;

template <typename Source, typename = void> struct IndexedRecordIdSource : std::false_type {};

template <typename Source>
struct IndexedRecordIdSource<Source,
							 std::void_t<decltype(std::declval<const Source &>().id(std::declval<std::size_t>()))>>
	: std::is_convertible<decltype(std::declval<const Source &>().id(std::declval<std::size_t>())), RecordId> {};

template <typename Source> constexpr bool IndexedRecordIdSource_v = IndexedRecordIdSource<Source>::value;

struct RecallAccumulator {
	std::size_t matched{};
	std::size_t possible{};
	std::size_t observations{};
	double recall_sum{};
	double minimum_recall{std::numeric_limits<double>::max()};
	double maximum_recall{};

	auto add(std::size_t matched_count, std::size_t possible_count, double empty_observation_recall = 0.0) -> double
	{
		matched += matched_count;
		possible += possible_count;
		++observations;

		const auto observation_recall =
			possible_count == 0 ? empty_observation_recall
								: static_cast<double>(matched_count) / static_cast<double>(possible_count);
		recall_sum += observation_recall;
		if (observation_recall < minimum_recall) {
			minimum_recall = observation_recall;
		}
		if (observation_recall > maximum_recall) {
			maximum_recall = observation_recall;
		}
		return observation_recall;
	}

	auto total_recall(double empty_recall = 0.0) const -> double
	{
		if (possible == 0) {
			return empty_recall;
		}
		return static_cast<double>(matched) / static_cast<double>(possible);
	}

	auto average_recall(double empty_recall = 0.0) const -> double
	{
		if (observations == 0) {
			return empty_recall;
		}
		return recall_sum / static_cast<double>(observations);
	}

	auto minimum_recall_or(double empty_recall = 0.0) const -> double
	{
		return observations == 0 ? empty_recall : minimum_recall;
	}

	auto maximum_recall_or(double empty_recall = 0.0) const -> double
	{
		return observations == 0 ? empty_recall : maximum_recall;
	}
};

template <typename Value = double> struct ScalarAccumulator {
	std::size_t observations{};
	double sum{};
	Value minimum{};
	Value maximum{};
	bool has_value{};

	auto add(Value value) -> void
	{
		sum += static_cast<double>(value);
		if (!has_value) {
			minimum = value;
			maximum = value;
			has_value = true;
		} else {
			if (value < minimum) {
				minimum = value;
			}
			if (maximum < value) {
				maximum = value;
			}
		}
		++observations;
	}

	auto average_or(double empty_value = 0.0) const -> double
	{
		if (observations == 0) {
			return empty_value;
		}
		return sum / static_cast<double>(observations);
	}

	auto minimum_or(Value empty_value = Value{}) const -> Value { return has_value ? minimum : empty_value; }

	auto maximum_or(Value empty_value = Value{}) const -> Value { return has_value ? maximum : empty_value; }
};

template <typename Value> auto summarize_scalars(const std::vector<Value> &values) -> ScalarAccumulator<Value>
{
	ScalarAccumulator<Value> summary;
	for (const auto value : values) {
		summary.add(value);
	}
	return summary;
}

template <typename DistanceMatrix, typename Distance>
auto expansion_dimension(const DistanceMatrix &distances, std::size_t record_count, Distance zero) -> double
{
	double maximum_dimension = 0.0;
	for (std::size_t row = 0; row < record_count; ++row) {
		for (std::size_t column = 0; column < record_count; ++column) {
			const auto radius = distances(row, column);
			if (radius <= zero) {
				continue;
			}

			const auto outer_radius = radius + radius;
			std::size_t inner_count = 0;
			std::size_t outer_count = 0;
			for (std::size_t candidate = 0; candidate < record_count; ++candidate) {
				const auto value = distances(row, candidate);
				if (value <= radius) {
					++inner_count;
				}
				if (value <= outer_radius) {
					++outer_count;
				}
			}

			if (inner_count > 0 && outer_count >= inner_count) {
				maximum_dimension =
					std::max(maximum_dimension, std::log(static_cast<double>(outer_count) /
														 static_cast<double>(inner_count)) /
											   std::log(2.0));
			}
		}
	}
	return maximum_dimension;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto record_ids(const Space &space) -> std::vector<RecordId>
{
	std::vector<RecordId> ids;
	ids.reserve(space.size());
	for (std::size_t index = 0; index < space.size(); ++index) {
		ids.push_back(space.id(index));
	}
	return ids;
}

template <typename Source, typename std::enable_if<IndexedRecordIdSource_v<Source>, int>::type = 0>
auto record_ids_at_positions(const Source &source, const std::vector<std::size_t> &positions) -> std::vector<RecordId>
{
	std::vector<RecordId> ids;
	ids.reserve(positions.size());
	for (const auto position : positions) {
		ids.push_back(source.id(position));
	}
	return ids;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto records_for_record_ids(const Space &space, const std::vector<RecordId> &ids)
	-> std::vector<typename Space::record_type>
{
	std::vector<typename Space::record_type> records;
	records.reserve(ids.size());
	for (const auto id : ids) {
		records.push_back(space.record(id));
	}
	return records;
}

template <typename Space, typename Transform, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto transformed_records(const Space &space, Transform &transform)
	-> std::vector<typename std::decay<decltype(std::declval<Transform &>()(
		std::declval<const typename Space::record_type &>()))>::type>
{
	using target_record_type = typename std::decay<decltype(std::declval<Transform &>()(
		std::declval<const typename Space::record_type &>()))>::type;

	std::vector<target_record_type> records;
	records.reserve(space.size());
	for (std::size_t index = 0; index < space.size(); ++index) {
		records.push_back(transform(space.record(space.id(index))));
	}
	return records;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto require_record_ids_in_space(const Space &space, const std::vector<RecordId> &ids, const char *not_found_message)
	-> void
{
	for (const auto id : ids) {
		if (!space.contains(id)) {
			throw std::invalid_argument(not_found_message);
		}
	}
}

struct OneToOneLineage {
	std::vector<std::vector<RecordId>> source_records;
	std::vector<RecordId> representative_records;
};

inline auto one_to_one_lineage(std::vector<RecordId> representative_records) -> OneToOneLineage
{
	std::vector<std::vector<RecordId>> source_records;
	source_records.reserve(representative_records.size());

	for (const auto id : representative_records) {
		source_records.push_back(std::vector<RecordId>{id});
	}

	return {std::move(source_records), std::move(representative_records)};
}

inline auto one_to_one_lineage_source_ids(const std::vector<std::vector<RecordId>> &source_records,
										  std::size_t expected_rows, const char *row_count_message,
										  const char *lineage_shape_message,
										  const char *duplicate_message) -> std::vector<RecordId>
{
	if (source_records.size() != expected_rows) {
		throw std::invalid_argument(row_count_message);
	}

	std::vector<RecordId> source_ids;
	source_ids.reserve(source_records.size());
	for (const auto &source_row : source_records) {
		if (source_row.size() != 1) {
			throw std::invalid_argument(lineage_shape_message);
		}
		if (contains_record_id(source_ids, source_row.front())) {
			throw std::invalid_argument(duplicate_message);
		}
		source_ids.push_back(source_row.front());
	}
	return source_ids;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto one_to_one_lineage_source_ids_in_space(const Space &space,
											const std::vector<std::vector<RecordId>> &source_records,
											std::size_t expected_rows, const char *row_count_message,
											const char *lineage_shape_message, const char *duplicate_message,
											const char *not_found_message) -> std::vector<RecordId>
{
	auto source_ids =
		one_to_one_lineage_source_ids(source_records, expected_rows, row_count_message, lineage_shape_message,
									  duplicate_message);
	require_record_ids_in_space(space, source_ids, not_found_message);
	return source_ids;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto one_to_one_lineage(const Space &space) -> OneToOneLineage
{
	return one_to_one_lineage(record_ids(space));
}

template <typename Provider, typename = void> struct PairwiseDistances : std::false_type {};

template <typename Provider>
struct PairwiseDistances<
	Provider,
	std::void_t<typename Provider::distance_type,
				decltype(std::declval<const Provider &>().distance(std::declval<RecordId>(), std::declval<RecordId>())),
				decltype(std::declval<const Provider &>().record_count()),
				decltype(std::declval<const Provider &>().id(std::declval<std::size_t>())),
				decltype(std::declval<const Provider &>().position_of(std::declval<RecordId>())),
				decltype(std::declval<const Provider &>().contains(std::declval<RecordId>())),
				decltype(std::declval<const Provider &>().version()),
				decltype(std::declval<const Provider &>().is_stale())>> : std::true_type {};

template <typename Provider> constexpr bool PairwiseDistances_v = PairwiseDistances<Provider>::value;

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto total_distance_to_provider_records(const Provider &provider, RecordId source_id) -> typename Provider::distance_type
{
	typename Provider::distance_type total{};
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		total += provider.distance(source_id, provider.id(index));
	}
	return total;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto total_distance_to_record_ids(const Provider &provider, RecordId source_id, const std::vector<RecordId> &record_ids)
	-> typename Provider::distance_type
{
	typename Provider::distance_type total{};
	for (const auto id : record_ids) {
		total += provider.distance(source_id, id);
	}
	return total;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto minimum_total_distance_record_id(const Provider &provider, const std::vector<RecordId> &record_ids,
									  const char *empty_message, const char *not_found_message) -> RecordId
{
	if (record_ids.empty()) {
		throw std::invalid_argument(empty_message);
	}

	for (const auto id : record_ids) {
		if (!provider.contains(id)) {
			throw std::invalid_argument(not_found_message);
		}
	}

	auto best_id = record_ids.front();
	auto best_total = total_distance_to_record_ids(provider, best_id, record_ids);
	for (std::size_t index = 1; index < record_ids.size(); ++index) {
		const auto candidate = record_ids[index];
		const auto total = total_distance_to_record_ids(provider, candidate, record_ids);
		if (total < best_total || (total == best_total && candidate.index() < best_id.index())) {
			best_id = candidate;
			best_total = total;
		}
	}
	return best_id;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto distance_table_for_record_ids(const Provider &provider, const std::vector<RecordId> &record_ids,
								   const char *not_found_message)
	-> std::vector<std::vector<typename Provider::distance_type>>
{
	for (const auto id : record_ids) {
		if (!provider.contains(id)) {
			throw std::invalid_argument(not_found_message);
		}
	}

	std::vector<std::vector<typename Provider::distance_type>> distances(
		record_ids.size(), std::vector<typename Provider::distance_type>(record_ids.size()));
	for (std::size_t lhs = 0; lhs < record_ids.size(); ++lhs) {
		for (std::size_t rhs = 0; rhs < record_ids.size(); ++rhs) {
			distances[lhs][rhs] = provider.distance(record_ids[lhs], record_ids[rhs]);
		}
	}
	return distances;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto distances_to_record_id(const Provider &provider, RecordId target_id, const char *not_found_message)
	-> std::vector<typename Provider::distance_type>
{
	if (!provider.contains(target_id)) {
		throw std::invalid_argument(not_found_message);
	}

	std::vector<typename Provider::distance_type> distances;
	distances.reserve(provider.record_count());
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		distances.push_back(provider.distance(provider.id(index), target_id));
	}
	return distances;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto update_min_distances_to_record_id(const Provider &provider,
									   std::vector<typename Provider::distance_type> &distances, RecordId target_id,
									   const char *size_message, const char *not_found_message) -> void
{
	if (distances.size() != provider.record_count()) {
		throw std::invalid_argument(size_message);
	}
	if (!provider.contains(target_id)) {
		throw std::invalid_argument(not_found_message);
	}

	for (std::size_t index = 0; index < distances.size(); ++index) {
		const auto distance = provider.distance(provider.id(index), target_id);
		if (distance < distances[index]) {
			distances[index] = distance;
		}
	}
}

template <typename Distances>
auto farthest_unselected_position(const Distances &distances, const std::vector<bool> &selected,
								  const char *size_message, const char *empty_message) -> std::size_t
{
	if (distances.size() != selected.size()) {
		throw std::invalid_argument(size_message);
	}

	std::size_t best_position = distances.size();
	bool has_best = false;
	for (std::size_t position = 0; position < distances.size(); ++position) {
		if (selected[position]) {
			continue;
		}
		if (!has_best || distances[best_position] < distances[position]) {
			best_position = position;
			has_best = true;
		}
	}

	if (!has_best) {
		throw std::logic_error(empty_message);
	}
	return best_position;
}

template <typename Source, typename Distances, typename std::enable_if<IndexedRecordIdSource_v<Source>, int>::type = 0>
auto farthest_unselected_record_position(const Source &source, const Distances &distances,
										 const std::vector<bool> &selected, const char *size_message,
										 const char *empty_message) -> std::size_t
{
	if (distances.size() != selected.size()) {
		throw std::invalid_argument(size_message);
	}

	std::size_t best_position = distances.size();
	bool has_best = false;
	for (std::size_t position = 0; position < distances.size(); ++position) {
		if (selected[position]) {
			continue;
		}
		if (!has_best || distances[best_position] < distances[position] ||
			(distances[position] == distances[best_position] && source.id(position) < source.id(best_position))) {
			best_position = position;
			has_best = true;
		}
	}

	if (!has_best) {
		throw std::logic_error(empty_message);
	}
	return best_position;
}

inline auto first_unmarked_position(const std::vector<bool> &marked, const char *empty_message) -> std::size_t
{
	for (std::size_t position = 0; position < marked.size(); ++position) {
		if (!marked[position]) {
			return position;
		}
	}
	throw std::logic_error(empty_message);
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto mark_records_within_radius(const Provider &provider, RecordId seed_id, typename Provider::distance_type radius,
								std::vector<bool> &marked, const char *size_message,
								const char *seed_message) -> std::size_t
{
	if (marked.size() != provider.record_count()) {
		throw std::invalid_argument(size_message);
	}
	if (!provider.contains(seed_id)) {
		throw std::invalid_argument(seed_message);
	}

	std::size_t newly_marked = 0;
	for (std::size_t index = 0; index < marked.size(); ++index) {
		if (!marked[index] && provider.distance(seed_id, provider.id(index)) <= radius) {
			marked[index] = true;
			++newly_marked;
		}
	}
	return newly_marked;
}

template <typename Provider, typename MinimumDistance, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto record_is_separated_from_record_ids(const Provider &provider, RecordId candidate_id,
										 const std::vector<RecordId> &reference_ids,
										 MinimumDistance minimum_distance,
										 const char *candidate_message,
										 const char *reference_message) -> bool
{
	using comparison_type = typename std::common_type<typename Provider::distance_type, MinimumDistance>::type;

	if (!provider.contains(candidate_id)) {
		throw std::invalid_argument(candidate_message);
	}

	const auto threshold = static_cast<comparison_type>(minimum_distance);
	for (const auto reference_id : reference_ids) {
		if (!provider.contains(reference_id)) {
			throw std::invalid_argument(reference_message);
		}
		if (static_cast<comparison_type>(provider.distance(candidate_id, reference_id)) < threshold) {
			return false;
		}
	}
	return true;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto nearest_distance_to_record_ids(const Provider &provider, RecordId source_id, const std::vector<RecordId> &record_ids,
									const char *empty_message) -> typename Provider::distance_type
{
	if (record_ids.empty()) {
		throw std::invalid_argument(empty_message);
	}

	auto best_distance = provider.distance(source_id, record_ids.front());
	for (std::size_t index = 1; index < record_ids.size(); ++index) {
		const auto distance = provider.distance(source_id, record_ids[index]);
		if (distance < best_distance) {
			best_distance = distance;
		}
	}
	return best_distance;
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto nearest_other_record_distance_or(const Provider &provider, RecordId source_id, typename Provider::distance_type fallback)
	-> typename Provider::distance_type
{
	auto best_distance = fallback;
	bool has_best = false;
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		const auto candidate_id = provider.id(index);
		if (candidate_id == source_id) {
			continue;
		}
		const auto distance = provider.distance(source_id, candidate_id);
		if (!has_best || distance < best_distance) {
			best_distance = distance;
			has_best = true;
		}
	}
	return best_distance;
}

template <typename Distance> struct RepresentativeAssignment {
	std::vector<std::size_t> assignments;
	std::vector<Distance> nearest_distances;
};

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto assign_records_to_representatives(const Provider &provider, const std::vector<RecordId> &representatives,
									   const char *empty_message, const char *not_found_message)
	-> RepresentativeAssignment<typename Provider::distance_type>
{
	if (representatives.empty()) {
		throw std::invalid_argument(empty_message);
	}

	for (const auto id : representatives) {
		if (!provider.contains(id)) {
			throw std::invalid_argument(not_found_message);
		}
	}

	RepresentativeAssignment<typename Provider::distance_type> result;
	result.assignments.reserve(provider.record_count());
	result.nearest_distances.reserve(provider.record_count());

	for (std::size_t record_index = 0; record_index < provider.record_count(); ++record_index) {
		const auto source_id = provider.id(record_index);
		std::size_t best_representative_index = 0;
		auto best_distance = provider.distance(source_id, representatives.front());

		for (std::size_t representative_index = 1; representative_index < representatives.size();
			 ++representative_index) {
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

template <typename Index, typename = void> struct NeighborSearchIndex : std::false_type {};

template <typename Index>
struct NeighborSearchIndex<
	Index,
	std::void_t<typename Index::record_type, typename Index::distance_type,
				decltype(std::declval<const Index &>().knn(std::declval<const typename Index::record_type &>(),
														   std::declval<std::size_t>())),
				decltype(std::declval<const Index &>().record_count()),
				decltype(std::declval<const Index &>().version()), decltype(std::declval<const Index &>().is_stale())>>
	: std::true_type {};

template <typename Index> constexpr bool NeighborSearchIndex_v = NeighborSearchIndex<Index>::value;

template <typename Topology, typename = void> struct GraphTopology : std::false_type {};

template <typename Topology>
struct GraphTopology<Topology, std::void_t<typename Topology::edge_type, typename Topology::distance_type,
										   decltype(std::declval<const Topology &>().edges()),
										   decltype(std::declval<const Topology &>().record_count()),
										   decltype(std::declval<const Topology &>().version()),
										   decltype(std::declval<const Topology &>().is_stale())>> : std::true_type {};

template <typename Topology> constexpr bool GraphTopology_v = GraphTopology<Topology>::value;

template <typename MappingT, typename Space, typename = void> struct Mapping : std::false_type {};

template <typename MappingT, typename Space>
struct Mapping<MappingT, Space,
			   std::void_t<decltype(std::declval<const MappingT &>().fit(std::declval<const Space &>()))>>
	: std::true_type {};

template <typename MappingT, typename Space> constexpr bool Mapping_v = Mapping<MappingT, Space>::value;

template <typename ModelT, typename Space, typename = void> struct MappingModel : std::false_type {};

template <typename ModelT, typename Space>
struct MappingModel<ModelT, Space,
					std::void_t<decltype(std::declval<const ModelT &>().transform(std::declval<const Space &>()))>>
	: std::true_type {};

template <typename ModelT, typename Space> constexpr bool MappingModel_v = MappingModel<ModelT, Space>::value;

} // namespace mtrc::core

namespace mtrc {
using core::assign_records_to_representatives;
using core::PairwiseDistances;
using core::PairwiseDistances_v;
using core::distance_table_for_record_ids;
using core::distances_to_record_id;
using core::expansion_dimension;
using core::farthest_unselected_position;
using core::farthest_unselected_record_position;
using core::first_unmarked_position;
using core::GraphTopology;
using core::GraphTopology_v;
using core::contains_record_id;
using core::IndexedRecordIdSource;
using core::IndexedRecordIdSource_v;
using core::Mapping;
using core::Mapping_v;
using core::MappingModel;
using core::MappingModel_v;
using core::mark_records_within_radius;
using core::metric_result_t;
using core::minimum_total_distance_record_id;
using core::MetricCallable;
using core::MetricCallable_v;
using core::MetricSpaceLike;
using core::MetricSpaceLike_v;
using core::nearest_distance_to_record_ids;
using core::nearest_other_record_distance_or;
using core::NeighborSearchIndex;
using core::NeighborSearchIndex_v;
using core::OneToOneLineage;
using core::one_to_one_lineage;
using core::one_to_one_lineage_source_ids;
using core::one_to_one_lineage_source_ids_in_space;
using core::position_of_record_id;
using core::record_is_separated_from_record_ids;
using core::record_id_overlap_count;
using core::record_ids;
	using core::record_ids_at_positions;
	using core::records_for_record_ids;
	using core::RecallAccumulator;
	using core::RepresentativeAssignment;
	using core::require_record_ids_in_space;
	using core::ScalarAccumulator;
using core::summarize_scalars;
using core::total_distance_to_provider_records;
using core::total_distance_to_record_ids;
using core::transformed_records;
using core::update_min_distances_to_record_id;
} // namespace mtrc

#endif
