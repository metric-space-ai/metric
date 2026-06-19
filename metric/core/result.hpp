// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_RESULT_HPP
#define _METRIC_CORE_RESULT_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "neighbor.hpp"
#include "record_id.hpp"

namespace metric::core {

template <typename Distance> struct NeighborSet {
	using distance_type = Distance;
	using neighbor_type = Neighbor<Distance>;

	std::vector<neighbor_type> neighbors;
	std::size_t record_count{};
	std::size_t requested_count{};
	bool exact{true};
	std::string operator_name;
	std::string representation;

	auto size() const -> std::size_t { return neighbors.size(); }
	auto empty() const -> bool { return neighbors.empty(); }
	auto begin() const -> typename std::vector<neighbor_type>::const_iterator { return neighbors.begin(); }
	auto end() const -> typename std::vector<neighbor_type>::const_iterator { return neighbors.end(); }
	auto operator[](std::size_t index) const -> const neighbor_type & { return neighbors[index]; }
};

template <typename Distance> struct ClusteringResult {
	using distance_type = Distance;
	static constexpr std::size_t noise_label = static_cast<std::size_t>(-1);

	std::vector<std::size_t> assignments;
	std::vector<RecordId> medoids;
	std::vector<RecordId> core_records;
	std::vector<RecordId> noise_records;
	std::vector<std::size_t> cluster_sizes;
	std::size_t record_count{};
	std::size_t cluster_count{};
	std::size_t noise_count{};
	std::size_t iterations{};
	bool converged{false};
	std::string algorithm;
	std::string representation;

	auto empty() const -> bool { return assignments.empty(); }
};

template <typename Distance> struct RepresentativeSet {
	using distance_type = Distance;

	std::vector<RecordId> representatives;
	std::vector<Distance> nearest_representative_distances;
	std::size_t record_count{};
	std::size_t requested_count{};
	Distance coverage_radius{};
	double average_nearest_distance{};
	bool exact{true};
	std::string operator_name;
	std::string strategy;
	std::string representation;

	auto size() const -> std::size_t { return representatives.size(); }
	auto empty() const -> bool { return representatives.empty(); }
	auto begin() const -> typename std::vector<RecordId>::const_iterator { return representatives.begin(); }
	auto end() const -> typename std::vector<RecordId>::const_iterator { return representatives.end(); }
	auto operator[](std::size_t index) const -> RecordId { return representatives[index]; }
};

template <typename Value = double> struct EntropyResult {
	using value_type = Value;

	Value value{};
	std::size_t record_count{};
	std::size_t neighbor_count{};
	std::size_t approximation_order{};
	bool exponentiated{false};
	bool exact{true};
	std::string algorithm;
	std::string representation;
};

template <typename Value = double> struct CorrelationResult {
	using value_type = Value;

	Value value{};
	std::size_t left_record_count{};
	std::size_t right_record_count{};
	bool exact{true};
	std::string algorithm;
	std::string left_representation;
	std::string right_representation;
};

template <typename Distance, typename Value = double> struct StructureDescription {
	using distance_type = Distance;
	using value_type = Value;

	std::size_t record_count{};
	std::size_t pair_count{};
	std::size_t zero_distance_pair_count{};
	Distance minimum_nonzero_distance{};
	Distance maximum_distance{};
	Value average_distance{};
	Value intrinsic_dimension{};
	bool has_nonzero_distances{false};
	bool exact{true};
	std::string operator_name;
	std::string strategy;
	std::string representation;
};

template <typename Space> struct MappingResult {
	using space_type = Space;

	Space space;
	std::vector<std::vector<RecordId>> source_records;
	std::vector<RecordId> representative_records;
	std::size_t source_record_count{};
	bool inverse_supported{false};
	std::string mapping;
	std::string strategy;
	std::string representation;

	auto size() const -> std::size_t { return space.size(); }
	auto empty() const -> bool { return space.empty(); }
};

} // namespace metric::core

namespace metric {
template <typename Distance> using NeighborSet = core::NeighborSet<Distance>;
template <typename Distance> using ClusteringResult = core::ClusteringResult<Distance>;
template <typename Distance> using RepresentativeSet = core::RepresentativeSet<Distance>;
template <typename Value = double> using EntropyResult = core::EntropyResult<Value>;
template <typename Value = double> using CorrelationResult = core::CorrelationResult<Value>;
template <typename Distance, typename Value = double>
using StructureDescription = core::StructureDescription<Distance, Value>;
template <typename Space> using MappingResult = core::MappingResult<Space>;
} // namespace metric

#endif
