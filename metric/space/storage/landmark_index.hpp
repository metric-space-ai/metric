// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_LANDMARK_INDEX_HPP
#define _METRIC_REPRESENTATIONS_LANDMARK_INDEX_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/core/neighbor.hpp>
#include <metric/record/id.hpp>
#include "diagnostics.hpp"
#include "policy.hpp"

namespace mtrc::space::storage {

inline constexpr std::size_t default_landmark_index_max_build_distance_evaluations = 1'000'000;
inline constexpr std::size_t default_landmark_index_max_memory_bytes = 512ULL * 1024ULL * 1024ULL;

struct landmark_index_options {
	std::size_t landmark_count{default_landmark_index_landmarks};
	std::size_t candidate_limit{default_landmark_index_candidates};
	// Set to 0 only when the caller intentionally opts into an unbounded exact landmark projection build.
	std::size_t max_build_distance_evaluations{default_landmark_index_max_build_distance_evaluations};
	// Set to 0 only when the caller intentionally opts into unbounded landmark projection storage.
	std::size_t max_memory_bytes{default_landmark_index_max_memory_bytes};
	runtime_guard runtime;
};

inline auto landmark_index_options_from_policy(std::size_t landmark_count, std::size_t candidate_limit,
											   policy runtime_policy, runtime_guard runtime = {})
	-> landmark_index_options
{
	return landmark_index_options{
		landmark_count,
		candidate_limit,
		runtime_policy.max_distance_evaluations() == 0
			? default_landmark_index_max_build_distance_evaluations
			: runtime_policy.max_distance_evaluations(),
		runtime_policy.max_memory_bytes() == 0 ? default_landmark_index_max_memory_bytes
											   : runtime_policy.max_memory_bytes(),
		runtime};
}

struct landmark_recall_calibration {
	bool measured{false};
	std::size_t sample_query_count{};
	std::size_t reference_candidate_count{};
	std::size_t reference_count{};
	std::size_t matched_count{};
	std::size_t distance_evaluations{};
	double recall{};
	double standard_error{};
	double confidence_radius_95{};
};

struct landmark_index_refresh_report {
	bool refreshed{false};
	bool rebuild_required{false};
	bool rebuild_recommended{false};
	bool no_op{false};
	std::size_t old_size{};
	std::size_t new_size{};
	std::size_t appended{};
	std::size_t version_before{};
	std::size_t version_after{};
	std::size_t landmark_count{};
	std::size_t distance_evaluations{};
	std::string reason;
	std::vector<std::string> warnings;

	auto changed() const -> bool { return appended > 0; }
};

template <typename Space> class LandmarkIndex {
  public:
	using space_type = Space;
	using record_type = typename space_type::record_type;
	using distance_type = typename space_type::distance_type;
	using neighbor_type = Neighbor<distance_type>;

	explicit LandmarkIndex(const space_type &space,
						   std::size_t landmark_count = default_landmark_index_landmarks,
						   std::size_t candidate_limit = default_landmark_index_candidates,
						   runtime_guard runtime = {})
		: LandmarkIndex(
			  space, landmark_index_options{landmark_count, candidate_limit,
											default_landmark_index_max_build_distance_evaluations,
											default_landmark_index_max_memory_bytes, runtime})
	{
	}

	explicit LandmarkIndex(const space_type &space, landmark_index_options options)
		: space_(&space), requested_landmark_count_(options.landmark_count == 0 ? default_landmark_index_landmarks
																				: options.landmark_count),
		  candidate_limit_(options.candidate_limit == 0 ? default_landmark_index_candidates : options.candidate_limit),
		  record_count_(space.size()), version_(space.version()), metric_key_(core::metric_cache_key(space.metric()))
	{
		constexpr auto law = core::metric_traits<typename space_type::metric_type>::law;
		if constexpr (law != core::metric_law::metric && law != core::metric_law::pseudo_metric) {
			throw RepresentationError(
				"landmark index requires a metric or pseudo-metric so triangle-inequality bounds are valid");
		}
		max_build_distance_evaluations_ = options.max_build_distance_evaluations;
		max_memory_bytes_ = options.max_memory_bytes;
		enforce_build_budget();

		ids_ = mtrc::record_ids(space);
		records_ = mtrc::records_for_record_ids(space, ids_);
		cache_key_ = representation_cache_key(
			"landmark_index", metric_key_, version_, ids_,
			{{"landmarks", std::to_string(requested_landmark_count_)},
			 {"candidates", std::to_string(candidate_limit_)}});
		build(options.runtime);
	}

	auto knn(const record_type &query, std::size_t k, std::size_t candidate_limit = 0,
			 runtime_guard runtime = {}) const -> std::vector<neighbor_type>
	{
		if (k == 0 || record_count_ == 0) {
			return {};
		}
		const auto query_landmarks = query_landmark_distances(query, runtime);
		const auto positions = candidate_positions_by_bound(
			query_landmarks, bounded_candidate_limit(candidate_limit, record_count_),
			[](RecordId, std::size_t) { return true; }, runtime, "landmark knn candidate ranking");
		return exact_neighbors_for_record_query(query, positions, k, runtime);
	}

	auto knn(RecordId query_id, std::size_t k, std::size_t candidate_limit = 0,
			 runtime_guard runtime = {}) const -> std::vector<neighbor_type>
	{
		if (k == 0 || record_count_ == 0) {
			return {};
		}
		const auto query_position = position_of(query_id);
		const auto query_landmarks = query_landmark_distances(query_id, runtime);
		const auto available = record_count_ > 0 ? record_count_ - 1 : 0;
		const auto positions = candidate_positions_by_bound(
			query_landmarks, bounded_candidate_limit(candidate_limit, available),
			[query_id](RecordId id, std::size_t) { return id != query_id; }, runtime,
			"landmark knn candidate ranking");
		(void)query_position;
		return exact_neighbors_for_id_query(query_id, positions, k, runtime);
	}

	template <typename Radius>
	auto range(const record_type &query, Radius radius, std::size_t candidate_limit = 0,
			   runtime_guard runtime = {}) const -> std::vector<neighbor_type>
	{
		if (record_count_ == 0) {
			return {};
		}
		const auto query_landmarks = query_landmark_distances(query, runtime);
		const auto positions = range_candidate_positions_by_bound(
			query_landmarks, radius, bounded_candidate_limit(candidate_limit, record_count_),
			[](RecordId, std::size_t) { return true; }, runtime, "landmark range candidate ranking");
		return exact_range_for_record_query(query, positions, radius, runtime);
	}

	template <typename Radius>
	auto range(RecordId query_id, Radius radius, std::size_t candidate_limit = 0,
			   runtime_guard runtime = {}) const -> std::vector<neighbor_type>
	{
		if (record_count_ == 0) {
			return {};
		}
		(void)position_of(query_id);
		const auto query_landmarks = query_landmark_distances(query_id, runtime);
		const auto available = record_count_ > 0 ? record_count_ - 1 : 0;
		const auto positions = range_candidate_positions_by_bound(
			query_landmarks, radius, bounded_candidate_limit(candidate_limit, available),
			[query_id](RecordId id, std::size_t) { return id != query_id; }, runtime,
			"landmark range candidate ranking");
		return exact_range_for_id_query(query_id, positions, radius, runtime);
	}

	auto landmark_count() const -> std::size_t { return landmark_positions_.size(); }
	auto requested_landmark_count() const -> std::size_t { return requested_landmark_count_; }
	auto candidate_limit() const -> std::size_t { return candidate_limit_; }
	auto record_count() const -> std::size_t { return record_count_; }
	auto max_build_distance_evaluations() const -> std::size_t { return max_build_distance_evaluations_; }
	auto max_memory_bytes() const -> std::size_t { return max_memory_bytes_; }
	auto build_distance_evaluations() const -> std::size_t { return build_distance_evaluations_; }
	auto maintenance_distance_evaluations() const -> std::size_t { return maintenance_distance_evaluations_; }
	auto total_distance_evaluations() const -> std::size_t
	{
		return build_distance_evaluations_ + maintenance_distance_evaluations_;
	}
	auto id(std::size_t position) const -> RecordId
	{
		validate_position(position);
		return ids_[position];
	}
	auto position_of(RecordId id) const -> std::size_t
	{
		return mtrc::position_of_record_id(ids_, id, "record id is outside the landmark index");
	}
	auto contains(RecordId id) const -> bool { return mtrc::contains_record_id(ids_, id); }
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }
	auto metric_key() const -> const std::string & { return metric_key_; }
	auto cache_key() const -> const std::string & { return cache_key_; }
	auto source_record_ids() const -> const std::vector<RecordId> & { return ids_; }
	auto landmark_record_ids() const -> std::vector<RecordId>
	{
		std::vector<RecordId> result;
		result.reserve(landmark_positions_.size());
		for (const auto position : landmark_positions_) {
			result.push_back(ids_[position]);
		}
		return result;
	}

	auto stats() const -> landmark_index_stats
	{
		landmark_index_stats result;
		result.nodes = record_count_;
		result.landmarks = landmark_count();
		result.candidate_limit = candidate_limit_;
		result.build_distance_evaluations = total_distance_evaluations();
		return result;
	}

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::landmark_index, exactness::approximate,
										  materialization::materialized, update_mode::snapshot};
		result.space_version = space_->version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.records = record_count_;
		result.distance_evaluations = total_distance_evaluations();
		result.cached_distances = record_count_ * landmark_count();
		result.memory_bytes_estimate = ids_.size() * sizeof(RecordId) + records_.size() * sizeof(record_type) +
									   landmark_positions_.size() * sizeof(std::size_t);
		for (const auto &row : landmark_distances_) {
			result.memory_bytes_estimate += row.size() * sizeof(distance_type);
		}
		result.cache_key = cache_key_;
		result.metric_key = metric_key_;
		result.source_record_ids = ids_;
		if (result.stale) {
			result.warnings.push_back("landmark index was built for an older metric-space version");
		}
		return result;
	}

	auto query_landmark_distance_count() const -> std::size_t { return landmark_count(); }

	auto refresh_after_append(runtime_guard runtime = {}) -> landmark_index_refresh_report
	{
		return refresh_after_append_with_ids(mtrc::record_ids(*space_), runtime);
	}

	template <typename AppendReport>
	auto refresh_after_append(const AppendReport &append_report, runtime_guard runtime = {})
		-> landmark_index_refresh_report
	{
		auto current_ids = mtrc::record_ids(*space_);
		auto report = make_refresh_report();
		report.new_size = current_ids.size();
		report.version_after = space_->version();

		if (append_report.version_before != version_) {
			report.rebuild_required = true;
			report.reason =
				"append report does not start from the landmark index version; rebuild required before refresh";
			return report;
		}
		if (append_report.version_after != space_->version()) {
			report.rebuild_required = true;
			report.reason =
				"append report is no longer current for the source space; rebuild required before refresh";
			return report;
		}
		if (current_ids.size() < ids_.size()) {
			report.rebuild_required = true;
			report.reason = "source space shrank after the append report; rebuild required";
			return report;
		}

		const auto expected_appended = current_ids.size() - ids_.size();
		if (append_report.appended != expected_appended ||
			append_report.appended_ids.size() != expected_appended) {
			report.rebuild_required = true;
			report.reason =
				"append report does not match the source space suffix; rebuild required";
			return report;
		}
		for (std::size_t offset = 0; offset < expected_appended; ++offset) {
			if (append_report.appended_ids[offset] != current_ids[ids_.size() + offset]) {
				report.rebuild_required = true;
				report.reason =
					"append report RecordIds do not match the source space suffix; rebuild required";
				return report;
			}
		}

		return refresh_after_append_with_ids(current_ids, runtime);
	}

	auto calibrate_knn_recall(std::size_t requested_count, std::size_t candidate_limit,
							  std::size_t sample_query_count,
							  std::size_t reference_candidate_count,
							  runtime_guard runtime = {}) const -> landmark_recall_calibration
	{
		landmark_recall_calibration result;
		if (requested_count == 0 || record_count_ < 2 || sample_query_count == 0 ||
			reference_candidate_count == 0) {
			return result;
		}

		const auto query_positions = calibration_query_positions(sample_query_count);
		double query_recall_mean = 0.0;
		double query_recall_m2 = 0.0;
		for (const auto query_position : query_positions) {
			runtime.throw_if_cancelled("landmark knn recall calibration");
			const auto query_id = ids_[query_position];
			const auto query_landmarks = query_landmark_distances(query_id, runtime);
			result.distance_evaluations += query_landmarks.size();
			const auto include = [query_id](RecordId id, std::size_t) { return id != query_id; };
			const auto available = record_count_ > 0 ? record_count_ - 1 : 0;
			const auto candidate_positions = candidate_positions_by_bound(
				query_landmarks, bounded_candidate_limit(candidate_limit, available), include, runtime,
				"landmark knn recall candidate ranking");
			const auto reference_positions = candidate_positions_by_bound(
				query_landmarks, bounded_candidate_limit(reference_candidate_count, available), include, runtime,
				"landmark knn recall reference ranking");
			auto candidates = exact_neighbors_for_id_query(query_id, candidate_positions, requested_count, runtime);
			auto reference = exact_neighbors_for_id_query(query_id, reference_positions, requested_count, runtime);
			result.distance_evaluations += candidate_positions.size() + reference_positions.size();
			result.reference_candidate_count += reference_positions.size();
			result.reference_count += reference.size();
			const auto matched = core::neighbor_id_overlap_count(reference, candidates);
			result.matched_count += matched;
			++result.sample_query_count;
			const auto query_recall =
				reference.empty() ? 1.0 : static_cast<double>(matched) / static_cast<double>(reference.size());
			const auto delta = query_recall - query_recall_mean;
			query_recall_mean += delta / static_cast<double>(result.sample_query_count);
			query_recall_m2 += delta * (query_recall - query_recall_mean);
		}
		result.measured = result.sample_query_count > 0;
		result.recall = result.reference_count == 0
							? 1.0
							: static_cast<double>(result.matched_count) /
								  static_cast<double>(result.reference_count);
		if (result.sample_query_count > 1) {
			const auto sample_variance = query_recall_m2 / static_cast<double>(result.sample_query_count - 1);
			result.standard_error = std::sqrt(sample_variance / static_cast<double>(result.sample_query_count));
			result.confidence_radius_95 = std::min(1.0, 1.96 * result.standard_error);
		}
		return result;
	}

	template <typename Radius>
	auto calibrate_range_recall(Radius radius, std::size_t candidate_limit,
								std::size_t sample_query_count,
								std::size_t reference_candidate_count,
								runtime_guard runtime = {}) const -> landmark_recall_calibration
	{
		landmark_recall_calibration result;
		if (record_count_ < 2 || sample_query_count == 0 || reference_candidate_count == 0) {
			return result;
		}

		const auto query_positions = calibration_query_positions(sample_query_count);
		double query_recall_mean = 0.0;
		double query_recall_m2 = 0.0;
		for (const auto query_position : query_positions) {
			runtime.throw_if_cancelled("landmark range recall calibration");
			const auto query_id = ids_[query_position];
			const auto query_landmarks = query_landmark_distances(query_id, runtime);
			result.distance_evaluations += query_landmarks.size();
			const auto include = [query_id](RecordId id, std::size_t) { return id != query_id; };
			const auto available = record_count_ > 0 ? record_count_ - 1 : 0;
			const auto candidate_positions = range_candidate_positions_by_bound(
				query_landmarks, radius, bounded_candidate_limit(candidate_limit, available), include, runtime,
				"landmark range recall candidate ranking");
			const auto reference_positions = range_candidate_positions_by_bound(
				query_landmarks, radius, bounded_candidate_limit(reference_candidate_count, available), include,
				runtime, "landmark range recall reference ranking");
			auto candidates = exact_range_for_id_query(query_id, candidate_positions, radius, runtime);
			auto reference = exact_range_for_id_query(query_id, reference_positions, radius, runtime);
			result.distance_evaluations += candidate_positions.size() + reference_positions.size();
			result.reference_candidate_count += reference_positions.size();
			result.reference_count += reference.size();
			const auto matched = core::neighbor_id_overlap_count(reference, candidates);
			result.matched_count += matched;
			++result.sample_query_count;
			const auto query_recall =
				reference.empty() ? 1.0 : static_cast<double>(matched) / static_cast<double>(reference.size());
			const auto delta = query_recall - query_recall_mean;
			query_recall_mean += delta / static_cast<double>(result.sample_query_count);
			query_recall_m2 += delta * (query_recall - query_recall_mean);
		}
		result.measured = result.sample_query_count > 0;
		result.recall = result.reference_count == 0
							? 1.0
							: static_cast<double>(result.matched_count) /
								  static_cast<double>(result.reference_count);
		if (result.sample_query_count > 1) {
			const auto sample_variance = query_recall_m2 / static_cast<double>(result.sample_query_count - 1);
			result.standard_error = std::sqrt(sample_variance / static_cast<double>(result.sample_query_count));
			result.confidence_radius_95 = std::min(1.0, 1.96 * result.standard_error);
		}
		return result;
	}

  private:
	struct bounded_candidate {
		std::size_t position{};
		RecordId id{};
		distance_type lower_bound{};
	};

	static auto checked_size_product(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
	{
		if (rhs != 0 && lhs > std::numeric_limits<std::size_t>::max() / rhs) {
			throw RepresentationError(message);
		}
		return lhs * rhs;
	}

	static auto checked_size_sum(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
	{
		if (lhs > std::numeric_limits<std::size_t>::max() - rhs) {
			throw RepresentationError(message);
		}
		return lhs + rhs;
	}

	auto build_landmark_count() const -> std::size_t
	{
		return std::min(requested_landmark_count_, record_count_);
	}

	auto build_distance_evaluation_estimate() const -> std::size_t
	{
		if (record_count_ < 2) {
			return 0;
		}
		return checked_size_product(build_landmark_count(), record_count_ - 1,
									"landmark index build distance-evaluation estimate exceeds size_t capacity");
	}

	auto build_memory_estimate_bytes() const -> std::size_t
	{
		const auto landmarks = build_landmark_count();
		const auto projection_cells = checked_size_product(
			record_count_, landmarks, "landmark index build memory estimate exceeds size_t capacity");
		auto total = checked_size_product(record_count_, sizeof(RecordId),
										  "landmark index build memory estimate exceeds size_t capacity");
		total = checked_size_sum(
			total, checked_size_product(record_count_, sizeof(record_type),
										"landmark index build memory estimate exceeds size_t capacity"),
			"landmark index build memory estimate exceeds size_t capacity");
		total = checked_size_sum(
			total, checked_size_product(landmarks, sizeof(std::size_t),
										"landmark index build memory estimate exceeds size_t capacity"),
			"landmark index build memory estimate exceeds size_t capacity");
		total = checked_size_sum(
			total, checked_size_product(record_count_, sizeof(std::vector<distance_type>),
										"landmark index build memory estimate exceeds size_t capacity"),
			"landmark index build memory estimate exceeds size_t capacity");
		total = checked_size_sum(
			total, checked_size_product(projection_cells, sizeof(distance_type),
										"landmark index build memory estimate exceeds size_t capacity"),
			"landmark index build memory estimate exceeds size_t capacity");
		return total;
	}

	auto enforce_build_budget() const -> void
	{
		const auto estimated_distance_evaluations = build_distance_evaluation_estimate();
		if (max_build_distance_evaluations_ > 0 &&
			estimated_distance_evaluations > max_build_distance_evaluations_) {
			throw RepresentationError(
				"landmark index construction exceeds max_build_distance_evaluations: records=" +
				std::to_string(record_count_) + " requested_landmarks=" +
				std::to_string(requested_landmark_count_) + " admitted_landmarks=" +
				std::to_string(build_landmark_count()) + " estimated_distance_evaluations=" +
				std::to_string(estimated_distance_evaluations) + " max_build_distance_evaluations=" +
				std::to_string(max_build_distance_evaluations_) +
				"; reduce landmark_count/candidate_limit, use sampled search, or pass landmark_index_options with an explicit budget");
		}
		const auto estimated_memory_bytes = build_memory_estimate_bytes();
		if (max_memory_bytes_ > 0 && estimated_memory_bytes > max_memory_bytes_) {
			throw RepresentationError(
				"landmark index construction exceeds max_memory_bytes: records=" +
				std::to_string(record_count_) + " requested_landmarks=" +
				std::to_string(requested_landmark_count_) + " admitted_landmarks=" +
				std::to_string(build_landmark_count()) + " estimated_bytes=" +
				std::to_string(estimated_memory_bytes) + " max_memory_bytes=" +
				std::to_string(max_memory_bytes_) +
				"; reduce landmark_count/candidate_limit, use sampled search, or pass landmark_index_options with an explicit budget");
		}
	}

	static auto distance_gap(distance_type lhs, distance_type rhs) -> distance_type
	{
		return lhs < rhs ? static_cast<distance_type>(rhs - lhs) : static_cast<distance_type>(lhs - rhs);
	}

	auto bounded_candidate_limit(std::size_t requested, std::size_t available) const -> std::size_t
	{
		const auto preferred = requested == 0 ? candidate_limit_ : requested;
		return std::min(preferred, available);
	}

	auto calibration_query_positions(std::size_t sample_query_count) const -> std::vector<std::size_t>
	{
		const auto count = std::min(sample_query_count, record_count_);
		std::vector<std::size_t> positions;
		positions.reserve(count);
		for (std::size_t sample = 0; sample < count; ++sample) {
			positions.push_back((sample * record_count_) / count);
		}
		return positions;
	}

	auto build(runtime_guard runtime) -> void
	{
		landmark_distances_.assign(record_count_, {});
		if (record_count_ == 0) {
			return;
		}

		const auto target_landmarks = std::min(requested_landmark_count_, record_count_);
		add_landmark(0, runtime);
		while (landmark_positions_.size() < target_landmarks) {
			const auto next = farthest_record_from_landmarks();
			if (next >= record_count_) {
				break;
			}
			add_landmark(next, runtime);
		}
	}

	auto make_refresh_report() const -> landmark_index_refresh_report
	{
		landmark_index_refresh_report report;
		report.old_size = record_count_;
		report.new_size = space_->size();
		report.version_before = version_;
		report.version_after = space_->version();
		report.landmark_count = landmark_count();
		return report;
	}

	auto refresh_after_append_with_ids(const std::vector<RecordId> &current_ids, runtime_guard runtime)
		-> landmark_index_refresh_report
	{
		auto report = make_refresh_report();
		report.new_size = current_ids.size();
		report.version_after = space_->version();

		const auto refusal = append_only_refusal(current_ids);
		if (!refusal.empty()) {
			report.rebuild_required = true;
			report.reason = refusal;
			return report;
		}

		report.appended = current_ids.size() - ids_.size();
		if (report.appended == 0) {
			report.refreshed = true;
			report.no_op = true;
			report.reason = "landmark index is already current";
			return report;
		}

		if (record_count_ == 0 && landmark_positions_.empty()) {
			const auto before = build_distance_evaluations_;
			ids_ = current_ids;
			records_ = mtrc::records_for_record_ids(*space_, ids_);
			record_count_ = ids_.size();
			version_ = space_->version();
			landmark_positions_.clear();
			landmark_distances_.clear();
			build(runtime);
			report.refreshed = true;
			report.distance_evaluations = build_distance_evaluations_ - before;
			report.landmark_count = landmark_count();
			report.reason = "bootstrapped landmark index after append into an empty snapshot";
			refresh_cache_key();
			return report;
		}

		if (landmark_positions_.empty()) {
			report.rebuild_required = true;
			report.reason = "landmark index has no landmarks; rebuild required before append refresh";
			return report;
		}

		ids_.reserve(current_ids.size());
		records_.reserve(current_ids.size());
		landmark_distances_.reserve(current_ids.size());
		for (std::size_t position = record_count_; position < current_ids.size(); ++position) {
			runtime.throw_if_cancelled("landmark index append refresh");
			const auto id = current_ids[position];
			ids_.push_back(id);
			records_.push_back(space_->record(id));
			std::vector<distance_type> row;
			row.reserve(landmark_positions_.size());
			for (const auto landmark_position : landmark_positions_) {
				runtime.throw_if_cancelled("landmark index append refresh");
				row.push_back(static_cast<distance_type>(
					space_->metric()(records_[landmark_position], records_.back())));
				++report.distance_evaluations;
			}
			landmark_distances_.push_back(std::move(row));
		}

		record_count_ = ids_.size();
		version_ = space_->version();
		maintenance_distance_evaluations_ += report.distance_evaluations;
		report.refreshed = true;
		report.landmark_count = landmark_count();
		report.reason = "extended existing landmark projections for appended records";
		if (landmark_count() < std::min(requested_landmark_count_, record_count_)) {
			report.rebuild_recommended = true;
			report.warnings.push_back(
				"append refresh keeps the existing landmark set; rebuild to select additional farthest-first landmarks");
		}
		refresh_cache_key();
		return report;
	}

	auto append_only_refusal(const std::vector<RecordId> &current_ids) const -> std::string
	{
		if (current_ids.size() < ids_.size()) {
			return "source space shrank since the landmark snapshot; rebuild required";
		}
		for (std::size_t position = 0; position < ids_.size(); ++position) {
			if (current_ids[position] != ids_[position]) {
				return "source RecordIds no longer match the landmark snapshot prefix; rebuild required";
			}
		}
		if (space_->version() < version_) {
			return "source space version moved behind the landmark snapshot; rebuild required";
		}
		const auto appended = current_ids.size() - ids_.size();
		const auto version_delta = space_->version() - version_;
		if (version_delta != appended) {
			return "source space version changed without a matching append-only suffix; rebuild required";
		}
		return {};
	}

	auto refresh_cache_key() -> void
	{
		cache_key_ = representation_cache_key(
			"landmark_index", metric_key_, version_, ids_,
			{{"landmarks", std::to_string(requested_landmark_count_)},
			 {"candidates", std::to_string(candidate_limit_)}});
	}

	auto is_landmark_position(std::size_t position) const -> bool
	{
		return std::find(landmark_positions_.begin(), landmark_positions_.end(), position) != landmark_positions_.end();
	}

	auto farthest_record_from_landmarks() const -> std::size_t
	{
		std::size_t best_position = record_count_;
		distance_type best_distance{};
		for (std::size_t position = 0; position < record_count_; ++position) {
			if (is_landmark_position(position)) {
				continue;
			}
			const auto nearest = nearest_landmark_distance(position);
			if (best_position == record_count_ || best_distance < nearest ||
				(!(nearest < best_distance) && ids_[position] < ids_[best_position])) {
				best_position = position;
				best_distance = nearest;
			}
		}
		return best_position;
	}

	auto nearest_landmark_distance(std::size_t position) const -> distance_type
	{
		if (landmark_distances_[position].empty()) {
			return distance_type{};
		}
		auto nearest = landmark_distances_[position].front();
		for (const auto distance : landmark_distances_[position]) {
			if (distance < nearest) {
				nearest = distance;
			}
		}
		return nearest;
	}

	auto add_landmark(std::size_t landmark_position, runtime_guard runtime) -> void
	{
		landmark_positions_.push_back(landmark_position);
		for (std::size_t position = 0; position < record_count_; ++position) {
			runtime.throw_if_cancelled("landmark index construction");
			if (position == landmark_position) {
				landmark_distances_[position].push_back(distance_type{});
				continue;
			}
			landmark_distances_[position].push_back(static_cast<distance_type>(
				space_->metric()(records_[landmark_position], records_[position])));
			++build_distance_evaluations_;
		}
	}

	auto query_landmark_distances(const record_type &query, runtime_guard runtime) const -> std::vector<distance_type>
	{
		std::vector<distance_type> distances;
		distances.reserve(landmark_positions_.size());
		for (const auto landmark_position : landmark_positions_) {
			runtime.throw_if_cancelled("landmark query projection");
			distances.push_back(static_cast<distance_type>(space_->metric()(query, records_[landmark_position])));
		}
		return distances;
	}

	auto query_landmark_distances(RecordId query_id, runtime_guard runtime) const -> std::vector<distance_type>
	{
		std::vector<distance_type> distances;
		distances.reserve(landmark_positions_.size());
		for (const auto landmark_position : landmark_positions_) {
			runtime.throw_if_cancelled("landmark query projection");
			const auto landmark_id = ids_[landmark_position];
			if (landmark_id == query_id) {
				distances.push_back(distance_type{});
			} else {
				distances.push_back(static_cast<distance_type>(space_->distance(query_id, landmark_id)));
			}
		}
		return distances;
	}

	auto lower_bound_for(std::size_t position, const std::vector<distance_type> &query_landmarks) const
		-> distance_type
	{
		distance_type bound{};
		const auto landmarks = std::min(query_landmarks.size(), landmark_distances_[position].size());
		for (std::size_t index = 0; index < landmarks; ++index) {
			const auto gap = distance_gap(query_landmarks[index], landmark_distances_[position][index]);
			if (bound < gap) {
				bound = gap;
			}
		}
		return bound;
	}

	template <typename Include>
	auto all_candidates_by_bound(const std::vector<distance_type> &query_landmarks, Include include,
								 runtime_guard runtime, const char *operation) const -> std::vector<bounded_candidate>
	{
		std::vector<bounded_candidate> candidates;
		candidates.reserve(record_count_);
		for (std::size_t position = 0; position < record_count_; ++position) {
			runtime.throw_if_cancelled(operation);
			const auto id = ids_[position];
			if (!include(id, position)) {
				continue;
			}
			candidates.push_back(bounded_candidate{position, id, lower_bound_for(position, query_landmarks)});
		}
		std::sort(candidates.begin(), candidates.end(), [](const auto &lhs, const auto &rhs) {
			if (lhs.lower_bound < rhs.lower_bound) {
				return true;
			}
			if (rhs.lower_bound < lhs.lower_bound) {
				return false;
			}
			return lhs.id < rhs.id;
		});
		return candidates;
	}

	template <typename Include>
	auto candidate_positions_by_bound(const std::vector<distance_type> &query_landmarks,
									  std::size_t candidate_limit, Include include, runtime_guard runtime,
									  const char *operation) const -> std::vector<std::size_t>
	{
		auto candidates = all_candidates_by_bound(query_landmarks, include, runtime, operation);
		if (candidate_limit < candidates.size()) {
			candidates.resize(candidate_limit);
		}
		std::vector<std::size_t> positions;
		positions.reserve(candidates.size());
		for (const auto &candidate : candidates) {
			positions.push_back(candidate.position);
		}
		return positions;
	}

	template <typename Radius, typename Include>
	auto range_candidate_positions_by_bound(const std::vector<distance_type> &query_landmarks, Radius radius,
											std::size_t candidate_limit, Include include,
											runtime_guard runtime, const char *operation) const
		-> std::vector<std::size_t>
	{
		using comparison_type = typename std::common_type<distance_type, Radius>::type;
		const auto threshold = static_cast<comparison_type>(radius);
		auto candidates = all_candidates_by_bound(query_landmarks, include, runtime, operation);
		candidates.erase(std::remove_if(candidates.begin(), candidates.end(), [threshold](const auto &candidate) {
							return static_cast<comparison_type>(candidate.lower_bound) > threshold;
						}),
						 candidates.end());
		if (candidate_limit < candidates.size()) {
			candidates.resize(candidate_limit);
		}
		std::vector<std::size_t> positions;
		positions.reserve(candidates.size());
		for (const auto &candidate : candidates) {
			positions.push_back(candidate.position);
		}
		return positions;
	}

	auto exact_neighbors_for_record_query(const record_type &query, const std::vector<std::size_t> &positions,
										  std::size_t k, runtime_guard runtime) const -> std::vector<neighbor_type>
	{
		std::vector<neighbor_type> candidates;
		candidates.reserve(positions.size());
		for (const auto position : positions) {
			runtime.throw_if_cancelled("landmark knn candidate refinement");
			candidates.push_back(
				neighbor_type{ids_[position], static_cast<distance_type>(space_->metric()(query, records_[position]))});
		}
		return core::take_nearest_neighbors(std::move(candidates), k);
	}

	auto exact_neighbors_for_id_query(RecordId query_id, const std::vector<std::size_t> &positions, std::size_t k,
									  runtime_guard runtime) const -> std::vector<neighbor_type>
	{
		std::vector<neighbor_type> candidates;
		candidates.reserve(positions.size());
		for (const auto position : positions) {
			runtime.throw_if_cancelled("landmark knn candidate refinement");
			const auto id = ids_[position];
			candidates.push_back(neighbor_type{id, static_cast<distance_type>(space_->distance(query_id, id))});
		}
		return core::take_nearest_neighbors(std::move(candidates), k);
	}

	template <typename Radius>
	auto exact_range_for_record_query(const record_type &query, const std::vector<std::size_t> &positions,
									  Radius radius, runtime_guard runtime) const -> std::vector<neighbor_type>
	{
		using comparison_type = typename std::common_type<distance_type, Radius>::type;
		const auto threshold = static_cast<comparison_type>(radius);
		std::vector<neighbor_type> candidates;
		candidates.reserve(positions.size());
		for (const auto position : positions) {
			runtime.throw_if_cancelled("landmark range candidate refinement");
			const auto distance = static_cast<distance_type>(space_->metric()(query, records_[position]));
			if (static_cast<comparison_type>(distance) <= threshold) {
				candidates.push_back(neighbor_type{ids_[position], distance});
			}
		}
		core::sort_neighbors(candidates);
		return candidates;
	}

	template <typename Radius>
	auto exact_range_for_id_query(RecordId query_id, const std::vector<std::size_t> &positions, Radius radius,
								  runtime_guard runtime) const -> std::vector<neighbor_type>
	{
		using comparison_type = typename std::common_type<distance_type, Radius>::type;
		const auto threshold = static_cast<comparison_type>(radius);
		std::vector<neighbor_type> candidates;
		candidates.reserve(positions.size());
		for (const auto position : positions) {
			runtime.throw_if_cancelled("landmark range candidate refinement");
			const auto id = ids_[position];
			const auto distance = static_cast<distance_type>(space_->distance(query_id, id));
			if (static_cast<comparison_type>(distance) <= threshold) {
				candidates.push_back(neighbor_type{id, distance});
			}
		}
		core::sort_neighbors(candidates);
		return candidates;
	}

	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the landmark index");
		}
	}

	const space_type *space_;
	std::size_t requested_landmark_count_;
	std::size_t candidate_limit_;
	std::size_t record_count_;
	std::size_t version_;
	std::string metric_key_;
	std::string cache_key_;
	std::vector<RecordId> ids_;
	std::vector<record_type> records_;
	std::vector<std::size_t> landmark_positions_;
	std::vector<std::vector<distance_type>> landmark_distances_;
	std::size_t max_build_distance_evaluations_{};
	std::size_t max_memory_bytes_{};
	std::size_t build_distance_evaluations_{};
	std::size_t maintenance_distance_evaluations_{};
};

template <typename Space>
auto landmark_index(const Space &space, std::size_t landmark_count = default_landmark_index_landmarks,
					std::size_t candidate_limit = default_landmark_index_candidates, runtime_guard runtime = {})
	-> LandmarkIndex<Space>
{
	return LandmarkIndex<Space>(space, landmark_count, candidate_limit, runtime);
}

template <typename Space>
auto landmark_index(const Space &space, landmark_index_options options) -> LandmarkIndex<Space>
{
	return LandmarkIndex<Space>(space, options);
}

} // namespace mtrc::space::storage

#endif
