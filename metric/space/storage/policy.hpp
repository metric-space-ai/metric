// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RUNTIME_POLICY_HPP
#define _METRIC_RUNTIME_POLICY_HPP

#include <chrono>
#include <cstddef>
#include <string>

#include <metric/core/errors.hpp>

namespace mtrc::space::storage {

enum class accuracy {
	exact,
	approximate,
};

enum class materialization_policy {
	lazy,
	materialized,
};

// NOTE: execution::parallel is currently a declared preference only. The space /
// search / stats paths run serially (no threads are forked); selecting parallel
// gates a metric thread-safety check and diagnostics string but does NOT yet
// accelerate work. It is exposed honestly here so callers do not assume threading
// they are not getting; see docs/supported-surface.md (Scale & performance).
enum class execution {
	serial,
	parallel,
};

enum class representation {
	automatic,
	implicit,
	distance_table,
	cover_tree,
	knn_graph,
	landmark_index,
};

inline constexpr std::size_t default_landmark_index_landmarks = 16;
inline constexpr std::size_t default_landmark_index_candidates = 4096;

struct resource_budget {
	std::size_t max_memory_bytes{};
	std::size_t max_distance_evaluations{};
	std::size_t max_dense_records{};
	bool allow_approximate{false};
	bool allow_chunking{false};
	bool allow_out_of_core_spill{false};
	std::size_t max_runtime_ms{};
	std::size_t max_spill_bytes{};
};

class policy {
  public:
	constexpr policy(accuracy accuracy_mode = accuracy::exact,
					 materialization_policy materialization_mode = materialization_policy::lazy,
					 execution execution_mode = execution::serial,
					 representation representation_mode = representation::automatic, std::size_t graph_neighbors = 0,
					 std::size_t max_dense_records = 0, std::size_t max_memory_bytes = 0,
					 std::size_t max_distance_evaluations = 0, bool allow_approximate = false,
					 bool allow_chunking = false, bool allow_out_of_core_spill = false,
					 std::size_t max_runtime_ms = 0, std::size_t max_spill_bytes = 0)
		: accuracy_(accuracy_mode), materialization_(materialization_mode), execution_(execution_mode),
		  representation_(representation_mode), graph_neighbors_(graph_neighbors),
		  max_dense_records_(max_dense_records), max_memory_bytes_(max_memory_bytes),
		  max_distance_evaluations_(max_distance_evaluations), allow_approximate_(allow_approximate),
		  allow_chunking_(allow_chunking), allow_out_of_core_spill_(allow_out_of_core_spill),
		  max_runtime_ms_(max_runtime_ms), max_spill_bytes_(max_spill_bytes)
	{
	}

	constexpr auto accuracy_mode() const noexcept -> accuracy { return accuracy_; }
	constexpr auto materialization_mode() const noexcept -> materialization_policy { return materialization_; }
	constexpr auto execution_mode() const noexcept -> execution { return execution_; }
	constexpr auto representation_mode() const noexcept -> representation { return representation_; }
	constexpr auto graph_neighbors() const noexcept -> std::size_t { return graph_neighbors_; }
	constexpr auto max_dense_records() const noexcept -> std::size_t { return max_dense_records_; }
	constexpr auto max_memory_bytes() const noexcept -> std::size_t { return max_memory_bytes_; }
	constexpr auto max_distance_evaluations() const noexcept -> std::size_t { return max_distance_evaluations_; }
	constexpr auto allows_approximate_fallback() const noexcept -> bool { return allow_approximate_; }
	constexpr auto allows_chunking_fallback() const noexcept -> bool { return allow_chunking_; }
	constexpr auto allows_out_of_core_spill() const noexcept -> bool { return allow_out_of_core_spill_; }
	constexpr auto max_runtime_ms() const noexcept -> std::size_t { return max_runtime_ms_; }
	constexpr auto max_spill_bytes() const noexcept -> std::size_t { return max_spill_bytes_; }

	constexpr auto is_exact() const noexcept -> bool { return accuracy_ == accuracy::exact; }
	constexpr auto is_approximate() const noexcept -> bool { return accuracy_ == accuracy::approximate; }
	constexpr auto uses_materialization() const noexcept -> bool
	{
		return materialization_ == materialization_policy::materialized || representation_requires_materialization();
	}
	constexpr auto uses_parallel_execution() const noexcept -> bool { return execution_ == execution::parallel; }
	constexpr auto has_representation_preference() const noexcept -> bool
	{
		return representation_ != representation::automatic;
	}

	auto representation_preference() const -> std::string
	{
		switch (representation_) {
		case representation::implicit:
			return "implicit";
		case representation::distance_table:
			return "distance_table";
		case representation::cover_tree:
			return "cover_tree_index";
		case representation::knn_graph:
			return "knn_graph_index";
		case representation::landmark_index:
			return "landmark_index";
		case representation::automatic:
			return materialization_ == materialization_policy::materialized ? "distance_table" : "implicit";
		}
		return "implicit";
	}

	auto name() const -> std::string
	{
		std::string value = is_exact() ? "exact" : "approximate";
		value += uses_materialization() ? "_materialized" : "_lazy";
		value += uses_parallel_execution() ? "_parallel" : "_serial";
		return value;
	}

  private:
	constexpr auto representation_requires_materialization() const noexcept -> bool
	{
		return representation_ == representation::distance_table || representation_ == representation::cover_tree ||
			   representation_ == representation::knn_graph || representation_ == representation::landmark_index;
	}

	accuracy accuracy_;
	materialization_policy materialization_;
	execution execution_;
	representation representation_;
	std::size_t graph_neighbors_;
	std::size_t max_dense_records_;
	std::size_t max_memory_bytes_;
	std::size_t max_distance_evaluations_;
	bool allow_approximate_;
	bool allow_chunking_;
	bool allow_out_of_core_spill_;
	std::size_t max_runtime_ms_;
	std::size_t max_spill_bytes_;
};

constexpr auto exact() -> policy { return policy(accuracy::exact, materialization_policy::lazy, execution::serial); }

constexpr auto approximate() -> policy
{
	return policy(accuracy::approximate, materialization_policy::lazy, execution::serial);
}

constexpr auto lazy(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::lazy, base.execution_mode(), base.representation_mode(),
				  base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(),
				  base.max_distance_evaluations(), base.allows_approximate_fallback(),
				  base.allows_chunking_fallback(), base.allows_out_of_core_spill(), base.max_runtime_ms(),
				  base.max_spill_bytes());
}

constexpr auto materialized(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::materialized, base.execution_mode(),
				  base.representation_mode(), base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(),
				  base.max_distance_evaluations(), base.allows_approximate_fallback(),
				  base.allows_chunking_fallback(), base.allows_out_of_core_spill(), base.max_runtime_ms(),
				  base.max_spill_bytes());
}

constexpr auto serial(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), execution::serial, base.representation_mode(),
				  base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(),
				  base.max_distance_evaluations(), base.allows_approximate_fallback(),
				  base.allows_chunking_fallback(), base.allows_out_of_core_spill(), base.max_runtime_ms(),
				  base.max_spill_bytes());
}

constexpr auto parallel(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), execution::parallel, base.representation_mode(),
				  base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(),
				  base.max_distance_evaluations(), base.allows_approximate_fallback(),
				  base.allows_chunking_fallback(), base.allows_out_of_core_spill(), base.max_runtime_ms(),
				  base.max_spill_bytes());
}

constexpr auto using_implicit(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::lazy, base.execution_mode(), representation::implicit,
				  base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(),
				  base.max_distance_evaluations(), base.allows_approximate_fallback(),
				  base.allows_chunking_fallback(), base.allows_out_of_core_spill(), base.max_runtime_ms(),
				  base.max_spill_bytes());
}

constexpr auto using_distance_table(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::materialized, base.execution_mode(),
				  representation::distance_table, base.graph_neighbors(), base.max_dense_records(),
				  base.max_memory_bytes(), base.max_distance_evaluations(), base.allows_approximate_fallback(),
				  base.allows_chunking_fallback(), base.allows_out_of_core_spill(), base.max_runtime_ms(),
				  base.max_spill_bytes());
}

constexpr auto using_cover_tree(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::materialized, base.execution_mode(),
				  representation::cover_tree, base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(),
				  base.max_distance_evaluations(), base.allows_approximate_fallback(),
				  base.allows_chunking_fallback(), base.allows_out_of_core_spill(), base.max_runtime_ms(),
				  base.max_spill_bytes());
}

constexpr auto using_knn_graph(std::size_t graph_neighbors = 0, policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::materialized, base.execution_mode(), representation::knn_graph,
				  graph_neighbors, base.max_dense_records(), base.max_memory_bytes(), base.max_distance_evaluations(),
				  base.allows_approximate_fallback(), base.allows_chunking_fallback(),
				  base.allows_out_of_core_spill(), base.max_runtime_ms(), base.max_spill_bytes());
}

constexpr auto using_landmark_index(std::size_t candidate_count = 0, policy base = approximate()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::materialized, base.execution_mode(),
				  representation::landmark_index, candidate_count, base.max_dense_records(),
				  base.max_memory_bytes(), base.max_distance_evaluations(), base.allows_approximate_fallback(),
				  base.allows_chunking_fallback(), base.allows_out_of_core_spill(), base.max_runtime_ms(),
				  base.max_spill_bytes());
}

constexpr auto with_distance_table_budget(policy base, std::size_t max_dense_records = 0,
										  std::size_t max_memory_bytes = 0) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), base.execution_mode(), base.representation_mode(),
				  base.graph_neighbors(), max_dense_records, max_memory_bytes, base.max_distance_evaluations(),
				  base.allows_approximate_fallback(), base.allows_chunking_fallback(),
				  base.allows_out_of_core_spill(), base.max_runtime_ms(), base.max_spill_bytes());
}

constexpr auto with_distance_evaluation_budget(policy base, std::size_t max_distance_evaluations = 0) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), base.execution_mode(), base.representation_mode(),
				  base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(), max_distance_evaluations,
				  base.allows_approximate_fallback(), base.allows_chunking_fallback(),
				  base.allows_out_of_core_spill(), base.max_runtime_ms(), base.max_spill_bytes());
}

constexpr auto with_resource_budget(policy base, resource_budget budget) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), base.execution_mode(), base.representation_mode(),
				  base.graph_neighbors(), budget.max_dense_records, budget.max_memory_bytes,
				  budget.max_distance_evaluations, budget.allow_approximate, budget.allow_chunking,
				  budget.allow_out_of_core_spill, budget.max_runtime_ms, budget.max_spill_bytes);
}

constexpr auto with_runtime_budget(policy base, std::size_t max_runtime_ms = 0) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), base.execution_mode(), base.representation_mode(),
				  base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(),
				  base.max_distance_evaluations(), base.allows_approximate_fallback(),
				  base.allows_chunking_fallback(), base.allows_out_of_core_spill(), max_runtime_ms,
				  base.max_spill_bytes());
}

constexpr auto with_spill_byte_budget(policy base, std::size_t max_spill_bytes = 0) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), base.execution_mode(), base.representation_mode(),
				  base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(),
				  base.max_distance_evaluations(), base.allows_approximate_fallback(),
				  base.allows_chunking_fallback(), base.allows_out_of_core_spill(), base.max_runtime_ms(),
				  max_spill_bytes);
}

constexpr auto allow_approximate_fallback(policy base, bool enabled = true) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), base.execution_mode(), base.representation_mode(),
				  base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(),
				  base.max_distance_evaluations(), enabled, base.allows_chunking_fallback(),
				  base.allows_out_of_core_spill(), base.max_runtime_ms(), base.max_spill_bytes());
}

constexpr auto allow_chunking_fallback(policy base, bool enabled = true) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), base.execution_mode(), base.representation_mode(),
				  base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(),
				  base.max_distance_evaluations(), base.allows_approximate_fallback(), enabled,
				  base.allows_out_of_core_spill(), base.max_runtime_ms(), base.max_spill_bytes());
}

constexpr auto allow_out_of_core_spill(policy base, bool enabled = true) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), base.execution_mode(), base.representation_mode(),
				  base.graph_neighbors(), base.max_dense_records(), base.max_memory_bytes(),
				  base.max_distance_evaluations(), base.allows_approximate_fallback(),
				  base.allows_chunking_fallback(), enabled, base.max_runtime_ms(), base.max_spill_bytes());
}

constexpr auto budget_from_policy(policy runtime_policy) -> resource_budget
{
	return resource_budget{runtime_policy.max_memory_bytes(), runtime_policy.max_distance_evaluations(),
						   runtime_policy.max_dense_records(), runtime_policy.allows_approximate_fallback(),
						   runtime_policy.allows_chunking_fallback(), runtime_policy.allows_out_of_core_spill(),
						   runtime_policy.max_runtime_ms(), runtime_policy.max_spill_bytes()};
}

class runtime_guard {
  public:
	using clock = std::chrono::steady_clock;

	runtime_guard() = default;

	explicit runtime_guard(std::size_t max_runtime_ms)
		: max_runtime_ms_(max_runtime_ms),
		  deadline_(max_runtime_ms == 0 ? clock::time_point{} :
									   clock::now() + std::chrono::milliseconds(max_runtime_ms))
	{
	}

	explicit runtime_guard(policy runtime_policy)
		: runtime_guard(runtime_policy.max_runtime_ms())
	{
	}

	explicit runtime_guard(resource_budget budget)
		: runtime_guard(budget.max_runtime_ms)
	{
	}

	auto active() const noexcept -> bool { return max_runtime_ms_ != 0; }
	auto max_runtime_ms() const noexcept -> std::size_t { return max_runtime_ms_; }

	auto throw_if_cancelled(const char *operation = "execution") const -> void
	{
		if (!active()) {
			return;
		}
		if (clock::now() >= deadline_) {
			throw RepresentationError(
				std::string("execution cancelled after exceeding max_runtime_ms: operation=") +
				(operation == nullptr ? "execution" : operation) +
				" max_runtime_ms=" + std::to_string(max_runtime_ms_));
		}
	}

  private:
	std::size_t max_runtime_ms_{};
	clock::time_point deadline_{};
};

} // namespace mtrc::space::storage

#endif
