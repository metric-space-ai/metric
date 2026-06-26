// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_CACHE_HPP
#define _METRIC_SPACE_CACHE_HPP

#include <cstddef>
#include <utility>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/space/storage/cover_tree_index.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/exact_scan_index.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/space/storage/knn_graph_index.hpp>
#include <metric/space/storage/landmark_index.hpp>

// Stale-representation detection and recovery for finite-space caches and indexes.
//
// A MetricSpace carries a version that advances on every record mutation. A derived representation
// (DistanceTable, CoverTreeIndex, KnnGraphIndex, ExactScanIndex, ...) is built for one version and
// becomes stale the moment the space mutates. The representations already know this (is_stale());
// mtrc::space::cache turns it into a usable workflow:
//
//   * status(rep)        -> a compact stale / version snapshot.
//   * require_fresh(rep) -> throw StaleRepresentationError instead of letting a caller read stale data.
//   * rebuild(rep, space)-> a fresh representation of the same kind for the space's current version,
//                           reusing the storage component's own constructor (no algorithm is duplicated
//                           and the representation's defining parameters, e.g. a kNN graph's k, are
//                           preserved).
//   * refresh(rep, space)-> rebuild in place only if stale; returns whether a rebuild happened.
//
// Recovery is never automatic on a read path: a caller must observe staleness and choose to rebuild,
// so stale values are never served silently after a mutation.
namespace mtrc::space::cache {

// Compact staleness snapshot of a representation against the version it was built for.
struct cache_status {
	bool stale{};
	std::size_t built_for_version{};
};

// True iff the representation is stale (built for an older space version than its source now has).
template <typename Representation> auto is_stale(const Representation &representation) -> bool
{
	return representation.is_stale();
}

template <typename Representation> auto status(const Representation &representation) -> cache_status
{
	return cache_status{representation.is_stale(), representation.version()};
}

// Throw StaleRepresentationError if the representation is stale. Use this on the read path of any
// cache/index whose values must reflect the current space.
template <typename Representation>
auto require_fresh(const Representation &representation,
				   const char *message = "finite-space representation is stale: it was built for an older "
										 "metric-space version; rebuild it before use") -> void
{
	if (representation.is_stale()) {
		throw StaleRepresentationError(message);
	}
}

// ---------------------------------------------------------------------------------------------
// rebuild(): a fresh representation of the same kind, built for the space's current version. Each
// overload reuses the storage component's existing constructor, preserving its defining parameters.
// ---------------------------------------------------------------------------------------------

template <typename Space>
auto rebuild(const storage::DistanceTable<Space> &table, const Space &space) -> storage::DistanceTable<Space>
{
	// Preserve the table's defining parameters. A table the caller built lazy stays lazy after a
	// rebuild (no surprise all-pairs materialization), and budget limits continue to apply.
	return storage::DistanceTable<Space>(
		space, storage::distance_table_options{table.mode(), table.max_dense_records(), table.max_memory_bytes()});
}

template <typename Space>
auto rebuild(const storage::CoverTreeIndex<Space> &index, const Space &space) -> storage::CoverTreeIndex<Space>
{
	return storage::CoverTreeIndex<Space>(
		space, storage::cover_tree_index_options{index.max_build_distance_evaluations(), index.max_memory_bytes()});
}

template <typename Space>
auto rebuild(const storage::KnnGraphIndex<Space> &index, const Space &space) -> storage::KnnGraphIndex<Space>
{
	return storage::KnnGraphIndex<Space>(
		space, storage::knn_graph_index_options{index.k(), index.max_build_distance_evaluations()});
}

template <typename Space>
auto rebuild(const storage::LandmarkIndex<Space> &index, const Space &space) -> storage::LandmarkIndex<Space>
{
	return storage::LandmarkIndex<Space>(
		space, storage::landmark_index_options{index.requested_landmark_count(), index.candidate_limit(),
											   index.max_build_distance_evaluations(), index.max_memory_bytes()});
}

template <typename Space>
auto rebuild(const storage::ExactScanIndex<Space> &, const Space &space) -> storage::ExactScanIndex<Space>
{
	return storage::ExactScanIndex<Space>(space);
}

// Rebuild the representation in place only if it is stale. Returns true iff a rebuild happened, so a
// caller can tell whether downstream state needs refreshing too. No-op (returns false) when fresh.
template <typename Representation, typename Space>
auto refresh(Representation &representation, const Space &space) -> bool
{
	if (!representation.is_stale()) {
		return false;
	}
	representation = rebuild(representation, space);
	return true;
}

} // namespace mtrc::space::cache

#endif
