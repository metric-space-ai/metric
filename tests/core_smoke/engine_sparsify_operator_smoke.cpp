#include <cassert>
#include <tuple>
#include <vector>

#include "metric/operators/sparsify.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

int main()
{
	const std::vector<int> records = {0, 1, 2, 4};

	const auto graph = metric::operators::exact_knn_graph(records, AbsoluteDistance{}, 2);
	assert(graph.metadata.strategy == "exact_knn");
	assert(graph.metadata.record_count == records.size());
	assert(graph.metadata.edge_count == 8);
	assert(graph.metadata.directed);

	const auto pruned = metric::operators::prune_graph_out_degree(graph, 1);
	assert(pruned.metadata.sparsification == "out_degree");
	assert(pruned.metadata.max_out_degree.has_value());
	assert(pruned.metadata.max_out_degree.value() == 1);
	assert(pruned.metadata.edge_count == records.size());
	assert((pruned.edges == std::vector<std::tuple<std::size_t, std::size_t, int>>{
							 {0, 1, 1},
							 {1, 0, 1},
							 {2, 1, 1},
							 {3, 2, 2},
						 }));

	const auto undirected = metric::operators::symmetrize_graph(pruned, "union", "minimum_distance");
	assert(!undirected.metadata.directed);
	assert(undirected.metadata.symmetrization == "union");
	assert(undirected.metadata.edge_count == 3);

	const auto degrees = metric::operators::graph_degree_diagnostics(undirected);
	assert(degrees.degree_policy == "undirected_endpoint");
	assert((degrees.degrees == std::vector<std::size_t>{1, 2, 2, 1}));

	const auto connectivity = metric::operators::graph_connectivity_diagnostics(undirected);
	assert(connectivity.connected);
	assert(connectivity.component_count == 1);

	const auto stretch = metric::operators::graph_stretch_diagnostics(records, AbsoluteDistance{}, undirected);
	assert(stretch.record_count == records.size());
	assert(stretch.edge_count == undirected.edges.size());
	assert(stretch.reachable_pair_count == 6);
	assert(stretch.unreachable_pair_count == 0);

	return 0;
}
