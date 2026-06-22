// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_RECORDS_HPP
#define _METRIC_SPACE_RECORDS_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/version.hpp>
#include <metric/record/id.hpp>
#include <metric/record/ids.hpp>

// Structural record-set management for a finite metric space.
//
// mtrc::space::records owns the user-facing batch operations on the record set of a
// mtrc::core::MetricSpace: batch insert, batch erase, batch replace/edit, plus the ID/position
// validation and stable-identity utilities a caller needs to drive those operations safely.
//
// This namespace deliberately contains NO metric math: it never evaluates the metric, never
// queries neighbours, and never analyses structure. It only adds, removes, edits, and locates
// records, and reports exactly which RecordIds were affected and how the space size/version moved.
// Metric selection lives in mtrc::metric; querying lives in mtrc::stats::search /
// mtrc::space::query; analysis lives in mtrc::stats. Keeping record-set mutation separate means a
// caller can reason about identity and cache/index invalidation without reasoning about distances.
namespace mtrc::space::records {

enum class mutation_kind {
	none,
	insert,
	erase,
	replace,
};

inline auto mutation_kind_name(mutation_kind kind) -> const char *
{
	switch (kind) {
	case mutation_kind::none:
		return "none";
	case mutation_kind::insert:
		return "insert";
	case mutation_kind::erase:
		return "erase";
	case mutation_kind::replace:
		return "replace";
	}
	return "none";
}

// Report of a single batch record-set mutation. It records what changed so a caller can update
// caches/indexes, audit identity, or surface the effect to a user without re-deriving it.
//
//  - affected_ids: the RecordIds that were inserted (new ids), erased, or replaced, in operation order.
//  - missing_ids:  requested RecordIds that were not present (erase/replace); empty for insert.
//  - old_size / new_size: record count before and after the whole batch.
//  - version_before / version_after: space version before and after the whole batch. They differ iff at
//    least one record actually changed, so changed() is the authoritative "did this invalidate caches?"
//    signal: any provider/index built for version_before is stale once changed() is true.
struct mutation_report {
	mutation_kind kind{mutation_kind::none};
	std::size_t old_size{};
	std::size_t new_size{};
	std::vector<RecordId> affected_ids;
	std::vector<RecordId> missing_ids;
	core::SpaceVersion version_before{};
	core::SpaceVersion version_after{};

	auto count() const -> std::size_t { return affected_ids.size(); }
	auto missing_count() const -> std::size_t { return missing_ids.size(); }
	auto changed() const -> bool { return version_before != version_after; }
};

// ---------------------------------------------------------------------------------------------
// Validation and stable-identity utilities.
// ---------------------------------------------------------------------------------------------

// Every RecordId currently in the space, in position order. Stable identity: a RecordId keeps
// referring to the same record across insert/erase of other records, even though its position moves.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto ids(const Space &space) -> std::vector<RecordId>
{
	return mtrc::record_ids(space);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto contains_all(const Space &space, const std::vector<RecordId> &ids) -> bool
{
	for (const auto id : ids) {
		if (!space.contains(id)) {
			return false;
		}
	}
	return true;
}

// RecordIds from the request that are not present in the space, in request order (each reported once).
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto missing_ids(const Space &space, const std::vector<RecordId> &ids) -> std::vector<RecordId>
{
	std::vector<RecordId> missing;
	for (const auto id : ids) {
		if (!space.contains(id) && !mtrc::contains_record_id(missing, id)) {
			missing.push_back(id);
		}
	}
	return missing;
}

// Throw MetricInputError if any requested RecordId is not present. Used to make batch erase/replace
// strict: validate the whole request up front so a partially-applied mutation never happens silently.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto validate_ids(const Space &space, const std::vector<RecordId> &ids,
				  const char *message = "record id is not present in the metric space") -> void
{
	for (const auto id : ids) {
		if (!space.contains(id)) {
			throw MetricInputError(message);
		}
	}
}

// Current position of each requested RecordId. Throws std::out_of_range if any id is missing.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto positions_of(const Space &space, const std::vector<RecordId> &ids) -> std::vector<std::size_t>
{
	std::vector<std::size_t> positions;
	positions.reserve(ids.size());
	for (const auto id : ids) {
		positions.push_back(space.position_of(id));
	}
	return positions;
}

// Throw std::out_of_range if a position is outside the space. Mirrors the space's own bounds rule for
// callers that index by position rather than RecordId.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto validate_position(const Space &space, std::size_t position,
					   const char *message = "record position is outside the metric space") -> void
{
	if (position >= space.size()) {
		throw std::out_of_range(message);
	}
}

// A requested edit: replace the record currently identified by `id` with `record`.
template <typename Record> struct edit {
	RecordId id;
	Record record;
};

// ---------------------------------------------------------------------------------------------
// Batch mutations.
// ---------------------------------------------------------------------------------------------

// Batch insert. Appends every record in `records` and returns the new RecordIds in insertion order.
// New ids are stable and never reuse an erased id's index, so an existing RecordId is never aliased.
template <typename Space, typename Container,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto insert(Space &space, const Container &records) -> mutation_report
{
	mutation_report report;
	report.kind = mutation_kind::insert;
	report.old_size = space.size();
	report.version_before = space.version();
	for (const auto &record : records) {
		report.affected_ids.push_back(space.insert(typename Space::record_type(record)));
	}
	report.new_size = space.size();
	report.version_after = space.version();
	return report;
}

// Batch erase, lenient. Erases every requested RecordId that is present and reports the rest under
// missing_ids; never throws for a missing id. affected_ids holds the ids actually removed, in request
// order. Duplicate ids in the request are only acted on once (the second sees the id already gone and
// is reported as missing).
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto erase(Space &space, const std::vector<RecordId> &ids) -> mutation_report
{
	mutation_report report;
	report.kind = mutation_kind::erase;
	report.old_size = space.size();
	report.version_before = space.version();
	for (const auto id : ids) {
		if (space.erase(id)) {
			report.affected_ids.push_back(id);
		} else {
			report.missing_ids.push_back(id);
		}
	}
	report.new_size = space.size();
	report.version_after = space.version();
	return report;
}

// Batch erase, strict. Validates the whole request before erasing anything, so the space is either
// fully updated or untouched (no partial application). Throws MetricInputError if any id is missing,
// or if the request contains a duplicate id (a duplicate would otherwise be reported as "missing"
// after the first occurrence erased it, silently breaking the all-or-nothing contract).
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto erase_strict(Space &space, const std::vector<RecordId> &ids) -> mutation_report
{
	if (mtrc::has_duplicate_record_ids(ids)) {
		throw MetricInputError("cannot erase strictly with a duplicate record id in the request");
	}
	validate_ids(space, ids, "cannot erase a record id that is not present in the metric space");
	return erase(space, ids);
}

// Batch replace/edit. Validates the whole request before replacing anything, then replaces in request
// order. Identity is preserved: a replaced record keeps its RecordId, so caches keyed by id stay
// addressable (but their cached distances are invalidated by the version bump). Throws
// MetricInputError if any edited id is missing, or if two edits target the same RecordId (a duplicate
// target would otherwise be silently last-writer-wins with the winner decided by request order).
template <typename Space, typename Record = typename Space::record_type,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto replace(Space &space, const std::vector<edit<Record>> &edits) -> mutation_report
{
	std::vector<RecordId> ids;
	ids.reserve(edits.size());
	for (const auto &item : edits) {
		ids.push_back(item.id);
	}
	if (mtrc::has_duplicate_record_ids(ids)) {
		throw MetricInputError("cannot replace with a duplicate record id target in the request");
	}
	validate_ids(space, ids, "cannot replace a record id that is not present in the metric space");

	mutation_report report;
	report.kind = mutation_kind::replace;
	report.old_size = space.size();
	report.version_before = space.version();
	for (const auto &item : edits) {
		space.replace(item.id, typename Space::record_type(item.record));
		report.affected_ids.push_back(item.id);
	}
	report.new_size = space.size();
	report.version_after = space.version();
	return report;
}

} // namespace mtrc::space::records

#endif
