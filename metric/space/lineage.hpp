// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_LINEAGE_HPP
#define _METRIC_SPACE_LINEAGE_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/record/id.hpp>
#include <metric/space/partition.hpp>

// Inspecting and protecting the lineage of sub-spaces and merged spaces.
//
// mtrc::space::split / select_subspace / merge (in partition.hpp) already carry lineage: a Subspace
// records the parent RecordId of each record (source_ids), and a MergedSpace records, per record, the
// input space and original RecordId it came from (lineage). Those derived spaces are independent COPIES
// with their own fresh RecordIds, so mutating the parent after the fact cannot corrupt them.
//
// This header adds the user-facing workflow around that data:
//
//   * Easy lineage lookup: parent_record_id(sub, local_id) / merge_origin(merged, local_id) and table
//     views, so a caller maps a derived RecordId back to its source without indexing parallel vectors
//     by hand.
//   * A mutation guard: a Subspace/MergedSpace is "aligned" only while its records still line up one-
//     to-one with its lineage vector. If the derived space is itself structurally mutated (records
//     inserted/erased) after creation, the lineage vector no longer matches; the lookups detect this
//     and throw StaleRepresentationError rather than return a wrong source id. (A pure replace() keeps
//     identity and stays aligned.)
//   * merge_checked(): a merge that first verifies the inputs share the same metric, so two spaces with
//     incompatible (differently-parameterised) metrics are never silently fused under one metric.
namespace mtrc::space {

// Expected RecordId index for the record at a given position in a freshly built derived space, where
// MetricSpace assigns ids 0..n-1 in order. Used by the alignment guard below.
namespace lineage_detail {

template <typename Space>
auto derived_space_is_pristine(const Space &space, std::size_t expected_lineage_size) -> bool
{
	if (space.size() != expected_lineage_size) {
		return false;
	}
	for (std::size_t position = 0; position < space.size(); ++position) {
		if (space.id(position).index() != position) {
			return false;
		}
	}
	return true;
}

} // namespace lineage_detail

// One row of a sub-space lineage table: a sub-space RecordId and the parent RecordId it came from.
struct subspace_lineage_entry {
	RecordId local_id;
	RecordId parent_id;
};

// One row of a merged-space lineage table: a merged RecordId and where it originated.
struct merged_lineage_entry {
	RecordId local_id;
	std::size_t source_index{};
	RecordId source_id{};
};

// True while the sub-space still lines up one-to-one with its lineage vector (i.e. it has not been
// structurally mutated since creation). A pure replace() keeps it aligned; insert/erase breaks it.
template <typename Record, typename Metric>
auto subspace_is_aligned(const Subspace<Record, Metric> &sub) -> bool
{
	return lineage_detail::derived_space_is_pristine(sub.space, sub.source_ids.size());
}

// True while the merged space still lines up one-to-one with its lineage vector.
template <typename Record, typename Metric>
auto merged_is_aligned(const MergedSpace<Record, Metric> &merged) -> bool
{
	return lineage_detail::derived_space_is_pristine(merged.space, merged.lineage.size());
}

// Parent RecordId that the sub-space record `local_id` came from. Throws StaleRepresentationError if
// the sub-space was structurally mutated after creation (so lineage can never silently return a wrong
// parent), and std::out_of_range if `local_id` is not in the sub-space.
template <typename Record, typename Metric>
auto parent_record_id(const Subspace<Record, Metric> &sub, RecordId local_id) -> RecordId
{
	if (!subspace_is_aligned(sub)) {
		throw StaleRepresentationError(
			"sub-space lineage is no longer aligned: the sub-space was structurally mutated after creation");
	}
	const auto position = sub.space.position_of(local_id);
	return sub.source_ids[position];
}

// Origin (input space index + original RecordId) of the merged record `local_id`. Throws
// StaleRepresentationError if the merged space was structurally mutated after creation, and
// std::out_of_range if `local_id` is not in the merged space.
template <typename Record, typename Metric>
auto merge_origin(const MergedSpace<Record, Metric> &merged, RecordId local_id) -> MergeOrigin
{
	if (!merged_is_aligned(merged)) {
		throw StaleRepresentationError(
			"merged-space lineage is no longer aligned: the merged space was structurally mutated after creation");
	}
	const auto position = merged.space.position_of(local_id);
	return merged.lineage[position];
}

// Full sub-space lineage as (local RecordId -> parent RecordId) rows, in sub-space position order.
// Throws StaleRepresentationError if the sub-space was structurally mutated after creation.
template <typename Record, typename Metric>
auto subspace_lineage(const Subspace<Record, Metric> &sub) -> std::vector<subspace_lineage_entry>
{
	if (!subspace_is_aligned(sub)) {
		throw StaleRepresentationError(
			"sub-space lineage is no longer aligned: the sub-space was structurally mutated after creation");
	}
	std::vector<subspace_lineage_entry> rows;
	rows.reserve(sub.source_ids.size());
	for (std::size_t position = 0; position < sub.source_ids.size(); ++position) {
		rows.push_back(subspace_lineage_entry{sub.space.id(position), sub.source_ids[position]});
	}
	return rows;
}

// Full merged-space lineage as rows, in merged position order. Throws StaleRepresentationError if the
// merged space was structurally mutated after creation.
template <typename Record, typename Metric>
auto merged_lineage(const MergedSpace<Record, Metric> &merged) -> std::vector<merged_lineage_entry>
{
	if (!merged_is_aligned(merged)) {
		throw StaleRepresentationError(
			"merged-space lineage is no longer aligned: the merged space was structurally mutated after creation");
	}
	std::vector<merged_lineage_entry> rows;
	rows.reserve(merged.lineage.size());
	for (std::size_t position = 0; position < merged.lineage.size(); ++position) {
		rows.push_back(merged_lineage_entry{merged.space.id(position), merged.lineage[position].source_index,
											merged.lineage[position].source_id});
	}
	return rows;
}

// True iff every non-null input space uses the same metric, compared by metric cache key. This is the
// compatibility precondition for a meaningful merge: merge() reuses the first input's metric, so fusing
// spaces whose metrics differ would silently reinterpret the others' distances. Note the resolution is
// the metric's cache key: metric types that do not specialise metric_traits<Metric>::cache_key fall back
// to the type name, so two instances of such a type with different runtime parameters compare equal.
// Callers that need parameter-level checking must give the metric a cache_key specialisation.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto metrics_compatible(const std::vector<const Space *> &spaces) -> bool
{
	bool have_key = false;
	std::string key;
	for (const auto *space : spaces) {
		if (space == nullptr) {
			continue;
		}
		const auto candidate = core::metric_cache_key(space->metric());
		if (!have_key) {
			key = candidate;
			have_key = true;
		} else if (candidate != key) {
			return false;
		}
	}
	return true;
}

// Merge that first checks metric compatibility. Throws MetricInputError if there is no usable
// (non-null) input space to adopt a metric from, or if the inputs do not share a metric; otherwise
// behaves exactly like space::merge. The no-usable-input guard keeps the "no silent metric
// reinterpretation" guarantee total: it refuses rather than fall back to a default-constructed metric.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto merge_checked(const std::vector<const Space *> &spaces)
	-> MergedSpace<typename Space::record_type, typename Space::metric_type>
{
	bool has_usable_space = false;
	for (const auto *space : spaces) {
		if (space != nullptr) {
			has_usable_space = true;
			break;
		}
	}
	if (!has_usable_space) {
		throw MetricInputError("merge_checked: no non-null input space to merge");
	}
	if (!metrics_compatible(spaces)) {
		throw MetricInputError("merge_checked: input spaces do not share a compatible metric");
	}
	return merge(spaces);
}

// Convenience two-argument metric-checked merge.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto merge_checked(const Space &lhs, const Space &rhs)
	-> MergedSpace<typename Space::record_type, typename Space::metric_type>
{
	return merge_checked(std::vector<const Space *>{&lhs, &rhs});
}

} // namespace mtrc::space

namespace mtrc {
using space::merge_checked;
using space::merge_origin;
using space::parent_record_id;
} // namespace mtrc

#endif
