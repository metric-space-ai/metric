// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RUNTIME_POLICY_HPP
#define _METRIC_RUNTIME_POLICY_HPP

#include <cstddef>
#include <string>

namespace mtrc::space::storage {

enum class accuracy {
	exact,
	approximate,
};

enum class materialization_policy {
	lazy,
	materialized,
};

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
};

class policy {
  public:
	constexpr policy(accuracy accuracy_mode = accuracy::exact,
					 materialization_policy materialization_mode = materialization_policy::lazy,
					 execution execution_mode = execution::serial,
					 representation representation_mode = representation::automatic, std::size_t graph_neighbors = 0)
		: accuracy_(accuracy_mode), materialization_(materialization_mode), execution_(execution_mode),
		  representation_(representation_mode), graph_neighbors_(graph_neighbors)
	{
	}

	constexpr auto accuracy_mode() const noexcept -> accuracy { return accuracy_; }
	constexpr auto materialization_mode() const noexcept -> materialization_policy { return materialization_; }
	constexpr auto execution_mode() const noexcept -> execution { return execution_; }
	constexpr auto representation_mode() const noexcept -> representation { return representation_; }
	constexpr auto graph_neighbors() const noexcept -> std::size_t { return graph_neighbors_; }

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
			   representation_ == representation::knn_graph;
	}

	accuracy accuracy_;
	materialization_policy materialization_;
	execution execution_;
	representation representation_;
	std::size_t graph_neighbors_;
};

constexpr auto exact() -> policy { return policy(accuracy::exact, materialization_policy::lazy, execution::serial); }

constexpr auto approximate() -> policy
{
	return policy(accuracy::approximate, materialization_policy::lazy, execution::serial);
}

constexpr auto lazy(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::lazy, base.execution_mode(), base.representation_mode(),
				  base.graph_neighbors());
}

constexpr auto materialized(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::materialized, base.execution_mode(),
				  base.representation_mode(), base.graph_neighbors());
}

constexpr auto serial(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), execution::serial, base.representation_mode(),
				  base.graph_neighbors());
}

constexpr auto parallel(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), execution::parallel, base.representation_mode(),
				  base.graph_neighbors());
}

constexpr auto using_implicit(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::lazy, base.execution_mode(), representation::implicit,
				  base.graph_neighbors());
}

constexpr auto using_distance_table(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::materialized, base.execution_mode(),
				  representation::distance_table, base.graph_neighbors());
}

constexpr auto using_cover_tree(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::materialized, base.execution_mode(),
				  representation::cover_tree, base.graph_neighbors());
}

constexpr auto using_knn_graph(std::size_t graph_neighbors = 0, policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization_policy::materialized, base.execution_mode(), representation::knn_graph,
				  graph_neighbors);
}

} // namespace mtrc::space::storage

#endif
