#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/engine.hpp>

namespace {

constexpr std::size_t grid_width = 5;
constexpr std::size_t grid_height = 5;
constexpr std::size_t cell_count = grid_width * grid_height;
constexpr double epsilon = 1.0e-9;

using Patch = std::array<double, cell_count>;

struct PatchQuery {
	std::string name;
	std::string expected_name;
	Patch patch{};
};

struct FlowEdge {
	int to{};
	int reverse{};
	double capacity{};
	double cost{};
};

auto close_to(double lhs, double rhs, double tolerance = 1.0e-9) -> bool { return std::abs(lhs - rhs) <= tolerance; }

auto cell(std::size_t row, std::size_t column) -> std::size_t { return row * grid_width + column; }

auto row_of(std::size_t index) -> int { return static_cast<int>(index / grid_width); }

auto column_of(std::size_t index) -> int { return static_cast<int>(index % grid_width); }

auto patch(std::initializer_list<std::pair<std::size_t, std::size_t>> active_cells) -> Patch
{
	if (active_cells.size() == 0) {
		throw std::invalid_argument("patch must contain at least one active cell");
	}

	Patch result{};
	const auto mass = 1.0 / static_cast<double>(active_cells.size());
	for (const auto &[row, column] : active_cells) {
		if (row >= grid_height || column >= grid_width) {
			throw std::out_of_range("patch cell is outside the grid");
		}
		result[cell(row, column)] += mass;
	}
	return result;
}

auto total_mass(const Patch &patch_value) -> double
{
	double total = 0.0;
	for (const auto value : patch_value) {
		total += value;
	}
	return total;
}

void add_edge(std::vector<std::vector<FlowEdge>> &graph, int from, int to, double capacity, double cost)
{
	const auto forward_index = static_cast<int>(graph[to].size());
	const auto reverse_index = static_cast<int>(graph[from].size());
	graph[from].push_back(FlowEdge{to, forward_index, capacity, cost});
	graph[to].push_back(FlowEdge{from, reverse_index, 0.0, -cost});
}

auto manhattan_ground_cost(std::size_t source, std::size_t target) -> double
{
	return static_cast<double>(std::abs(row_of(source) - row_of(target)) +
							   std::abs(column_of(source) - column_of(target)));
}

auto min_cost_transport(const Patch &source_patch, const Patch &target_patch) -> double
{
	const auto source_mass = total_mass(source_patch);
	const auto target_mass = total_mass(target_patch);
	if (!close_to(source_mass, target_mass)) {
		throw std::invalid_argument("patch distributions must have equal total mass");
	}

	constexpr int source_offset = 0;
	constexpr int target_offset = static_cast<int>(cell_count);
	constexpr int super_source = static_cast<int>(2 * cell_count);
	constexpr int super_sink = static_cast<int>(2 * cell_count + 1);
	constexpr int node_count = static_cast<int>(2 * cell_count + 2);

	std::vector<std::vector<FlowEdge>> graph(node_count);
	for (std::size_t source = 0; source < cell_count; ++source) {
		if (source_patch[source] > epsilon) {
			add_edge(graph, super_source, source_offset + static_cast<int>(source), source_patch[source], 0.0);
		}
		if (target_patch[source] > epsilon) {
			add_edge(graph, target_offset + static_cast<int>(source), super_sink, target_patch[source], 0.0);
		}
	}

	for (std::size_t source = 0; source < cell_count; ++source) {
		for (std::size_t target = 0; target < cell_count; ++target) {
			add_edge(graph, source_offset + static_cast<int>(source), target_offset + static_cast<int>(target),
					 source_mass, manhattan_ground_cost(source, target));
		}
	}

	double flow = 0.0;
	double cost = 0.0;
	while (flow + epsilon < source_mass) {
		std::vector<double> distance(node_count, std::numeric_limits<double>::infinity());
		std::vector<int> parent_node(node_count, -1);
		std::vector<int> parent_edge(node_count, -1);
		std::vector<bool> queued(node_count, false);
		std::queue<int> queue;

		distance[super_source] = 0.0;
		queue.push(super_source);
		queued[super_source] = true;

		while (!queue.empty()) {
			const auto node = queue.front();
			queue.pop();
			queued[node] = false;

			for (std::size_t edge_index = 0; edge_index < graph[node].size(); ++edge_index) {
				const auto &edge = graph[node][edge_index];
				if (edge.capacity <= epsilon) {
					continue;
				}
				const auto candidate = distance[node] + edge.cost;
				if (candidate + epsilon < distance[edge.to]) {
					distance[edge.to] = candidate;
					parent_node[edge.to] = node;
					parent_edge[edge.to] = static_cast<int>(edge_index);
					if (!queued[edge.to]) {
						queue.push(edge.to);
						queued[edge.to] = true;
					}
				}
			}
		}

		if (parent_node[super_sink] < 0) {
			throw std::runtime_error("transport flow could not satisfy target mass");
		}

		auto increment = source_mass - flow;
		for (auto node = super_sink; node != super_source; node = parent_node[node]) {
			const auto previous = parent_node[node];
			const auto edge_index = parent_edge[node];
			increment = std::min(increment, graph[previous][edge_index].capacity);
		}

		for (auto node = super_sink; node != super_source; node = parent_node[node]) {
			const auto previous = parent_node[node];
			const auto edge_index = parent_edge[node];
			auto &edge = graph[previous][edge_index];
			edge.capacity -= increment;
			graph[node][edge.reverse].capacity += increment;
		}

		flow += increment;
		cost += increment * distance[super_sink];
	}

	return cost;
}

struct GridTransportDistance {
	auto operator()(const Patch &lhs, const Patch &rhs) const -> double { return min_cost_transport(lhs, rhs); }
};

struct PointwiseGridDistance {
	auto operator()(const Patch &lhs, const Patch &rhs) const -> double
	{
		double squared = 0.0;
		for (std::size_t index = 0; index < lhs.size(); ++index) {
			const auto delta = lhs[index] - rhs[index];
			squared += delta * delta;
		}
		return std::sqrt(squared);
	}
};

auto make_patch_names() -> std::vector<std::string>
{
	return {
		"vertical_shift_left",		   "horizontal_at_vertical_query", "horizontal_shift_up",
		"vertical_at_lower_query",	   "vertical_shift_right",		   "horizontal_at_right_query",
		"upper_horizontal_shift_down", "vertical_at_upper_query",
	};
}

auto make_patches() -> std::vector<Patch>
{
	return {
		patch({{0, 1}, {1, 1}, {2, 1}}), patch({{1, 1}, {1, 2}, {1, 3}}), patch({{2, 0}, {2, 1}, {2, 2}}),
		patch({{2, 1}, {3, 1}, {4, 1}}), patch({{2, 4}, {3, 4}, {4, 4}}), patch({{3, 2}, {3, 3}, {3, 4}}),
		patch({{1, 2}, {1, 3}, {1, 4}}), patch({{0, 3}, {1, 3}, {2, 3}}),
	};
}

auto make_queries() -> std::vector<PatchQuery>
{
	return {
		{"center_vertical", "vertical_shift_left", patch({{0, 2}, {1, 2}, {2, 2}})},
		{"lower_horizontal", "horizontal_shift_up", patch({{3, 0}, {3, 1}, {3, 2}})},
		{"lower_right_vertical", "vertical_shift_right", patch({{2, 3}, {3, 3}, {4, 3}})},
		{"upper_right_horizontal", "upper_horizontal_shift_down", patch({{0, 2}, {0, 3}, {0, 4}})},
	};
}

template <typename Space> auto graph_edge_count(Space &space, double threshold) -> std::size_t
{
	mtrc::space::storage::GraphTopology<Space> graph(space);
	for (std::size_t source = 0; source < space.size(); ++source) {
		for (std::size_t target = 0; target < space.size(); ++target) {
			if (source == target) {
				continue;
			}
			const auto distance = space.distance(space.id(source), space.id(target));
			if (distance <= threshold) {
				graph.add_edge(space.id(source), space.id(target), distance);
			}
		}
	}
	return graph.edge_count();
}

} // namespace

int main()
{
	const auto names = make_patch_names();
	const auto records = make_patches();
	const auto queries = make_queries();
	const GridTransportDistance metric;

	assert(names.size() == records.size());

	auto transport_space = mtrc::make_space(records, metric);
	auto vector_baseline = mtrc::make_space(records, PointwiseGridDistance{});

	mtrc::space::storage::DistanceTable<decltype(transport_space)> matrix(transport_space);
	const auto matrix_diagnostics = matrix.diagnostics();
	assert(matrix_diagnostics.cached_distances == records.size() * records.size());

	const auto first_metric_neighbors = mtrc::find_neighbors(transport_space, queries[0].patch, mtrc::count{1});
	const auto first_vector_neighbors = mtrc::find_neighbors(vector_baseline, queries[0].patch, mtrc::count{1});
	assert(first_metric_neighbors.size() == 1);
	assert(first_vector_neighbors.size() == 1);
	assert(names[first_metric_neighbors[0].id.index()] == queries[0].expected_name);
	assert(names[first_vector_neighbors[0].id.index()] != queries[0].expected_name);

	const auto groups = mtrc::find_groups(transport_space, mtrc::stats::structural_analysis::k_medoids_options(2));
	assert(groups.cluster_count == 2);
	assert(groups.record_count == records.size());

	std::size_t metric_correct = 0;
	std::size_t vector_mismatches = 0;
	double margin_sum = 0.0;
	for (const auto &query : queries) {
		const auto metric_neighbors = mtrc::find_neighbors(transport_space, query.patch, mtrc::count{1});
		const auto vector_neighbors = mtrc::find_neighbors(vector_baseline, query.patch, mtrc::count{1});
		assert(metric_neighbors.size() == 1);
		assert(vector_neighbors.size() == 1);

		const auto metric_name = names[metric_neighbors[0].id.index()];
		const auto vector_name = names[vector_neighbors[0].id.index()];
		if (metric_name == query.expected_name) {
			++metric_correct;
		}
		if (vector_name != query.expected_name) {
			++vector_mismatches;
		}

		const auto vector_distance_under_metric = metric(query.patch, records[vector_neighbors[0].id.index()]);
		const auto margin = vector_distance_under_metric - metric_neighbors[0].distance;
		assert(margin > 0.0);
		margin_sum += margin;
	}

	assert(metric_correct == queries.size());
	assert(vector_mismatches == queries.size());
	const auto average_margin = margin_sum / static_cast<double>(queries.size());
	assert(average_margin > 0.0);

	const auto edge_count = graph_edge_count(transport_space, 1.1);
	assert(edge_count > 0);

	std::cout << "distribution workflow metric winner = " << names[first_metric_neighbors[0].id.index()] << " at "
			  << first_metric_neighbors[0].distance << "\n";
	std::cout << "distribution workflow vector comparison winner = " << names[first_vector_neighbors[0].id.index()] << " at "
			  << first_vector_neighbors[0].distance << "\n";
	std::cout << "distribution workflow baseline mismatch = yes\n";
	std::cout << "distribution workflow metric margin = "
			  << metric(queries[0].patch, records[first_vector_neighbors[0].id.index()]) -
					 first_metric_neighbors[0].distance
			  << "\n";
	std::cout << "distribution workflow groups = " << groups.cluster_count << "\n";
	std::cout << "distribution workflow dense evaluations = " << matrix_diagnostics.cached_distances << "\n";
	std::cout << "distribution benchmark records = " << records.size() << "\n";
	std::cout << "distribution benchmark queries = " << queries.size() << "\n";
	std::cout << "distribution benchmark metric correct = " << metric_correct << "/" << queries.size() << "\n";
	std::cout << "distribution benchmark vector mismatches = " << vector_mismatches << "/" << queries.size() << "\n";
	std::cout << "distribution benchmark average metric margin = " << average_margin << "\n";
	std::cout << "distribution benchmark graph edges = " << edge_count << "\n";

	return 0;
}
