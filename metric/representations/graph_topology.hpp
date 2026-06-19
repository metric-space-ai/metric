// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_GRAPH_TOPOLOGY_HPP
#define _METRIC_REPRESENTATIONS_GRAPH_TOPOLOGY_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../core/record_id.hpp"

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
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }

  private:
	auto validate(RecordId id) const -> void
	{
		if (id.index() >= record_count_) {
			throw std::out_of_range("record id is outside the graph topology");
		}
	}

	const space_type *space_;
	std::size_t record_count_;
	std::size_t version_;
	std::vector<edge_type> edges_;
};

} // namespace metric::representations

#endif
