// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_GRAPH_TOPOLOGY_HPP
#define _METRIC_REPRESENTATIONS_GRAPH_TOPOLOGY_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../core/record_id.hpp"
#include "diagnostics.hpp"

namespace metric::representations {

template <typename Distance> struct GraphEdge {
	RecordId source;
	RecordId target;
	Distance distance;
};

template <typename Space> class GraphTopology {
  public:
	using space_type = Space;
	using distance_type = typename space_type::distance_type;
	using edge_type = GraphEdge<distance_type>;

	explicit GraphTopology(const space_type &space)
		: space_(&space)
		, record_count_(space.size())
		, version_(space.version())
	{
		ids_.reserve(record_count_);
		for (std::size_t index = 0; index < record_count_; ++index) {
			ids_.push_back(space.id(index));
		}
	}

	auto add_edge(RecordId source, RecordId target, distance_type distance) -> void
	{
		validate(source);
		validate(target);
		edges_.push_back(edge_type{source, target, distance});
	}

	auto edges() const -> const std::vector<edge_type> & { return edges_; }
	auto edge_count() const -> std::size_t { return edges_.size(); }
	auto record_count() const -> std::size_t { return record_count_; }
	auto id(std::size_t position) const -> RecordId
	{
		validate_position(position);
		return ids_[position];
	}
	auto position_of(RecordId id) const -> std::size_t
	{
		for (std::size_t position = 0; position < ids_.size(); ++position) {
			if (ids_[position] == id) {
				return position;
			}
		}
		throw std::out_of_range("record id is outside the graph topology");
	}
	auto contains(RecordId id) const -> bool
	{
		for (const auto current : ids_) {
			if (current == id) {
				return true;
			}
		}
		return false;
	}
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }
	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::graph_topology,
										  exactness::approximate,
										  materialization::topology,
										  update_mode::snapshot};
		result.space_version = space_->version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.records = record_count_;
		result.cached_distances = edges_.size();
		result.memory_bytes_estimate = ids_.size() * sizeof(RecordId) + edges_.size() * sizeof(edge_type);
		if (result.stale) {
			result.warnings.push_back("graph topology was built for an older metric-space version");
		}
		return result;
	}

  private:
	auto validate(RecordId id) const -> void
	{
		if (!contains(id)) {
			throw std::out_of_range("record id is outside the graph topology");
		}
	}

	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the graph topology");
		}
	}

	const space_type *space_;
	std::size_t record_count_;
	std::size_t version_;
	std::vector<RecordId> ids_;
	std::vector<edge_type> edges_;
};

} // namespace metric::representations

#endif
