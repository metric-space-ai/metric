// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_REPRESENTATIONS_KNN_GRAPH_INDEX_HPP
#define _METRIC_REPRESENTATIONS_KNN_GRAPH_INDEX_HPP

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
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

inline constexpr std::size_t default_knn_graph_max_build_distance_evaluations = 1'000'000;
inline constexpr std::size_t default_knn_graph_quality_sample_count = 250;

struct knn_graph_index_options {
	std::size_t neighbors{};
	// Set to 0 only when the caller intentionally opts into an unbounded exact graph build.
	std::size_t max_build_distance_evaluations{default_knn_graph_max_build_distance_evaluations};
};

struct knn_graph_index_refresh_options {
	// Set to 0 only when the caller intentionally opts into an unbounded append refresh.
	std::size_t max_refresh_distance_evaluations{default_knn_graph_max_build_distance_evaluations};
	runtime_guard runtime;
};

struct knn_graph_index_refresh_report {
	bool refreshed{false};
	bool rebuild_required{false};
	bool no_op{false};
	bool exact_rows_updated{false};
	std::size_t old_size{};
	std::size_t new_size{};
	std::size_t appended{};
	std::size_t version_before{};
	std::size_t version_after{};
	std::size_t graph_neighbors{};
	std::size_t old_row_update_distance_evaluations{};
	std::size_t appended_row_distance_evaluations{};
	std::size_t distance_evaluations{};
	std::size_t max_refresh_distance_evaluations{};
	std::string reason;
	std::vector<std::string> warnings;

	auto changed() const -> bool { return appended > 0; }
};

template <typename Space> class KnnGraphIndex {
  public:
	using space_type = Space;
	using record_type = typename space_type::record_type;
	using distance_type = typename space_type::distance_type;
	using neighbor_type = Neighbor<distance_type>;

	KnnGraphIndex(const space_type &space, std::size_t k)
		: KnnGraphIndex(space, knn_graph_index_options{k})
	{
	}

	KnnGraphIndex(const space_type &space, knn_graph_index_options options)
		: space_(&space), k_(options.neighbors), record_count_(space.size()), version_(space.version()),
		  metric_key_(core::metric_cache_key(space.metric())),
		  max_build_distance_evaluations_(options.max_build_distance_evaluations)
	{
		enforce_build_budget();
		ids_ = mtrc::record_ids(space);
		records_ = mtrc::records_for_record_ids(space, ids_);
		cache_key_ =
			representation_cache_key("knn_graph_index", metric_key_, version_, ids_, {{"k", std::to_string(k_)}});

		adjacency_.reserve(record_count_);
		if (k_ == 0) {
			adjacency_.resize(record_count_);
			return;
		}
		for (std::size_t source = 0; source < record_count_; ++source) {
			adjacency_.push_back(build_neighbors(source, &build_distance_evaluations_));
		}
	}

	auto neighbors(RecordId source) const -> const std::vector<neighbor_type> &
	{
		return adjacency_[position_of(source)];
	}

	auto knn(const record_type &query, std::size_t k) const -> std::vector<neighbor_type>
	{
		auto candidates = core::neighbor_candidates<distance_type>(
			record_count_, [this](std::size_t index) { return ids_[index]; },
			[this, &query](RecordId, std::size_t index) { return space_->metric()(query, records_[index]); });
		return core::take_nearest_neighbors(std::move(candidates), k);
	}

	auto k() const -> std::size_t { return k_; }
	auto record_count() const -> std::size_t { return record_count_; }
	auto build_distance_evaluations() const -> std::size_t { return build_distance_evaluations_; }
	auto max_build_distance_evaluations() const -> std::size_t { return max_build_distance_evaluations_; }
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
		return mtrc::position_of_record_id(ids_, id, "record id is outside the kNN graph index");
	}
	auto contains(RecordId id) const -> bool
	{
		return mtrc::contains_record_id(ids_, id);
	}
	auto version() const -> std::size_t { return version_; }
	auto is_stale() const -> bool { return space_->version() != version_; }
	auto metric_key() const -> const std::string & { return metric_key_; }
	auto cache_key() const -> const std::string & { return cache_key_; }
	auto source_record_ids() const -> const std::vector<RecordId> & { return ids_; }
	auto edge_count() const -> std::size_t
	{
		std::size_t count = 0;
		for (const auto &neighbors : adjacency_) {
			count += neighbors.size();
		}
		return count;
	}
	auto stats() const -> knn_graph_stats
	{
		knn_graph_stats result;
		result.nodes = record_count_;
		result.edges = edge_count();
		result.neighbors_requested = k_;
		return result;
	}

	template <typename Provider>
	auto stats_against(const Provider &provider, std::size_t sample_count = 0) const -> knn_graph_stats
	{
		auto result = stats();
		result.recall_validated = true;
		result.sampled_recall = quality_against(provider, sample_count).recall;
		return result;
	}

	template <typename Provider>
	auto sampled_recall(const Provider &provider, std::size_t sample_count = 0) const -> double
	{
		return quality_against(provider, sample_count).recall;
	}

	template <typename Provider>
	auto quality_against(const Provider &provider, std::size_t sample_count = 0) const -> knn_graph_quality_diagnostics
	{
		if (k_ == 0 || record_count_ <= 1) {
			knn_graph_quality_diagnostics result;
			result.record_count = record_count_;
			result.edge_count = edge_count();
			result.requested_neighbors = k_;
			result.evaluated_queries = record_count_;
			result.recall = 1.0;
			result.average_row_recall = 1.0;
			result.minimum_row_recall = 1.0;
			result.maximum_row_recall = 1.0;
			result.stale = is_stale();
			if (result.stale) {
				result.warnings.push_back("kNN graph quality was evaluated on a stale graph index");
			}
			return result;
		}

		const auto requested_samples = sample_count == 0 ? default_knn_graph_quality_sample_count : sample_count;
		const auto samples = requested_samples > record_count_ ? record_count_ : requested_samples;
		knn_graph_quality_diagnostics result;
		result.record_count = record_count_;
		result.edge_count = edge_count();
		result.requested_neighbors = k_;
		result.requested_sample_count = requested_samples;
		result.evaluated_queries = samples;
		result.evaluated_neighbor_count = k_;
		result.exact_distance_evaluations = samples * (provider.record_count() > 0 ? provider.record_count() - 1 : 0);
		result.sampled = samples < record_count_;
		result.stale = is_stale();
		if (result.sampled) {
			result.warnings.push_back(
				"kNN graph quality used sampled query rows; pass a larger sample_count to validate more rows");
		}

		core::RecallAccumulator recall;
		core::ScalarAccumulator<double> best_distance_inflation_summary;
		for (std::size_t sample = 0; sample < samples; ++sample) {
			const auto source = quality_sample_position(sample, samples, record_count_);
			const auto source_id = ids_[source];
			auto exact_neighbors = exact_provider_neighbors(provider, source_id);
			const auto &graph_neighbors = adjacency_[source];
			result.graph_edge_evaluations += graph_neighbors.size();
			const auto hits = core::neighbor_id_overlap_count(exact_neighbors, graph_neighbors);
			const auto possible = exact_neighbors.size();
			(void)recall.add(hits, possible, 1.0);

			double best_distance_inflation = 1.0;
			if (!exact_neighbors.empty() && !graph_neighbors.empty()) {
				const auto exact_best = static_cast<double>(exact_neighbors.front().distance);
				const auto graph_best = static_cast<double>(graph_neighbors.front().distance);
				if (exact_best == 0.0) {
					best_distance_inflation = graph_best == 0.0 ? 1.0 : std::numeric_limits<double>::infinity();
				} else {
					best_distance_inflation = graph_best / exact_best;
				}
			}
			best_distance_inflation_summary.add(best_distance_inflation);
		}
		result.matched_neighbors = recall.matched;
		result.possible_neighbors = recall.possible;
		result.recall = recall.total_recall(1.0);
		result.average_row_recall = recall.average_recall(1.0);
		result.minimum_row_recall = recall.minimum_recall_or(1.0);
		result.maximum_row_recall = recall.maximum_recall_or(1.0);
		result.average_best_distance_inflation = best_distance_inflation_summary.average_or(1.0);
		result.maximum_best_distance_inflation = best_distance_inflation_summary.maximum_or(1.0);
		if (result.stale) {
			result.warnings.push_back("kNN graph quality was evaluated on a stale graph index");
		}
		return result;
	}

	auto diagnostics() const -> representation_diagnostics
	{
		representation_diagnostics result{representation_kind::knn_graph_index, exactness::approximate,
										  materialization::materialized, update_mode::snapshot};
		result.space_version = space_->version();
		result.built_for_version = version_;
		result.stale = is_stale();
		result.records = record_count_;
		result.distance_evaluations = total_distance_evaluations();
		result.cached_distances = edge_count();
		result.memory_bytes_estimate = ids_.size() * sizeof(RecordId) + records_.size() * sizeof(record_type);
		for (const auto &neighbors : adjacency_) {
			result.memory_bytes_estimate += neighbors.size() * sizeof(neighbor_type);
		}
		result.cache_key = cache_key_;
		result.metric_key = metric_key_;
		result.source_record_ids = ids_;
		if (result.stale) {
			result.warnings.push_back("kNN graph index was built for an older metric-space version");
		}
		return result;
	}

	auto refresh_after_append(knn_graph_index_refresh_options options = {}) -> knn_graph_index_refresh_report
	{
		return refresh_after_append_with_ids(mtrc::record_ids(*space_), options);
	}

	template <typename AppendReport>
	auto refresh_after_append(const AppendReport &append_report, knn_graph_index_refresh_options options = {})
		-> knn_graph_index_refresh_report
	{
		auto current_ids = mtrc::record_ids(*space_);
		auto report = make_refresh_report();
		report.new_size = current_ids.size();
		report.version_after = space_->version();
		report.max_refresh_distance_evaluations = options.max_refresh_distance_evaluations;

		if (append_report.version_before != version_) {
			report.rebuild_required = true;
			report.reason = "append report does not start from the kNN graph index version; rebuild required";
			return report;
		}
		if (append_report.version_after != space_->version()) {
			report.rebuild_required = true;
			report.reason = "append report is no longer current for the source space; rebuild required";
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
			report.reason = "append report does not match the source space suffix; rebuild required";
			return report;
		}
		for (std::size_t offset = 0; offset < expected_appended; ++offset) {
			if (append_report.appended_ids[offset] != current_ids[ids_.size() + offset]) {
				report.rebuild_required = true;
				report.reason = "append report RecordIds do not match the source space suffix; rebuild required";
				return report;
			}
		}
		return refresh_after_append_with_ids(current_ids, options);
	}

  private:
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

	static auto build_distance_evaluation_estimate(std::size_t record_count, std::size_t k) -> std::size_t
	{
		if (k == 0 || record_count < 2) {
			return 0;
		}
		return checked_size_product(
			record_count, record_count - 1, "kNN graph build distance-evaluation estimate exceeds size_t capacity");
	}

	auto refresh_distance_evaluation_estimate(std::size_t old_count, std::size_t new_count,
											 std::size_t appended) const
		-> std::pair<std::size_t, std::size_t>
	{
		if (k_ == 0 || appended == 0 || new_count < 2) {
			return {0, 0};
		}
		const auto old_row_updates = checked_size_product(
			old_count, appended, "kNN graph refresh distance-evaluation estimate exceeds size_t capacity");
		const auto appended_rows = checked_size_product(
			appended, new_count - 1, "kNN graph refresh distance-evaluation estimate exceeds size_t capacity");
		return {old_row_updates, appended_rows};
	}

	static auto quality_sample_position(std::size_t sample_index, std::size_t sample_count,
									   std::size_t record_count) -> std::size_t
	{
		if (sample_count >= record_count) {
			return sample_index;
		}
		if (sample_count <= 1) {
			return record_count / 2;
		}
		return (sample_index * (record_count - 1)) / (sample_count - 1);
	}

	auto enforce_build_budget() const -> void
	{
		const auto estimated = build_distance_evaluation_estimate(record_count_, k_);
		if (max_build_distance_evaluations_ > 0 && estimated > max_build_distance_evaluations_) {
			throw RepresentationError(
				"kNN graph construction exceeds max_build_distance_evaluations: records=" +
				std::to_string(record_count_) + " graph_neighbors=" + std::to_string(k_) +
				" estimated_distance_evaluations=" + std::to_string(estimated) +
				" max_build_distance_evaluations=" + std::to_string(max_build_distance_evaluations_) +
				"; use live/exact-scan search, Landmark/Pivot search, or pass knn_graph_index_options with an explicit budget");
		}
	}

	template <typename Provider>
	auto exact_provider_neighbors(const Provider &provider, RecordId source_id) const -> std::vector<neighbor_type>
	{
		auto candidates = core::neighbor_candidates_if<distance_type>(
			provider.record_count(), [&provider](std::size_t target) { return provider.id(target); },
			[&provider, source_id](RecordId target_id, std::size_t) { return provider.distance(source_id, target_id); },
			[source_id](RecordId target_id, std::size_t) { return target_id != source_id; });
		return core::take_nearest_neighbors(std::move(candidates), k_);
	}

	auto build_neighbors(std::size_t source_position, std::size_t *distance_evaluations = nullptr,
						 runtime_guard runtime = {}) const -> std::vector<neighbor_type>
	{
		auto candidates = core::neighbor_candidates_if<distance_type>(
			record_count_, [this](std::size_t target) { return ids_[target]; },
			[this, source_position, distance_evaluations, runtime](RecordId, std::size_t target) {
				runtime.throw_if_cancelled("kNN graph row build");
				if (distance_evaluations != nullptr) {
					++(*distance_evaluations);
				}
				return space_->metric()(records_[source_position], records_[target]);
			},
			[source_position](RecordId, std::size_t target) { return target != source_position; });
		return core::take_nearest_neighbors(std::move(candidates), k_);
	}

	auto make_refresh_report() const -> knn_graph_index_refresh_report
	{
		knn_graph_index_refresh_report report;
		report.old_size = record_count_;
		report.new_size = space_->size();
		report.version_before = version_;
		report.version_after = space_->version();
		report.graph_neighbors = k_;
		return report;
	}

	auto refresh_after_append_with_ids(const std::vector<RecordId> &current_ids,
									  knn_graph_index_refresh_options options)
		-> knn_graph_index_refresh_report
	{
		auto report = make_refresh_report();
		report.new_size = current_ids.size();
		report.version_after = space_->version();
		report.max_refresh_distance_evaluations = options.max_refresh_distance_evaluations;

		const auto refusal = append_only_refusal(current_ids);
		if (!refusal.empty()) {
			report.rebuild_required = true;
			report.reason = refusal;
			return report;
		}

		const auto old_count = record_count_;
		report.appended = current_ids.size() - ids_.size();
		if (report.appended == 0) {
			report.refreshed = true;
			report.no_op = true;
			report.exact_rows_updated = true;
			report.reason = "kNN graph index is already current";
			return report;
		}
		const auto planned = refresh_distance_evaluation_estimate(old_count, current_ids.size(), report.appended);
		report.old_row_update_distance_evaluations = planned.first;
		report.appended_row_distance_evaluations = planned.second;
		report.distance_evaluations = checked_size_sum(
			planned.first, planned.second, "kNN graph refresh distance-evaluation estimate exceeds size_t capacity");
		if (options.max_refresh_distance_evaluations > 0 &&
			report.distance_evaluations > options.max_refresh_distance_evaluations) {
			report.rebuild_required = true;
			report.exact_rows_updated = false;
			report.reason = "kNN graph append refresh exceeds max_refresh_distance_evaluations: old_size=" +
							std::to_string(old_count) + " appended=" + std::to_string(report.appended) +
							" new_size=" + std::to_string(current_ids.size()) +
							" estimated_distance_evaluations=" + std::to_string(report.distance_evaluations) +
							" max_refresh_distance_evaluations=" +
							std::to_string(options.max_refresh_distance_evaluations) +
							"; rebuild with an explicit budget or use live/landmark search for the appended batch";
			return report;
		}

		ids_.reserve(current_ids.size());
		records_.reserve(current_ids.size());
		adjacency_.reserve(current_ids.size());
		for (std::size_t position = old_count; position < current_ids.size(); ++position) {
			const auto id = current_ids[position];
			ids_.push_back(id);
			records_.push_back(space_->record(id));
			adjacency_.push_back({});
		}
		record_count_ = ids_.size();

		if (k_ > 0) {
			for (std::size_t source = 0; source < old_count; ++source) {
				options.runtime.throw_if_cancelled("kNN graph append refresh old-row update");
				auto candidates = adjacency_[source];
				candidates.reserve(candidates.size() + report.appended);
				for (std::size_t target = old_count; target < record_count_; ++target) {
					options.runtime.throw_if_cancelled("kNN graph append refresh old-row candidate");
					candidates.push_back(neighbor_type{
						ids_[target],
						static_cast<distance_type>(space_->metric()(records_[source], records_[target]))});
				}
				adjacency_[source] = core::take_nearest_neighbors(std::move(candidates), k_);
			}
			for (std::size_t source = old_count; source < record_count_; ++source) {
				options.runtime.throw_if_cancelled("kNN graph append refresh appended-row build");
				adjacency_[source] = build_neighbors(source, nullptr, options.runtime);
			}
		}
		version_ = space_->version();
		maintenance_distance_evaluations_ += report.distance_evaluations;
		report.refreshed = true;
		report.exact_rows_updated = true;
		report.reason = "updated exact kNN rows for appended records without rebuilding old all-pairs rows";
		refresh_cache_key();
		return report;
	}

	auto append_only_refusal(const std::vector<RecordId> &current_ids) const -> std::string
	{
		if (current_ids.size() < ids_.size()) {
			return "source space shrank since the kNN graph snapshot; rebuild required";
		}
		for (std::size_t position = 0; position < ids_.size(); ++position) {
			if (current_ids[position] != ids_[position]) {
				return "source RecordIds no longer match the kNN graph snapshot prefix; rebuild required";
			}
		}
		if (space_->version() < version_) {
			return "source space version moved behind the kNN graph snapshot; rebuild required";
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
		cache_key_ = representation_cache_key("knn_graph_index", metric_key_, version_, ids_,
											  {{"k", std::to_string(k_)}});
	}

	auto validate_position(std::size_t position) const -> void
	{
		if (position >= record_count_) {
			throw std::out_of_range("record position is outside the kNN graph index");
		}
	}

	const space_type *space_;
	std::size_t k_;
	std::size_t record_count_;
	std::size_t version_;
	std::string metric_key_;
	std::string cache_key_;
	std::size_t max_build_distance_evaluations_{};
	std::size_t build_distance_evaluations_{};
	std::size_t maintenance_distance_evaluations_{};
	std::vector<RecordId> ids_;
	std::vector<record_type> records_;
	std::vector<std::vector<neighbor_type>> adjacency_;
};

template <typename Space> auto knn_graph(const Space &space, std::size_t k) -> KnnGraphIndex<Space>
{
	return KnnGraphIndex<Space>(space, k);
}

template <typename Space> auto knn_graph(const Space &space, knn_graph_index_options options) -> KnnGraphIndex<Space>
{
	return KnnGraphIndex<Space>(space, options);
}

template <typename Space> auto graph(const Space &space, std::size_t k) -> KnnGraphIndex<Space>
{
	return knn_graph(space, k);
}

template <typename Space> auto graph(const Space &space, knn_graph_index_options options) -> KnnGraphIndex<Space>
{
	return knn_graph(space, options);
}

} // namespace mtrc::space::storage

#endif
