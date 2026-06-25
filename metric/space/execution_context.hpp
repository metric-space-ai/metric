// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_EXECUTION_CONTEXT_HPP
#define _METRIC_SPACE_EXECUTION_CONTEXT_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/parameters.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>
#include <metric/space/sample_plan.hpp>
#include <metric/space/storage/blocked_distance_table.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/space/storage/landmark_index.hpp>
#include <metric/space/storage/policy.hpp>
#include <metric/space/storage/triangular_distance_table.hpp>
#include <metric/stats/properties/describe.hpp>
#include <metric/stats/search/neighbors.hpp>
#include <metric/stats/search/nearest.hpp>
#include <metric/stats/structural_analysis/groups.hpp>
#include <metric/stats/structural_analysis/options.hpp>
#include <metric/stats/structural_analysis/outliers.hpp>

namespace mtrc::space {

struct execution_context_diagnostics {
	storage::RuntimeDiagnostics runtime;
	bool shared_provider{false};
	bool materialized_provider{false};
	std::size_t provider_build_count{};
	std::string provider_representation;
	bool provider_stale{false};
	std::size_t cached_distances{};
	std::size_t dense_distance_slots{};
	std::size_t memory_bytes_estimate{};
	bool sample_planning_enabled{false};
	bool sample_plan_cached{false};
	std::size_t sample_plan_build_count{};
	std::string sample_plan_policy;
	std::size_t sample_plan_record_count{};
	std::size_t sample_plan_requested_count{};
	std::size_t sample_plan_candidate_count{};
	std::size_t sample_plan_candidate_universe{};
	bool sample_plan_excludes_position{false};
	std::size_t sample_plan_excluded_position{};
};

template <typename Space> class basic_execution_context {
  public:
	using space_type = Space;
	using distance_type = typename space_type::distance_type;
	using live_provider_type = storage::LiveDistances<space_type>;
	using distance_table_type = storage::DistanceTable<space_type>;
	using symmetric_distance_table_type = storage::SymmetricDistanceTable<space_type>;
	using blocked_distance_table_type = storage::BlockedDistanceTable<space_type>;
	using landmark_index_type = storage::LandmarkIndex<space_type>;

	explicit basic_execution_context(const space_type &space,
									 storage::policy runtime_policy = storage::exact())
		: space_(&space), runtime_policy_(runtime_policy), live_provider_(space)
	{
		storage::require_parallel_metric<typename space_type::metric_type>(runtime_policy_);
		if (runtime_policy_.is_approximate()) {
			require_supported_approximate_context();
			const auto provider_choice = approximate_provider_choice();
			if (provider_choice.use_landmark) {
				landmark_provider_.emplace(
					space, provider_choice.landmark_count, provider_choice.candidate_limit,
					storage::runtime_guard(runtime_policy_));
				provider_build_count_ = 1;
			}
			return;
		}
		storage::require_exact_neighbors(runtime_policy_);
		if (uses_shared_distance_table()) {
			build_shared_provider(space);
			provider_build_count_ = 1;
		}
	}

	auto policy() const -> storage::policy { return runtime_policy_; }
	auto record_count() const -> std::size_t { return space_->size(); }
	auto shared_provider() const -> bool
	{
		return materialized_provider_.has_value() || symmetric_provider_.has_value() ||
			   blocked_provider_.has_value() || landmark_provider_.has_value();
	}
	auto provider_build_count() const -> std::size_t { return provider_build_count_; }
	auto sample_plan_build_count() const -> std::size_t { return sample_plan_build_count_; }
	auto has_cached_sample_plan() const -> bool { return sample_plan_cache_.has_value(); }
	auto cached_sample_plan() const -> std::optional<RegularSamplePlan>
	{
		if (!sample_plan_cache_) {
			return std::nullopt;
		}
		return sample_plan_cache_->plan;
	}
	auto provider_representation() const -> std::string
	{
		if (runtime_policy_.is_approximate()) {
			if (landmark_provider_) {
				return "landmark_index";
			}
			return "sampled_metric_space";
		}
		if (symmetric_provider_) {
			return "symmetric_distance_table";
		}
		if (blocked_provider_) {
			return "blocked_distance_table";
		}
		if (materialized_provider_) {
			return "distance_table";
		}
		return storage::execution_representation(runtime_policy_);
	}

	auto diagnostics(std::string intent = "workflow_context") const -> execution_context_diagnostics
	{
		auto runtime_intent = std::move(intent);
		if (runtime_policy_.is_approximate() && runtime_intent == "workflow_context") {
			runtime_intent = "neighbors";
		}

		execution_context_diagnostics result;
		const auto query_count = landmark_provider_ ? storage::default_automatic_context_query_count
													: std::size_t{0};
		result.runtime = storage::diagnostics_for_space(*space_, runtime_policy_, provider_representation(),
														std::move(runtime_intent), query_count);
		result.runtime.materialized = shared_provider();
		result.runtime.representation = provider_representation();
		result.shared_provider = shared_provider();
		result.materialized_provider = shared_provider();
		result.provider_build_count = provider_build_count_;
		result.provider_representation = provider_representation();
		result.sample_planning_enabled = runtime_policy_.is_approximate();
		result.sample_plan_build_count = sample_plan_build_count_;
		if (runtime_policy_.is_approximate()) {
			result.sample_plan_policy = landmark_provider_ ? "landmark_lower_bound_refinement" : "regular_sample";
		}
		if (sample_plan_cache_) {
			const auto &plan = sample_plan_cache_->plan;
			result.sample_plan_cached = true;
			result.sample_plan_record_count = plan.record_count;
			result.sample_plan_requested_count = plan.requested_count;
			result.sample_plan_candidate_count = plan.size();
			result.sample_plan_candidate_universe = plan.candidate_universe;
			result.sample_plan_excludes_position = plan.excludes_position;
			result.sample_plan_excluded_position = plan.excluded_position;
		}
		if (symmetric_provider_) {
			result.provider_stale = symmetric_provider_->is_stale();
			result.cached_distances = symmetric_provider_->cached_distances();
			result.dense_distance_slots = symmetric_provider_->dense_distance_slots();
			result.memory_bytes_estimate = symmetric_provider_->memory_bytes_estimate();
		} else if (blocked_provider_) {
			result.provider_stale = blocked_provider_->is_stale();
			result.cached_distances = blocked_provider_->cached_distances();
			result.dense_distance_slots = blocked_provider_->dense_distance_slots();
			result.memory_bytes_estimate = blocked_provider_->memory_bytes_estimate();
		} else if (materialized_provider_) {
			result.provider_stale = materialized_provider_->is_stale();
			result.cached_distances = materialized_provider_->cached_distances();
			result.dense_distance_slots = materialized_provider_->dense_distance_slots();
			result.memory_bytes_estimate = materialized_provider_->memory_bytes_estimate();
		} else if (landmark_provider_) {
			const auto diagnostics = landmark_provider_->diagnostics();
			result.provider_stale = diagnostics.stale;
			result.cached_distances = diagnostics.cached_distances;
			result.dense_distance_slots = diagnostics.dense_distance_slots;
			result.memory_bytes_estimate = diagnostics.memory_bytes_estimate;
		}
		return result;
	}

	auto status() const -> execution_context_diagnostics { return diagnostics(); }

	auto sample_plan_for(RecordId query_id, std::size_t requested_count) const -> const RegularSamplePlan &
	{
		if (!runtime_policy_.is_approximate()) {
			throw InvalidRuntimePolicyError(
				"sample plans are prepared only for approximate execution contexts");
		}
		if (landmark_provider_) {
			throw InvalidRuntimePolicyError(
				"sample plans are available only for sampled approximate execution contexts");
		}
		(void)space_->record(query_id);
		const auto query_position = space_->position_of(query_id);
		const auto candidate_count = approximate_candidate_sample_count(requested_count);
		return cached_sample_plan_for(query_position, candidate_count);
	}

	auto sample_plan_for(RecordId query_id, ::mtrc::count requested) const -> const RegularSamplePlan &
	{
		return sample_plan_for(query_id, requested.value);
	}

	auto neighbors(RecordId query_id, std::size_t count) const -> core::NeighborSet<distance_type>
	{
		if (runtime_policy_.is_approximate()) {
			return approximate_neighbors(query_id, count);
		}
		auto runtime = storage::runtime_guard(runtime_policy_);
		auto result = with_provider([query_id, count, runtime](const auto &provider) {
			return stats::search::knn(provider, query_id, count, runtime);
		});
		result.representation = provider_representation();
		core::set_default_search_provenance(result.provenance, result.representation);
		return result;
	}

	auto neighbors(RecordId query_id, ::mtrc::count requested) const -> core::NeighborSet<distance_type>
	{
		return neighbors(query_id, requested.value);
	}

	template <typename Radius> auto range(RecordId query_id, Radius radius) const -> core::NeighborSet<distance_type>
	{
		if (runtime_policy_.is_approximate()) {
			return approximate_range(query_id, radius);
		}
		auto runtime = storage::runtime_guard(runtime_policy_);
		auto result = with_provider([query_id, radius, runtime](const auto &provider) {
			return stats::search::range(provider, query_id, radius, runtime);
		});
		result.representation = provider_representation();
		core::set_default_search_provenance(result.provenance, result.representation);
		return result;
	}

	auto groups(stats::structural_analysis::k_medoids_options options) const
		-> core::ClusteringResult<distance_type>
	{
		storage::require_exact_groups(runtime_policy_);
		auto result = with_provider([&options](const auto &provider) {
			return stats::structural_analysis::find_groups(provider, options);
		});
		result.representation = provider_representation();
		return result;
	}

	auto groups(std::size_t group_count) const -> core::ClusteringResult<distance_type>
	{
		return groups(stats::structural_analysis::k_medoids_options(group_count));
	}

	auto groups(stats::structural_analysis::dbscan_options options) const
		-> core::ClusteringResult<distance_type>
	{
		storage::require_exact_groups(runtime_policy_);
		auto result = with_provider([&options](const auto &provider) {
			return stats::structural_analysis::find_groups(provider, options);
		});
		result.representation = provider_representation();
		return result;
	}

	auto groups(stats::structural_analysis::affinity_propagation_options options) const
		-> core::ClusteringResult<distance_type>
	{
		storage::require_exact_groups(runtime_policy_);
		auto result = with_provider([&options](const auto &provider) {
			return stats::structural_analysis::find_groups(provider, options);
		});
		result.representation = provider_representation();
		return result;
	}

	auto outliers(stats::structural_analysis::dbscan_options options) const
		-> core::OutlierResult<distance_type>
	{
		storage::require_exact_outliers(runtime_policy_);
		auto result = with_provider([&options](const auto &provider) {
			return stats::structural_analysis::find_outliers(provider, options);
		});
		result.representation = provider_representation();
		return result;
	}

	auto outliers(double radius, std::size_t min_points) const -> core::OutlierResult<distance_type>
	{
		return outliers(stats::structural_analysis::dbscan_options(radius, min_points));
	}

	auto nearest_neighbor_outliers(std::size_t k) const -> core::OutlierResult<distance_type>
	{
		storage::require_exact_outliers(runtime_policy_);
		auto result = with_provider([k](const auto &provider) {
			return stats::structural_analysis::nearest_neighbor_outliers(provider, k);
		});
		result.representation = provider_representation();
		return result;
	}

	auto describe(stats::properties::describe_options options = {}) const
		-> core::StructureDescription<distance_type>
	{
		storage::require_exact_describe(runtime_policy_);
		auto result = with_provider([&options](const auto &provider) {
			return stats::properties::describe_structure(provider, options);
		});
		result.representation = provider_representation();
		return result;
	}

  private:
	struct sample_plan_cache_entry {
		RegularSamplePlan plan;
		std::size_t query_position{};
		std::size_t candidate_count{};
	};

	auto require_supported_approximate_context() const -> void
	{
		const auto representation = runtime_policy_.representation_mode();
		if (representation != storage::representation::automatic &&
			representation != storage::representation::implicit &&
			representation != storage::representation::knn_graph &&
			representation != storage::representation::landmark_index) {
			throw InvalidRuntimePolicyError(
				"approximate execution_context currently supports automatic, implicit, kNN graph, or landmark search");
		}
	}

	auto approximate_candidate_sample_count(std::size_t requested_count) const -> std::size_t
	{
		return stats::search::detail::approximate_candidate_count(
			space_->size(), requested_count, runtime_policy_, true);
	}

	auto uses_landmark_context() const -> bool
	{
		return approximate_provider_choice().use_landmark;
	}

	auto approximate_provider_choice() const -> storage::approximate_search_provider_choice
	{
		return storage::choose_approximate_search_provider<typename space_type::metric_type, distance_type>(
			space_->size(), "workflow_context", storage::default_automatic_context_query_count, runtime_policy_);
	}

	auto cached_sample_plan_for(std::size_t query_position, std::size_t candidate_count) const
		-> const RegularSamplePlan &
	{
		if (sample_plan_cache_ && sample_plan_cache_->plan.record_count == space_->size() &&
			sample_plan_cache_->query_position == query_position &&
			sample_plan_cache_->candidate_count == candidate_count) {
			return sample_plan_cache_->plan;
		}

		sample_plan_cache_entry entry;
		entry.query_position = query_position;
		entry.candidate_count = candidate_count;
		entry.plan = regular_sample_positions_excluding(space_->size(), query_position, candidate_count);
		sample_plan_cache_ = std::move(entry);
		++sample_plan_build_count_;
		return sample_plan_cache_->plan;
	}

	template <typename Distance>
	static auto mark_context_sampled_search(core::NeighborSet<Distance> &result) -> void
	{
		stats::search::detail::mark_approximate_sampled_search(result);
	}

	template <typename Distance>
	static auto mark_context_sampled_quality(core::NeighborSet<Distance> &result, const RegularSamplePlan &plan,
											 std::size_t requested_count) -> void
	{
		stats::search::detail::mark_sampled_search_quality(
			result, plan.size(), plan.candidate_universe, requested_count);
	}

	auto approximate_neighbors(RecordId query_id, std::size_t count) const -> core::NeighborSet<distance_type>
	{
		(void)space_->record(query_id);
		auto runtime = storage::runtime_guard(runtime_policy_);
		if (landmark_provider_) {
			const auto candidate_limit = stats::search::detail::landmark_candidate_count(
				space_->size(), runtime_policy_, true);
			auto result = core::make_neighbor_set(
				landmark_provider_->knn(query_id, count, candidate_limit, runtime), space_->size(),
				count, "knn", "landmark_index", false);
			stats::search::detail::mark_approximate_landmark_search(
				result, candidate_limit, space_->size() > 0 ? space_->size() - 1 : 0, count,
				landmark_provider_->query_landmark_distance_count() + candidate_limit);
			return result;
		}
		if (count == 0) {
			auto result = core::make_neighbor_set(std::vector<core::Neighbor<distance_type>>{}, space_->size(),
												  count, "knn", "sampled_metric_space", false);
			mark_context_sampled_search(result);
			stats::search::detail::mark_sampled_search_quality(
				result, 0, space_->size() > 0 ? space_->size() - 1 : 0, count);
			return result;
		}

		const auto &plan = sample_plan_for(query_id, count);
		std::vector<core::Neighbor<distance_type>> candidates;
		candidates.reserve(plan.size());
		for (const auto position : plan.positions) {
			runtime.throw_if_cancelled("approximate sampled context neighbor search");
			const auto id = space_->id(position);
			candidates.push_back(
				core::Neighbor<distance_type>{id, static_cast<distance_type>(space_->distance(query_id, id))});
		}

		auto result = core::nearest_neighbor_set(std::move(candidates), count, space_->size(),
												 "sampled_metric_space");
		mark_context_sampled_search(result);
		mark_context_sampled_quality(result, plan, count);
		if (stats::search::detail::can_measure_sampled_recall(plan.size(), plan.candidate_universe,
															   runtime_policy_)) {
			auto reference = stats::search::detail::guarded_neighbor_candidates_if<distance_type>(
				space_->size(), [this](std::size_t position) { return space_->id(position); },
				[this, query_id](RecordId id, std::size_t) { return space_->distance(query_id, id); },
				[query_id](RecordId id, std::size_t) { return id != query_id; },
				runtime, "approximate sampled context neighbor recall");
			reference = core::take_nearest_neighbors(std::move(reference), count);
			stats::search::detail::mark_sampled_search_recall(
				result, reference, plan.candidate_universe);
		} else {
			stats::search::detail::mark_sampled_search_recall_unmeasured(
				result, plan.size(), plan.candidate_universe, runtime_policy_);
		}
		result.record_count = space_->size();
		result.requested_count = count;
		return result;
	}

	template <typename Radius>
	auto approximate_range(RecordId query_id, Radius radius) const -> core::NeighborSet<distance_type>
	{
		stats::search::engine_detail::validate_radius(radius);
		(void)space_->record(query_id);
		auto runtime = storage::runtime_guard(runtime_policy_);
		if (landmark_provider_) {
			const auto candidate_limit = stats::search::detail::landmark_candidate_count(
				space_->size(), runtime_policy_, true);
			auto result = core::range_neighbor_set(
				landmark_provider_->range(query_id, radius, candidate_limit, runtime), space_->size(),
				"landmark_index");
			stats::search::detail::mark_approximate_landmark_search(
				result, candidate_limit, space_->size() > 0 ? space_->size() - 1 : 0, 0,
				landmark_provider_->query_landmark_distance_count() + candidate_limit);
			return result;
		}
		const auto &plan = sample_plan_for(query_id, std::size_t{0});

		using comparison_type = typename std::common_type<distance_type, Radius>::type;
		const auto threshold = static_cast<comparison_type>(radius);
		std::vector<core::Neighbor<distance_type>> candidates;
		candidates.reserve(plan.size());
		for (const auto position : plan.positions) {
			runtime.throw_if_cancelled("approximate sampled context range search");
			const auto id = space_->id(position);
			const auto distance = static_cast<distance_type>(space_->distance(query_id, id));
			if (static_cast<comparison_type>(distance) <= threshold) {
				candidates.push_back(core::Neighbor<distance_type>{id, distance});
			}
		}

		auto result = core::range_neighbor_set(std::move(candidates), space_->size(), "sampled_metric_space");
		mark_context_sampled_search(result);
		mark_context_sampled_quality(result, plan, 0);
		if (stats::search::detail::can_measure_sampled_recall(plan.size(), plan.candidate_universe,
															   runtime_policy_)) {
			auto reference = stats::search::detail::guarded_neighbor_candidates_within_if<distance_type>(
				space_->size(), [this](std::size_t position) { return space_->id(position); },
				[this, query_id](RecordId id, std::size_t) { return space_->distance(query_id, id); },
				radius, [query_id](RecordId id, std::size_t) { return id != query_id; },
				runtime, "approximate sampled context range recall");
			core::sort_neighbors(reference);
			stats::search::detail::mark_sampled_search_recall(
				result, reference, plan.candidate_universe);
		} else {
			stats::search::detail::mark_sampled_search_recall_unmeasured(
				result, plan.size(), plan.candidate_universe, runtime_policy_);
		}
		return result;
	}

	auto uses_shared_distance_table() const -> bool
	{
		if (!runtime_policy_.uses_materialization()) {
			return false;
		}
		const auto representation = runtime_policy_.representation_mode();
		if (representation == storage::representation::landmark_index) {
			throw InvalidRuntimePolicyError("landmark index execution_context requires approximate accuracy");
		}
		if (representation == storage::representation::cover_tree ||
			representation == storage::representation::knn_graph) {
			throw InvalidRuntimePolicyError(
				"execution_context currently shares live distances or a distance_table provider");
		}
		return true;
	}

	static constexpr auto supports_symmetric_distance_table() -> bool
	{
		constexpr auto law = storage::SymmetricDistanceTable<space_type>::metric_type_law;
		return law == core::metric_law::metric || law == core::metric_law::pseudo_metric;
	}

	auto build_blocked_provider(const space_type &space) -> void
	{
		blocked_provider_.emplace(space, storage::blocked_distance_table_options_for<distance_type>(runtime_policy_));
	}

	auto handle_dense_budget_exceeded(const std::string &reason, const space_type &space) -> void
	{
		if (runtime_policy_.allows_chunking_fallback()) {
			build_blocked_provider(space);
			return;
		}
		throw RepresentationError(reason);
	}

	auto build_shared_provider(const space_type &space) -> void
	{
		auto runtime = storage::runtime_guard(runtime_policy_);
		if constexpr (supports_symmetric_distance_table()) {
			const auto records = space.size();
			if (runtime_policy_.max_dense_records() > 0 && records > runtime_policy_.max_dense_records()) {
				handle_dense_budget_exceeded(
					"symmetric distance table exceeds max_dense_records: records=" + std::to_string(records) +
					" max_dense_records=" + std::to_string(runtime_policy_.max_dense_records()),
					space);
				return;
			}
			const auto estimated_bytes =
				storage::estimate_triangular_distance_table_memory_bytes<distance_type>(records);
			if (runtime_policy_.max_memory_bytes() > 0 && estimated_bytes > runtime_policy_.max_memory_bytes()) {
				handle_dense_budget_exceeded(
					"symmetric distance table exceeds max_memory_bytes: records=" + std::to_string(records) +
					" estimated_bytes=" + std::to_string(estimated_bytes) +
					" max_memory_bytes=" + std::to_string(runtime_policy_.max_memory_bytes()),
					space);
				return;
			}
			symmetric_provider_.emplace(space, runtime);
		} else {
			const auto records = space.size();
			if (runtime_policy_.max_dense_records() > 0 && records > runtime_policy_.max_dense_records()) {
				handle_dense_budget_exceeded(
					"distance table dense storage exceeds max_dense_records: records=" +
					std::to_string(records) +
					" max_dense_records=" + std::to_string(runtime_policy_.max_dense_records()),
					space);
				return;
			}
			const auto estimated_bytes =
				storage::estimate_distance_table_memory_bytes<distance_type>(records);
			if (runtime_policy_.max_memory_bytes() > 0 && estimated_bytes > runtime_policy_.max_memory_bytes()) {
				handle_dense_budget_exceeded(
					"distance table dense storage exceeds max_memory_bytes: records=" +
					std::to_string(records) + " estimated_bytes=" + std::to_string(estimated_bytes) +
					" max_memory_bytes=" + std::to_string(runtime_policy_.max_memory_bytes()),
					space);
				return;
			}
			materialized_provider_.emplace(space, storage::distance_table_options_for(
				runtime_policy_, storage::distance_table_mode::eager, runtime));
		}
	}

	template <typename Fn> decltype(auto) with_provider(Fn &&fn) const
	{
		if (symmetric_provider_) {
			return std::forward<Fn>(fn)(*symmetric_provider_);
		}
		if (materialized_provider_) {
			return std::forward<Fn>(fn)(*materialized_provider_);
		}
		if (blocked_provider_) {
			return std::forward<Fn>(fn)(*blocked_provider_);
		}
		return std::forward<Fn>(fn)(live_provider_);
	}

	const space_type *space_;
	storage::policy runtime_policy_;
	live_provider_type live_provider_;
	std::optional<distance_table_type> materialized_provider_;
	std::optional<symmetric_distance_table_type> symmetric_provider_;
	std::optional<blocked_distance_table_type> blocked_provider_;
	std::optional<landmark_index_type> landmark_provider_;
	std::size_t provider_build_count_{};
	mutable std::optional<sample_plan_cache_entry> sample_plan_cache_;
	mutable std::size_t sample_plan_build_count_{};
};

template <typename Space>
auto execution_context(const Space &space, storage::policy runtime_policy = storage::exact())
	-> basic_execution_context<Space>
{
	return basic_execution_context<Space>(space, runtime_policy);
}

} // namespace mtrc::space

#endif
