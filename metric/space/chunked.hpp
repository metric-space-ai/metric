// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_CHUNKED_HPP
#define _METRIC_SPACE_CHUNKED_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/neighbor.hpp>
#include <metric/core/version.hpp>
#include <metric/record/id.hpp>

namespace mtrc::space {

namespace detail {

inline auto chunked_pair_count(std::size_t record_count) -> std::size_t
{
	return record_count < 2 ? std::size_t{0} : (record_count * (record_count - 1)) / 2;
}

inline auto chunk_count_for(std::size_t record_count, std::size_t chunk_size) -> std::size_t
{
	return record_count == 0 ? std::size_t{0} : ((record_count + chunk_size - 1) / chunk_size);
}

inline auto local_record_count_for(std::size_t record_count, std::size_t chunk_size, std::size_t chunk_index)
	-> std::size_t
{
	const auto start = chunk_index * chunk_size;
	const auto remaining = record_count - start;
	return remaining < chunk_size ? remaining : chunk_size;
}

template <typename Func, typename Distance>
auto invoke_local_pair(Func &callback, std::size_t chunk_index, RecordId lhs, RecordId rhs, Distance distance) -> void
{
	if constexpr (std::is_invocable_v<Func &, std::size_t, RecordId, RecordId, Distance>) {
		callback(chunk_index, lhs, rhs, distance);
	} else if constexpr (std::is_invocable_v<Func &, RecordId, RecordId, Distance>) {
		callback(lhs, rhs, distance);
	} else {
		static_assert(std::is_invocable_v<Func &, std::size_t, RecordId, RecordId, Distance> ||
						  std::is_invocable_v<Func &, RecordId, RecordId, Distance>,
					  "local pair callback must accept (chunk_index, lhs, rhs, distance) or "
					  "(lhs, rhs, distance)");
	}
}

template <typename Func, typename Record>
auto invoke_representative(Func &callback, std::size_t chunk_index, RecordId id, const Record &record) -> void
{
	if constexpr (std::is_invocable_v<Func &, std::size_t, RecordId, const Record &>) {
		callback(chunk_index, id, record);
	} else if constexpr (std::is_invocable_v<Func &, RecordId, const Record &>) {
		callback(id, record);
	} else if constexpr (std::is_invocable_v<Func &, std::size_t, RecordId>) {
		callback(chunk_index, id);
	} else if constexpr (std::is_invocable_v<Func &, RecordId>) {
		callback(id);
	} else {
		static_assert(std::is_invocable_v<Func &, std::size_t, RecordId, const Record &> ||
						  std::is_invocable_v<Func &, RecordId, const Record &> ||
						  std::is_invocable_v<Func &, std::size_t, RecordId> ||
						  std::is_invocable_v<Func &, RecordId>,
					  "representative callback must accept (chunk_index, id, record), "
					  "(id, record), (chunk_index, id), or (id)");
	}
}

template <typename Func, typename Distance>
auto invoke_representative_pair(Func &callback, std::size_t lhs_chunk, std::size_t rhs_chunk, RecordId lhs,
								RecordId rhs, Distance distance) -> void
{
	if constexpr (std::is_invocable_v<Func &, std::size_t, std::size_t, RecordId, RecordId, Distance>) {
		callback(lhs_chunk, rhs_chunk, lhs, rhs, distance);
	} else if constexpr (std::is_invocable_v<Func &, RecordId, RecordId, Distance>) {
		callback(lhs, rhs, distance);
	} else {
		static_assert(std::is_invocable_v<Func &, std::size_t, std::size_t, RecordId, RecordId, Distance> ||
						  std::is_invocable_v<Func &, RecordId, RecordId, Distance>,
					  "representative pair callback must accept "
					  "(lhs_chunk, rhs_chunk, lhs, rhs, distance) or (lhs, rhs, distance)");
	}
}

} // namespace detail

struct chunked_work_diagnostics {
	std::string representation{"chunked_space_view"};
	bool exact{true};
	std::size_t record_count{};
	std::size_t chunk_size{};
	std::size_t chunk_count{};
	std::size_t max_local_record_count{};
	std::size_t local_pair_distance_evaluations{};
	std::size_t dense_pair_distance_evaluations{};
	std::size_t representative_count{};
	std::size_t representative_pair_distance_evaluations{};
	std::size_t refinement_chunk_pair_count{};
	std::size_t refinement_candidate_pair_count{};
	std::size_t refinement_pair_distance_evaluations{};
	std::size_t bounded_pair_distance_evaluations{};
	core::SpaceVersion space_version{};
	core::SpaceVersion built_for_version{};
	bool stale{};
	std::string exactness{"exact"};
	std::vector<RecordId> source_record_ids;
	std::vector<RecordId> representative_ids;
	std::vector<std::string> warnings;
};

template <typename Distance> struct chunked_refinement_chunk_pair {
	using distance_type = Distance;

	std::size_t lhs_chunk{};
	std::size_t rhs_chunk{};
	RecordId lhs_representative_id{};
	RecordId rhs_representative_id{};
	Distance representative_distance{};
};

template <typename Distance> struct chunked_neighbor_operator_result {
	using distance_type = Distance;
	using neighbor_type = core::Neighbor<Distance>;
	using refinement_chunk_pair_type = chunked_refinement_chunk_pair<Distance>;

	std::string operation;
	std::string representation{"chunked_space_view"};
	std::string scope;
	bool exact{false};
	bool exact_within_scope{true};
	bool globally_exhaustive{false};
	std::string exactness{"non_exact"};
	std::size_t requested_count{};
	std::size_t requested_candidate_chunks_per_chunk{};
	std::size_t record_count{};
	std::size_t chunk_size{};
	std::size_t chunk_count{};
	std::size_t representative_count{};
	std::size_t local_pair_distance_evaluations{};
	std::size_t representative_pair_distance_evaluations{};
	std::size_t refinement_chunk_pair_count{};
	std::size_t refinement_candidate_pair_count{};
	std::size_t refinement_pair_distance_evaluations{};
	std::size_t bounded_pair_distance_evaluations{};
	std::size_t dense_pair_distance_evaluations{};
	std::size_t returned_neighbor_count{};
	core::SpaceVersion space_version{};
	core::SpaceVersion built_for_version{};
	bool stale{};
	std::vector<std::string> warnings;
	std::vector<refinement_chunk_pair_type> refinement_chunk_pairs;
	std::unordered_map<RecordId, std::vector<neighbor_type>> neighbors_by_id;

	auto neighbors(RecordId id) const -> const std::vector<neighbor_type> &
	{
		const auto found = neighbors_by_id.find(id);
		if (found == neighbors_by_id.end()) {
			throw std::out_of_range("record id is outside the chunked operator result");
		}
		return found->second;
	}
};

template <typename Distance> struct chunked_refinement_candidate_result {
	using distance_type = Distance;
	using refinement_chunk_pair_type = chunked_refinement_chunk_pair<Distance>;

	std::string operation;
	std::string representation{"chunked_space_view"};
	std::string scope;
	bool exact{false};
	bool exact_within_scope{true};
	bool globally_exhaustive{false};
	std::string exactness{"non_exact"};
	std::size_t requested_candidate_chunks_per_chunk{};
	std::size_t record_count{};
	std::size_t chunk_size{};
	std::size_t chunk_count{};
	std::size_t representative_count{};
	std::size_t local_pair_distance_evaluations{};
	std::size_t representative_pair_distance_evaluations{};
	std::size_t refinement_chunk_pair_count{};
	std::size_t refinement_candidate_pair_count{};
	std::size_t refinement_pair_distance_evaluations{};
	std::size_t bounded_pair_distance_evaluations{};
	std::size_t dense_pair_distance_evaluations{};
	core::SpaceVersion space_version{};
	core::SpaceVersion built_for_version{};
	bool stale{};
	std::vector<std::string> warnings;
	std::vector<refinement_chunk_pair_type> refinement_chunk_pairs;
};

inline auto chunked_plan_diagnostics_for_count(std::size_t record_count, std::size_t chunk_size,
											   std::string representation = "chunked_workflow_plan")
	-> chunked_work_diagnostics
{
	if (chunk_size == 0) {
		throw std::invalid_argument("chunked workflow plan requires chunk_size greater than zero");
	}

	chunked_work_diagnostics result;
	result.representation = std::move(representation);
	result.record_count = record_count;
	result.chunk_size = chunk_size;
	result.chunk_count = detail::chunk_count_for(record_count, chunk_size);
	result.dense_pair_distance_evaluations = detail::chunked_pair_count(record_count);
	result.representative_count = result.chunk_count;
	result.representative_pair_distance_evaluations = detail::chunked_pair_count(result.representative_count);
	for (std::size_t chunk_index = 0; chunk_index < result.chunk_count; ++chunk_index) {
		const auto local_count = detail::local_record_count_for(record_count, chunk_size, chunk_index);
		if (local_count > result.max_local_record_count) {
			result.max_local_record_count = local_count;
		}
		result.local_pair_distance_evaluations += detail::chunked_pair_count(local_count);
	}
	result.bounded_pair_distance_evaluations =
		result.local_pair_distance_evaluations + result.representative_pair_distance_evaluations;
	return result;
}

// Snapshot view of a finite metric space partitioned into contiguous chunks.
//
// The view copies the source RecordIds and records at construction time, and stores the source
// version it was built for. That keeps RecordId identity and local positions stable after the parent
// space mutates, while is_stale() still lets callers notice that the parent moved on.
template <typename Space> class ChunkedSpaceView {
  public:
	using space_type = Space;
	using record_type = typename space_type::record_type;
	using metric_type = typename space_type::metric_type;
	using distance_type = typename space_type::distance_type;

	class Chunk {
	  public:
		using view_type = ChunkedSpaceView<Space>;
		using record_type = typename view_type::record_type;
		using distance_type = typename view_type::distance_type;

		Chunk(const view_type *view, std::size_t chunk_index) : view_(view), chunk_index_(chunk_index) {}

		auto index() const -> std::size_t { return chunk_index_; }
		auto local_record_count() const -> std::size_t { return view_->local_record_count(chunk_index_); }
		auto record_count() const -> std::size_t { return local_record_count(); }
		auto local_pair_count() const -> std::size_t
		{
			return detail::chunked_pair_count(local_record_count());
		}

		auto global_position(std::size_t local_position) const -> std::size_t
		{
			validate_local_position(local_position);
			return start_position() + local_position;
		}

		auto id(std::size_t local_position) const -> RecordId { return view_->id(global_position(local_position)); }
		auto record(std::size_t local_position) const -> const record_type &
		{
			return view_->record_at_position(global_position(local_position));
		}

		auto representative_id() const -> RecordId { return id(0); }
		auto representative_record() const -> const record_type & { return record(0); }

		auto contains(RecordId id) const -> bool
		{
			if (!view_->contains(id)) {
				return false;
			}
			const auto position = view_->position_of(id);
			return start_position() <= position && position < end_position();
		}

		auto position_of(RecordId id) const -> std::size_t
		{
			const auto position = view_->position_of(id);
			if (position < start_position() || position >= end_position()) {
				throw std::out_of_range("record id is outside the chunk");
			}
			return position - start_position();
		}

		auto global_position_of(RecordId id) const -> std::size_t
		{
			(void)position_of(id);
			return view_->position_of(id);
		}

		auto distance(RecordId lhs, RecordId rhs) const -> distance_type
		{
			if (!contains(lhs) || !contains(rhs)) {
				throw std::out_of_range("local chunk distance requires two record ids from the chunk");
			}
			return view_->distance(lhs, rhs);
		}

		auto distance_at(std::size_t lhs_local_position, std::size_t rhs_local_position) const -> distance_type
		{
			return view_->distance(id(lhs_local_position), id(rhs_local_position));
		}

	  private:
		auto start_position() const -> std::size_t { return chunk_index_ * view_->chunk_size(); }
		auto end_position() const -> std::size_t { return start_position() + local_record_count(); }

		auto validate_local_position(std::size_t local_position) const -> void
		{
			if (local_position >= local_record_count()) {
				throw std::out_of_range("local chunk position is outside the chunk");
			}
		}

		const view_type *view_;
		std::size_t chunk_index_;
	};

	using chunk_type = Chunk;
	using refinement_chunk_pair_type = chunked_refinement_chunk_pair<distance_type>;

	explicit ChunkedSpaceView(const space_type &space, std::size_t chunk_size)
		: source_(&space), chunk_size_(validate_chunk_size(chunk_size)), version_(space.version())
	{
		ids_.reserve(space.size());
		records_.reserve(space.size());
		positions_.reserve(space.size());
		for (std::size_t position = 0; position < space.size(); ++position) {
			const auto id = space.id(position);
			ids_.push_back(id);
			records_.push_back(space.record(id));
			positions_.emplace(id, position);
		}
	}

	auto record_count() const -> std::size_t { return ids_.size(); }
	auto size() const -> std::size_t { return record_count(); }
	auto chunk_size() const -> std::size_t { return chunk_size_; }
	auto chunk_count() const -> std::size_t
	{
		return detail::chunk_count_for(record_count(), chunk_size_);
	}

	auto chunk(std::size_t chunk_index) const -> chunk_type
	{
		validate_chunk_index(chunk_index);
		return chunk_type(this, chunk_index);
	}

	auto local_record_count(std::size_t chunk_index) const -> std::size_t
	{
		validate_chunk_index(chunk_index);
		return detail::local_record_count_for(record_count(), chunk_size_, chunk_index);
	}

	auto local_pair_count(std::size_t chunk_index) const -> std::size_t
	{
		return detail::chunked_pair_count(local_record_count(chunk_index));
	}

	auto total_local_pair_count() const -> std::size_t
	{
		std::size_t total = 0;
		for (std::size_t chunk_index = 0; chunk_index < chunk_count(); ++chunk_index) {
			total += local_pair_count(chunk_index);
		}
		return total;
	}

	auto dense_pair_count() const -> std::size_t { return detail::chunked_pair_count(record_count()); }
	auto representative_count() const -> std::size_t { return chunk_count(); }
	auto representative_pair_count() const -> std::size_t
	{
		return detail::chunked_pair_count(representative_count());
	}

	auto representative_id(std::size_t chunk_index) const -> RecordId { return chunk(chunk_index).representative_id(); }
	auto representative_record(std::size_t chunk_index) const -> const record_type &
	{
		return chunk(chunk_index).representative_record();
	}

	auto representative_ids() const -> std::vector<RecordId>
	{
		std::vector<RecordId> result;
		result.reserve(representative_count());
		for (std::size_t chunk_index = 0; chunk_index < chunk_count(); ++chunk_index) {
			result.push_back(representative_id(chunk_index));
		}
		return result;
	}

	auto plan_diagnostics() const -> chunked_work_diagnostics
	{
		auto result =
			chunked_plan_diagnostics_for_count(record_count(), chunk_size_, "chunked_space_view");
		result.space_version = source_version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.source_record_ids = ids_;
		result.representative_ids = representative_ids();
		if (result.stale) {
			result.warnings.push_back("chunked space view was built for an older metric-space version");
		}
		return result;
	}

	auto diagnostics() const -> chunked_work_diagnostics { return plan_diagnostics(); }

	auto id(std::size_t global_position) const -> RecordId
	{
		validate_global_position(global_position);
		return ids_[global_position];
	}

	auto global_position(RecordId id) const -> std::size_t { return position_of(id); }

	auto contains(RecordId id) const -> bool { return positions_.find(id) != positions_.end(); }

	auto position_of(RecordId id) const -> std::size_t
	{
		const auto found = positions_.find(id);
		if (found == positions_.end()) {
			throw std::out_of_range("record id is outside the chunked space view");
		}
		return found->second;
	}

	auto record(RecordId id) const -> const record_type & { return record_at_position(position_of(id)); }
	auto record_at_position(std::size_t global_position) const -> const record_type &
	{
		validate_global_position(global_position);
		return records_[global_position];
	}

	auto distance(RecordId lhs, RecordId rhs) const -> distance_type
	{
		return source_->metric()(record(lhs), record(rhs));
	}

	auto version() const -> core::SpaceVersion { return version_; }
	auto built_for_version() const -> core::SpaceVersion { return version_; }
	auto source_version() const -> core::SpaceVersion { return source_->version(); }
	auto is_stale() const -> bool { return source_->version() != version_; }
	auto source_record_ids() const -> const std::vector<RecordId> & { return ids_; }

	template <typename Func> auto for_each_chunk(Func &&callback) const -> std::size_t
	{
		auto &&callback_ref = callback;
		for (std::size_t chunk_index = 0; chunk_index < chunk_count(); ++chunk_index) {
			callback_ref(chunk(chunk_index));
		}
		return chunk_count();
	}

	template <typename Func> auto for_each_local_pair(std::size_t chunk_index, Func &&callback) const -> std::size_t
	{
		const auto local_count = local_record_count(chunk_index);
		std::size_t visited = 0;
		auto &&callback_ref = callback;
		for (std::size_t lhs = 0; lhs < local_count; ++lhs) {
			const auto lhs_id = id(chunk_index * chunk_size_ + lhs);
			for (std::size_t rhs = lhs + 1; rhs < local_count; ++rhs) {
				const auto rhs_id = id(chunk_index * chunk_size_ + rhs);
				detail::invoke_local_pair(callback_ref, chunk_index, lhs_id, rhs_id, distance(lhs_id, rhs_id));
				++visited;
			}
		}
		return visited;
	}

	template <typename Func> auto for_each_local_pair(Func &&callback) const -> std::size_t
	{
		std::size_t visited = 0;
		for (std::size_t chunk_index = 0; chunk_index < chunk_count(); ++chunk_index) {
			visited += for_each_local_pair(chunk_index, callback);
		}
		return visited;
	}

	template <typename Func> auto for_each_representative(Func &&callback) const -> std::size_t
	{
		auto &&callback_ref = callback;
		for (std::size_t chunk_index = 0; chunk_index < chunk_count(); ++chunk_index) {
			detail::invoke_representative(
				callback_ref, chunk_index, representative_id(chunk_index), representative_record(chunk_index));
		}
		return representative_count();
	}

	template <typename Func> auto for_each_representative_pair(Func &&callback) const -> std::size_t
	{
		std::size_t visited = 0;
		auto &&callback_ref = callback;
		for (std::size_t lhs_chunk = 0; lhs_chunk < chunk_count(); ++lhs_chunk) {
			const auto lhs_id = representative_id(lhs_chunk);
			for (std::size_t rhs_chunk = lhs_chunk + 1; rhs_chunk < chunk_count(); ++rhs_chunk) {
				const auto rhs_id = representative_id(rhs_chunk);
				detail::invoke_representative_pair(
					callback_ref, lhs_chunk, rhs_chunk, lhs_id, rhs_id, distance(lhs_id, rhs_id));
				++visited;
			}
		}
		return visited;
	}

	auto local_neighbors(std::size_t count) const -> chunked_neighbor_operator_result<distance_type>
	{
		auto result = make_neighbor_result("local_neighbors", "local_chunks", count);
		if (count == 0) {
			return result;
		}

		result.local_pair_distance_evaluations =
			for_each_local_pair([&](std::size_t, RecordId lhs, RecordId rhs, distance_type distance) {
				result.neighbors_by_id.at(lhs).push_back({rhs, distance});
				result.neighbors_by_id.at(rhs).push_back({lhs, distance});
			});
		result.bounded_pair_distance_evaluations = result.local_pair_distance_evaluations;
		finalize_neighbor_result(result, count, true);
		return result;
	}

	template <typename Radius> auto local_range(Radius radius) const -> chunked_neighbor_operator_result<distance_type>
	{
		using comparison_type = typename std::common_type<distance_type, Radius>::type;

		auto result = make_neighbor_result("local_range", "local_chunks", 0);
		const auto threshold = static_cast<comparison_type>(radius);
		result.local_pair_distance_evaluations =
			for_each_local_pair([&](std::size_t, RecordId lhs, RecordId rhs, distance_type distance) {
				if (static_cast<comparison_type>(distance) <= threshold) {
					result.neighbors_by_id.at(lhs).push_back({rhs, distance});
					result.neighbors_by_id.at(rhs).push_back({lhs, distance});
				}
			});
		result.bounded_pair_distance_evaluations = result.local_pair_distance_evaluations;
		finalize_neighbor_result(result, 0, false);
		return result;
	}

	auto representative_neighbors(std::size_t count) const -> chunked_neighbor_operator_result<distance_type>
	{
		auto result = make_neighbor_result("representative_neighbors", "chunk_representatives", count);
		if (count == 0) {
			return result;
		}

		result.representative_pair_distance_evaluations =
			for_each_representative_pair([&](std::size_t, std::size_t, RecordId lhs, RecordId rhs,
											 distance_type distance) {
				result.neighbors_by_id.at(lhs).push_back({rhs, distance});
				result.neighbors_by_id.at(rhs).push_back({lhs, distance});
			});
		result.bounded_pair_distance_evaluations = result.representative_pair_distance_evaluations;
		finalize_neighbor_result(result, count, true);
		return result;
	}

	auto representative_candidates(std::size_t candidate_chunks_per_chunk) const
		-> chunked_refinement_candidate_result<distance_type>
	{
		auto result = make_candidate_result(
			"representative_candidates", "representative_cross_chunk_candidates", candidate_chunks_per_chunk);
		if (representative_pair_count() == 0 || candidate_chunks_per_chunk == 0) {
			finalize_candidate_result(result);
			return result;
		}

		std::vector<std::vector<representative_chunk_candidate>> candidates_by_chunk(chunk_count());
		result.representative_pair_distance_evaluations =
			for_each_representative_pair([&](std::size_t lhs_chunk, std::size_t rhs_chunk, RecordId, RecordId,
											 distance_type distance) {
				candidates_by_chunk[lhs_chunk].push_back({rhs_chunk, distance});
				candidates_by_chunk[rhs_chunk].push_back({lhs_chunk, distance});
			});

		const auto candidate_limit = std::min(candidate_chunks_per_chunk, chunk_count() - 1);
		for (std::size_t chunk_index = 0; chunk_index < candidates_by_chunk.size(); ++chunk_index) {
			auto &chunk_candidates = candidates_by_chunk[chunk_index];
			std::sort(chunk_candidates.begin(), chunk_candidates.end(), representative_chunk_candidate_less);
			if (candidate_limit < chunk_candidates.size()) {
				chunk_candidates.resize(candidate_limit);
			}
			for (const auto &candidate : chunk_candidates) {
				const auto lhs_chunk = std::min(chunk_index, candidate.chunk);
				const auto rhs_chunk = std::max(chunk_index, candidate.chunk);
				result.refinement_chunk_pairs.push_back(
					{lhs_chunk, rhs_chunk, representative_id(lhs_chunk), representative_id(rhs_chunk),
					 candidate.distance});
			}
		}

		std::sort(result.refinement_chunk_pairs.begin(), result.refinement_chunk_pairs.end(), chunk_pair_less);
		result.refinement_chunk_pairs.erase(
			std::unique(result.refinement_chunk_pairs.begin(), result.refinement_chunk_pairs.end(),
						chunk_pair_same_chunks),
			result.refinement_chunk_pairs.end());
		finalize_candidate_result(result);
		return result;
	}

	auto refined_neighbors(std::size_t count, std::size_t candidate_chunks_per_chunk) const
		-> chunked_neighbor_operator_result<distance_type>
	{
		auto result =
			make_neighbor_result("refined_neighbors", "local_chunks_plus_representative_refinement", count);
		result.requested_candidate_chunks_per_chunk = candidate_chunks_per_chunk;
		if (count == 0) {
			return result;
		}

		result.local_pair_distance_evaluations =
			for_each_local_pair([&](std::size_t, RecordId lhs, RecordId rhs, distance_type distance) {
				result.neighbors_by_id.at(lhs).push_back({rhs, distance});
				result.neighbors_by_id.at(rhs).push_back({lhs, distance});
			});

		const auto candidates = representative_candidates(candidate_chunks_per_chunk);
		apply_candidate_diagnostics(result, candidates);
		result.refinement_pair_distance_evaluations =
			refine_cross_chunk_pairs(result, candidates.refinement_chunk_pairs,
									 [](distance_type) { return true; });
		result.bounded_pair_distance_evaluations = result.local_pair_distance_evaluations +
												   result.representative_pair_distance_evaluations +
												   result.refinement_pair_distance_evaluations;
		set_neighbor_exactness(result, candidates.globally_exhaustive, "non_exact_bounded_refinement");
		finalize_neighbor_result(result, count, true);
		return result;
	}

	template <typename Radius>
	auto refined_range(Radius radius, std::size_t candidate_chunks_per_chunk) const
		-> chunked_neighbor_operator_result<distance_type>
	{
		using comparison_type = typename std::common_type<distance_type, Radius>::type;

		auto result =
			make_neighbor_result("refined_range", "local_chunks_plus_representative_refinement", 0);
		result.requested_candidate_chunks_per_chunk = candidate_chunks_per_chunk;
		const auto threshold = static_cast<comparison_type>(radius);
		result.local_pair_distance_evaluations =
			for_each_local_pair([&](std::size_t, RecordId lhs, RecordId rhs, distance_type distance) {
				if (static_cast<comparison_type>(distance) <= threshold) {
					result.neighbors_by_id.at(lhs).push_back({rhs, distance});
					result.neighbors_by_id.at(rhs).push_back({lhs, distance});
				}
			});

		const auto candidates = representative_candidates(candidate_chunks_per_chunk);
		apply_candidate_diagnostics(result, candidates);
		result.refinement_pair_distance_evaluations =
			refine_cross_chunk_pairs(result, candidates.refinement_chunk_pairs,
									 [threshold](distance_type distance) {
										 return static_cast<comparison_type>(distance) <= threshold;
									 });
		result.bounded_pair_distance_evaluations = result.local_pair_distance_evaluations +
												   result.representative_pair_distance_evaluations +
												   result.refinement_pair_distance_evaluations;
		set_neighbor_exactness(result, candidates.globally_exhaustive, "non_exact_bounded_refinement");
		finalize_neighbor_result(result, 0, false);
		return result;
	}

  private:
	struct representative_chunk_candidate {
		std::size_t chunk{};
		distance_type distance{};
	};

	static auto validate_chunk_size(std::size_t chunk_size) -> std::size_t
	{
		if (chunk_size == 0) {
			throw std::invalid_argument("chunked space view requires chunk_size greater than zero");
		}
		return chunk_size;
	}

	auto validate_chunk_index(std::size_t chunk_index) const -> void
	{
		if (chunk_index >= chunk_count()) {
			throw std::out_of_range("chunk index is outside the chunked space view");
		}
	}

	auto validate_global_position(std::size_t global_position) const -> void
	{
		if (global_position >= record_count()) {
			throw std::out_of_range("global record position is outside the chunked space view");
		}
	}

	static auto representative_chunk_candidate_less(const representative_chunk_candidate &lhs,
													const representative_chunk_candidate &rhs) -> bool
	{
		if (core::NeighborLess<distance_type>{}({RecordId::from_index(lhs.chunk), lhs.distance},
												{RecordId::from_index(rhs.chunk), rhs.distance})) {
			return true;
		}
		if (core::NeighborLess<distance_type>{}({RecordId::from_index(rhs.chunk), rhs.distance},
												{RecordId::from_index(lhs.chunk), lhs.distance})) {
			return false;
		}
		return lhs.chunk < rhs.chunk;
	}

	static auto chunk_pair_less(const refinement_chunk_pair_type &lhs, const refinement_chunk_pair_type &rhs) -> bool
	{
		if (lhs.lhs_chunk != rhs.lhs_chunk) {
			return lhs.lhs_chunk < rhs.lhs_chunk;
		}
		return lhs.rhs_chunk < rhs.rhs_chunk;
	}

	static auto chunk_pair_same_chunks(const refinement_chunk_pair_type &lhs, const refinement_chunk_pair_type &rhs)
		-> bool
	{
		return lhs.lhs_chunk == rhs.lhs_chunk && lhs.rhs_chunk == rhs.rhs_chunk;
	}

	auto make_neighbor_result(std::string operation, std::string scope, std::size_t requested_count) const
		-> chunked_neighbor_operator_result<distance_type>
	{
		const auto plan = plan_diagnostics();
		chunked_neighbor_operator_result<distance_type> result;
		result.operation = std::move(operation);
		result.scope = std::move(scope);
		result.requested_count = requested_count;
		result.record_count = plan.record_count;
		result.chunk_size = plan.chunk_size;
		result.chunk_count = plan.chunk_count;
		result.representative_count = plan.representative_count;
		result.dense_pair_distance_evaluations = plan.dense_pair_distance_evaluations;
		result.space_version = plan.space_version;
		result.built_for_version = plan.built_for_version;
		result.stale = plan.stale;
		result.warnings = plan.warnings;
		result.neighbors_by_id.reserve(ids_.size());
		for (const auto id : ids_) {
			result.neighbors_by_id.emplace(id, std::vector<core::Neighbor<distance_type>>{});
		}
		return result;
	}

	auto make_candidate_result(std::string operation, std::string scope,
							   std::size_t requested_candidate_chunks_per_chunk) const
		-> chunked_refinement_candidate_result<distance_type>
	{
		const auto plan = plan_diagnostics();
		chunked_refinement_candidate_result<distance_type> result;
		result.operation = std::move(operation);
		result.scope = std::move(scope);
		result.requested_candidate_chunks_per_chunk = requested_candidate_chunks_per_chunk;
		result.record_count = plan.record_count;
		result.chunk_size = plan.chunk_size;
		result.chunk_count = plan.chunk_count;
		result.representative_count = plan.representative_count;
		result.dense_pair_distance_evaluations = plan.dense_pair_distance_evaluations;
		result.space_version = plan.space_version;
		result.built_for_version = plan.built_for_version;
		result.stale = plan.stale;
		result.warnings = plan.warnings;
		return result;
	}

	static auto finalize_neighbor_result(chunked_neighbor_operator_result<distance_type> &result,
										 std::size_t count, bool limit_to_count) -> void
	{
		result.returned_neighbor_count = 0;
		for (auto &entry : result.neighbors_by_id) {
			if (limit_to_count) {
				entry.second = core::take_nearest_neighbors(std::move(entry.second), count);
			} else {
				core::sort_neighbors(entry.second);
			}
			result.returned_neighbor_count += entry.second.size();
		}
	}

	auto finalize_candidate_result(chunked_refinement_candidate_result<distance_type> &result) const -> void
	{
		result.refinement_chunk_pair_count = result.refinement_chunk_pairs.size();
		result.refinement_candidate_pair_count = 0;
		for (const auto &pair : result.refinement_chunk_pairs) {
			result.refinement_candidate_pair_count +=
				local_record_count(pair.lhs_chunk) * local_record_count(pair.rhs_chunk);
		}
		result.bounded_pair_distance_evaluations =
			result.representative_pair_distance_evaluations + result.refinement_pair_distance_evaluations;
		result.exact_within_scope = true;
		result.globally_exhaustive = result.refinement_chunk_pair_count == representative_pair_count();
		result.exact = result.globally_exhaustive;
		result.exactness =
			result.exact ? "exact_cross_chunk_candidates" : "non_exact_bounded_cross_chunk_candidates";
	}

	static auto set_neighbor_exactness(chunked_neighbor_operator_result<distance_type> &result,
									   bool globally_exhaustive, std::string non_exact_exactness) -> void
	{
		result.exact_within_scope = true;
		result.globally_exhaustive = globally_exhaustive;
		result.exact = globally_exhaustive;
		result.exactness = result.exact ? "exact" : std::move(non_exact_exactness);
	}

	static auto apply_candidate_diagnostics(
		chunked_neighbor_operator_result<distance_type> &result,
		const chunked_refinement_candidate_result<distance_type> &candidates) -> void
	{
		result.representative_pair_distance_evaluations = candidates.representative_pair_distance_evaluations;
		result.refinement_chunk_pair_count = candidates.refinement_chunk_pair_count;
		result.refinement_candidate_pair_count = candidates.refinement_candidate_pair_count;
		result.refinement_chunk_pairs = candidates.refinement_chunk_pairs;
		result.warnings.insert(result.warnings.end(), candidates.warnings.begin(), candidates.warnings.end());
	}

	template <typename Include>
	auto refine_cross_chunk_pairs(chunked_neighbor_operator_result<distance_type> &result,
								  const std::vector<refinement_chunk_pair_type> &chunk_pairs,
								  Include include) const -> std::size_t
	{
		std::size_t evaluated = 0;
		for (const auto &pair : chunk_pairs) {
			const auto lhs_chunk = chunk(pair.lhs_chunk);
			const auto rhs_chunk = chunk(pair.rhs_chunk);
			for (std::size_t lhs = 0; lhs < lhs_chunk.local_record_count(); ++lhs) {
				const auto lhs_id = lhs_chunk.id(lhs);
				for (std::size_t rhs = 0; rhs < rhs_chunk.local_record_count(); ++rhs) {
					const auto rhs_id = rhs_chunk.id(rhs);
					const auto distance_value = distance(lhs_id, rhs_id);
					++evaluated;
					if (include(distance_value)) {
						result.neighbors_by_id.at(lhs_id).push_back({rhs_id, distance_value});
						result.neighbors_by_id.at(rhs_id).push_back({lhs_id, distance_value});
					}
				}
			}
		}
		return evaluated;
	}

	const space_type *source_;
	std::size_t chunk_size_;
	core::SpaceVersion version_;
	std::vector<RecordId> ids_;
	std::vector<record_type> records_;
	std::unordered_map<RecordId, std::size_t> positions_;
};

template <typename Space, typename std::enable_if<core::RecordMetricSpaceLike_v<Space>, int>::type = 0>
auto chunked_view(const Space &space, std::size_t chunk_size) -> ChunkedSpaceView<Space>
{
	return ChunkedSpaceView<Space>(space, chunk_size);
}

template <typename Chunk> auto local_pair_count(const Chunk &chunk) -> decltype(chunk.local_pair_count())
{
	return chunk.local_pair_count();
}

template <typename Chunked> auto total_local_pair_count(const Chunked &chunks) -> decltype(chunks.total_local_pair_count())
{
	return chunks.total_local_pair_count();
}

template <typename Chunked> auto dense_pair_count(const Chunked &chunks) -> decltype(chunks.dense_pair_count())
{
	return chunks.dense_pair_count();
}

template <typename Chunked>
auto representative_ids(const Chunked &chunks) -> decltype(chunks.representative_ids())
{
	return chunks.representative_ids();
}

template <typename Chunked>
auto chunked_plan_diagnostics(const Chunked &chunks) -> decltype(chunks.plan_diagnostics())
{
	return chunks.plan_diagnostics();
}

template <typename Chunked, typename Func>
auto for_each_chunk(const Chunked &chunks, Func &&callback)
	-> decltype(chunks.for_each_chunk(std::forward<Func>(callback)))
{
	return chunks.for_each_chunk(std::forward<Func>(callback));
}

template <typename Chunked, typename Func>
auto for_each_local_pair(const Chunked &chunks, Func &&callback)
	-> decltype(chunks.for_each_local_pair(std::forward<Func>(callback)))
{
	return chunks.for_each_local_pair(std::forward<Func>(callback));
}

template <typename Chunked, typename Func>
auto for_each_representative(const Chunked &chunks, Func &&callback)
	-> decltype(chunks.for_each_representative(std::forward<Func>(callback)))
{
	return chunks.for_each_representative(std::forward<Func>(callback));
}

template <typename Chunked, typename Func>
auto for_each_representative_pair(const Chunked &chunks, Func &&callback)
	-> decltype(chunks.for_each_representative_pair(std::forward<Func>(callback)))
{
	return chunks.for_each_representative_pair(std::forward<Func>(callback));
}

template <typename Chunked>
auto local_neighbors(const Chunked &chunks, std::size_t count) -> decltype(chunks.local_neighbors(count))
{
	return chunks.local_neighbors(count);
}

template <typename Chunked, typename Radius>
auto local_range(const Chunked &chunks, Radius radius) -> decltype(chunks.local_range(radius))
{
	return chunks.local_range(radius);
}

template <typename Chunked>
auto representative_neighbors(const Chunked &chunks, std::size_t count)
	-> decltype(chunks.representative_neighbors(count))
{
	return chunks.representative_neighbors(count);
}

template <typename Chunked>
auto representative_candidates(const Chunked &chunks, std::size_t candidate_chunks_per_chunk)
	-> decltype(chunks.representative_candidates(candidate_chunks_per_chunk))
{
	return chunks.representative_candidates(candidate_chunks_per_chunk);
}

template <typename Chunked>
auto refined_neighbors(const Chunked &chunks, std::size_t count, std::size_t candidate_chunks_per_chunk)
	-> decltype(chunks.refined_neighbors(count, candidate_chunks_per_chunk))
{
	return chunks.refined_neighbors(count, candidate_chunks_per_chunk);
}

template <typename Chunked, typename Radius>
auto refined_range(const Chunked &chunks, Radius radius, std::size_t candidate_chunks_per_chunk)
	-> decltype(chunks.refined_range(radius, candidate_chunks_per_chunk))
{
	return chunks.refined_range(radius, candidate_chunks_per_chunk);
}

} // namespace mtrc::space

#endif
