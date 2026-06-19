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

} // namespace metric::core

namespace metric {
template <typename Distance> using NeighborSet = core::NeighborSet<Distance>;
template <typename Distance> using ClusteringResult = core::ClusteringResult<Distance>;
} // namespace metric

#endif
