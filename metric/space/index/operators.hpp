// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_HPP
#define _METRIC_OPERATORS_HPP

#include <metric/space.hpp>

#include <metric/core/concepts.hpp>
#include <metric/numeric/math/GraphPrimitives.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace mtrc::space::index {

inline constexpr std::size_t default_max_exact_graph_distance_evaluations = 1'000'000;
inline constexpr std::size_t default_max_graph_shortest_path_cells = 1'000'000;
inline constexpr std::size_t default_max_graph_shortest_path_relaxations = 100'000'000;

struct exact_graph_options {
	// Maximum metric calls for exhaustive directed graph construction.
	// Set to 0 only when the caller intentionally opts into unbounded exact work.
	std::size_t max_distance_evaluations{default_max_exact_graph_distance_evaluations};
};

struct graph_stretch_options {
	// Maximum metric calls used to compare graph paths with direct metric distances.
	// Set any field to 0 only when intentionally opting out of that guard.
	std::size_t max_metric_distance_evaluations{default_max_exact_graph_distance_evaluations};
	std::size_t max_shortest_path_cells{default_max_graph_shortest_path_cells};
	std::size_t max_shortest_path_relaxations{default_max_graph_shortest_path_relaxations};
};

namespace detail {

template <typename Container> using record_type_t = typename std::decay<typename Container::value_type>::type;

template <typename Container, typename Metric>
using finite_space_t = ::mtrc::FiniteSpace<record_type_t<Container>, Metric>;

inline auto checked_graph_size_product(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
{
	if (rhs != 0 && lhs > std::numeric_limits<std::size_t>::max() / rhs) {
		throw ::mtrc::RepresentationError(message);
	}
	return lhs * rhs;
}

inline auto graph_metric_pair_count(std::size_t record_count, bool directed) -> std::size_t
{
	if (record_count < 2) {
		return 0;
	}

	const auto directed_count = checked_graph_size_product(
		record_count, record_count - 1, "graph pair count exceeds size_t capacity");
	return directed ? directed_count : directed_count / 2;
}

inline auto require_graph_distance_budget(std::size_t record_count, bool directed,
										  std::size_t max_distance_evaluations, const char *operation,
										  const char *options_hint) -> std::size_t
{
	const auto pair_count = graph_metric_pair_count(record_count, directed);
	if (max_distance_evaluations == 0 || pair_count <= max_distance_evaluations) {
		return pair_count;
	}

	throw ::mtrc::RepresentationError(
		std::string(operation) + " refused exhaustive graph work before metric calls: records=" +
		std::to_string(record_count) +
		(directed ? ", directed_metric_pairs=" : ", metric_pairs=") + std::to_string(pair_count) +
		", max_distance_evaluations=" + std::to_string(max_distance_evaluations) + ". " + options_hint);
}

inline auto graph_shortest_path_cell_count(std::size_t record_count) -> std::size_t
{
	return checked_graph_size_product(record_count, record_count,
									  "graph shortest-path matrix exceeds size_t capacity");
}

inline auto graph_shortest_path_relaxation_count(std::size_t record_count) -> std::size_t
{
	return checked_graph_size_product(graph_shortest_path_cell_count(record_count), record_count,
									  "graph shortest-path relaxation estimate exceeds size_t capacity");
}

inline auto require_graph_shortest_path_budget(std::size_t record_count, graph_stretch_options options,
											   const char *operation) -> void
{
	const auto cell_count = graph_shortest_path_cell_count(record_count);
	if (options.max_shortest_path_cells != 0 && cell_count > options.max_shortest_path_cells) {
		throw ::mtrc::RepresentationError(
			std::string(operation) + " refused dense shortest-path matrix before allocation: records=" +
			std::to_string(record_count) + ", cells=" + std::to_string(cell_count) +
			", max_shortest_path_cells=" + std::to_string(options.max_shortest_path_cells) +
			". Pass graph_stretch_options{0, 0, 0} only when unbounded stretch diagnostics are intentional.");
	}

	if (options.max_shortest_path_relaxations == 0) {
		return;
	}

	const auto relaxation_count = graph_shortest_path_relaxation_count(record_count);
	if (relaxation_count > options.max_shortest_path_relaxations) {
		throw ::mtrc::RepresentationError(
			std::string(operation) +
			" refused all-pairs shortest-path closure before allocation: records=" + std::to_string(record_count) +
			", estimated_relaxations=" + std::to_string(relaxation_count) +
			", max_shortest_path_relaxations=" +
			std::to_string(options.max_shortest_path_relaxations) +
			". Pass graph_stretch_options{0, 0, 0} only when unbounded stretch diagnostics are intentional.");
	}
}

template <typename Space> class IndexPairwiseDistances {
  public:
	using distance_type = typename Space::distance_type;

	explicit IndexPairwiseDistances(const Space &space) : space_(&space) {}

	auto distance(::mtrc::RecordId lhs, ::mtrc::RecordId rhs) const -> distance_type
	{
		return space_->distance(position_of(lhs), position_of(rhs));
	}

	auto record_count() const -> std::size_t { return space_->size(); }
	auto id(std::size_t position) const -> ::mtrc::RecordId { return ::mtrc::RecordId::from_index(position); }
	auto position_of(::mtrc::RecordId id) const -> std::size_t
	{
		if (!contains(id)) {
			throw std::out_of_range("record id is outside the indexed distance provider");
		}
		return id.index();
	}
	auto contains(::mtrc::RecordId id) const -> bool { return id.index() < record_count(); }
	auto version() const -> std::size_t { return 0; }
	auto is_stale() const -> bool { return false; }

  private:
	const Space *space_;
};

template <typename Space> auto indexed_pairwise_distances(const Space &space) -> IndexPairwiseDistances<Space>
{
	return IndexPairwiseDistances<Space>(space);
}

} // namespace detail

template <typename Distance, typename RadiusValue = Distance> struct GraphConstructionMetadata {
	std::string strategy;
	std::size_t record_count{};
	std::size_t edge_count{};
	bool directed{true};
	bool self_loops{false};
	bool exact{true};
	std::optional<std::size_t> k;
	std::optional<RadiusValue> radius;
	std::string edge_payload;
	std::string weighting;
	std::string symmetrization;
	std::string normalization;
	std::string tie_break;
	std::optional<std::size_t> max_out_degree;
	std::string sparsification;
};

template <typename Distance, typename RadiusValue = Distance> struct GraphConstructionResult {
	using distance_type = Distance;
	using radius_type = RadiusValue;
	using edge_type = std::tuple<std::size_t, std::size_t, Distance>;

	std::vector<edge_type> edges;
	GraphConstructionMetadata<Distance, RadiusValue> metadata;
};

template <typename Distance, typename RadiusValue = Distance>
auto make_graph_construction_metadata(std::string strategy, std::size_t record_count, bool directed, bool self_loops,
									  bool exact, std::string edge_payload, std::string weighting,
									  std::string symmetrization, std::string normalization, std::string tie_break,
									  std::string sparsification = "none")
	-> GraphConstructionMetadata<Distance, RadiusValue>
{
	GraphConstructionMetadata<Distance, RadiusValue> metadata;
	metadata.strategy = std::move(strategy);
	metadata.record_count = record_count;
	metadata.directed = directed;
	metadata.self_loops = self_loops;
	metadata.exact = exact;
	metadata.edge_payload = std::move(edge_payload);
	metadata.weighting = std::move(weighting);
	metadata.symmetrization = std::move(symmetrization);
	metadata.normalization = std::move(normalization);
	metadata.tie_break = std::move(tie_break);
	metadata.sparsification = std::move(sparsification);
	return metadata;
}

template <typename Distance, typename RadiusValue = Distance>
auto make_graph_construction_result(
	std::vector<typename GraphConstructionResult<Distance, RadiusValue>::edge_type> edges,
	GraphConstructionMetadata<Distance, RadiusValue> metadata) -> GraphConstructionResult<Distance, RadiusValue>
{
	metadata.edge_count = edges.size();
	return GraphConstructionResult<Distance, RadiusValue>{std::move(edges), std::move(metadata)};
}

template <typename Distance>
auto make_exact_knn_graph_result(std::vector<typename GraphConstructionResult<Distance>::edge_type> edges,
								 std::size_t record_count, std::size_t k) -> GraphConstructionResult<Distance>
{
	auto metadata = make_graph_construction_metadata<Distance>("exact_knn", record_count, true, false, true,
															   "metric_distance", "none", "none", "none",
															   "distance_then_target_index");
	metadata.k = k;
	return make_graph_construction_result<Distance>(std::move(edges), std::move(metadata));
}

template <typename Distance, typename RadiusValue>
auto make_exact_radius_graph_result(
	std::vector<typename GraphConstructionResult<Distance, RadiusValue>::edge_type> edges, std::size_t record_count,
	RadiusValue radius) -> GraphConstructionResult<Distance, RadiusValue>
{
	auto metadata = make_graph_construction_metadata<Distance, RadiusValue>(
		"exact_radius", record_count, true, false, true, "metric_distance", "none", "none", "none",
		"source_then_target_index");
	metadata.radius = radius;
	return make_graph_construction_result<Distance, RadiusValue>(std::move(edges), std::move(metadata));
}

template <typename Distance, typename RadiusValue>
auto make_symmetrized_graph_result(
	const GraphConstructionResult<Distance, RadiusValue> &source,
	std::vector<typename GraphConstructionResult<Distance, RadiusValue>::edge_type> edges, std::string policy,
	std::string weighting) -> GraphConstructionResult<Distance, RadiusValue>
{
	auto metadata = source.metadata;
	metadata.directed = false;
	metadata.self_loops = false;
	metadata.symmetrization = std::move(policy);
	metadata.weighting = std::move(weighting);
	metadata.tie_break = "source_index_then_target_index";
	return make_graph_construction_result<Distance, RadiusValue>(std::move(edges), std::move(metadata));
}

template <typename Distance, typename RadiusValue>
auto make_out_degree_pruned_graph_result(
	const GraphConstructionResult<Distance, RadiusValue> &source,
	std::vector<typename GraphConstructionResult<Distance, RadiusValue>::edge_type> edges, std::size_t max_out_degree)
	-> GraphConstructionResult<Distance, RadiusValue>
{
	auto metadata = source.metadata;
	metadata.max_out_degree = max_out_degree;
	metadata.sparsification = "out_degree";
	metadata.tie_break = "source_index_then_distance_then_target_index";
	return make_graph_construction_result<Distance, RadiusValue>(std::move(edges), std::move(metadata));
}

struct GraphDegreeDiagnostics {
	std::size_t record_count{};
	std::size_t edge_count{};
	bool directed{true};
	std::vector<std::size_t> degrees;
	std::vector<std::size_t> out_degrees;
	std::vector<std::size_t> in_degrees;
	std::size_t isolated_count{};
	std::size_t max_degree{};
	double average_degree{};
	std::string degree_policy;
};

using GraphDegreeSummary = ::mtrc::numeric::GraphDegreeSummary;

struct GraphConnectivityDiagnostics {
	std::size_t record_count{};
	std::size_t edge_count{};
	bool directed{true};
	std::vector<std::size_t> component_labels;
	std::size_t component_count{};
	std::size_t isolated_count{};
	std::size_t largest_component_size{};
	bool connected{true};
	std::string connectivity_policy;
};

using GraphConnectivitySummary = ::mtrc::numeric::GraphConnectivitySummary;
using GraphConnectivityLabeling = ::mtrc::numeric::GraphConnectivityLabeling;

struct GraphStretchDiagnostics {
	std::size_t record_count{};
	std::size_t edge_count{};
	bool directed{true};
	std::size_t pair_count{};
	std::size_t reachable_pair_count{};
	std::size_t unreachable_pair_count{};
	std::size_t zero_metric_pair_count{};
	double max_stretch{};
	double average_stretch{};
	std::string stretch_policy;
};

using GraphStretchSummary = ::mtrc::numeric::GraphStretchSummary;
using GraphStretchAccumulator = ::mtrc::numeric::GraphStretchAccumulator;
using GraphComponentTracker = ::mtrc::numeric::GraphComponentTracker;

inline auto summarize_graph_degrees(const std::vector<std::size_t> &degrees) -> GraphDegreeSummary
{
	return ::mtrc::numeric::summarize_graph_degrees(degrees);
}

inline auto summarize_graph_components(const std::vector<std::size_t> &component_sizes) -> GraphConnectivitySummary
{
	return ::mtrc::numeric::summarize_graph_components(component_sizes);
}

inline auto apply_graph_degree_summary(GraphDegreeDiagnostics &diagnostics, const GraphDegreeSummary &summary) -> void
{
	::mtrc::numeric::apply_graph_degree_summary(diagnostics, summary);
}

inline auto apply_graph_connectivity_summary(GraphConnectivityDiagnostics &diagnostics,
											const GraphConnectivityLabeling &labeling,
											const GraphConnectivitySummary &summary) -> void
{
	::mtrc::numeric::apply_graph_connectivity_summary(diagnostics, labeling, summary);
}

inline auto apply_graph_stretch_summary(GraphStretchDiagnostics &diagnostics, const GraphStretchSummary &summary) -> void
{
	::mtrc::numeric::apply_graph_stretch_summary(diagnostics, summary);
}

inline auto make_graph_degree_diagnostics(std::size_t record_count, std::size_t edge_count, bool directed)
	-> GraphDegreeDiagnostics
{
	GraphDegreeDiagnostics diagnostics;
	diagnostics.record_count = record_count;
	diagnostics.edge_count = edge_count;
	diagnostics.directed = directed;
	diagnostics.degrees.assign(record_count, 0);
	diagnostics.out_degrees.assign(record_count, 0);
	diagnostics.in_degrees.assign(record_count, 0);
	diagnostics.degree_policy = directed ? "directed_in_out" : "undirected_endpoint";
	return diagnostics;
}

inline auto make_graph_connectivity_diagnostics(std::size_t record_count, std::size_t edge_count, bool directed)
	-> GraphConnectivityDiagnostics
{
	GraphConnectivityDiagnostics diagnostics;
	diagnostics.record_count = record_count;
	diagnostics.edge_count = edge_count;
	diagnostics.directed = directed;
	diagnostics.component_labels.assign(record_count, 0);
	diagnostics.connectivity_policy = directed ? "weak_undirected_reachability" : "undirected_reachability";
	return diagnostics;
}

inline auto make_graph_stretch_diagnostics(std::size_t record_count, std::size_t edge_count, bool directed)
	-> GraphStretchDiagnostics
{
	GraphStretchDiagnostics diagnostics;
	diagnostics.record_count = record_count;
	diagnostics.edge_count = edge_count;
	diagnostics.directed = directed;
	diagnostics.stretch_policy = directed ? "directed_shortest_path" : "undirected_shortest_path";
	return diagnostics;
}

inline auto observe_graph_degree_edge(GraphDegreeDiagnostics &diagnostics, std::size_t source_index,
									  std::size_t target_index) -> void
{
	::mtrc::numeric::observe_graph_degree_edge(diagnostics, source_index, target_index);
}

template <typename Edge>
auto checked_graph_edge_indices(const Edge &edge, std::size_t record_count,
								const char *message = "graph edge index exceeds metadata record_count")
	-> std::pair<std::size_t, std::size_t>
{
	return ::mtrc::numeric::checked_graph_edge_indices(edge, record_count, message);
}

template <typename Edge>
auto observe_graph_degree_edges(GraphDegreeDiagnostics &diagnostics, const std::vector<Edge> &edges) -> void
{
	::mtrc::numeric::observe_graph_degree_edges(diagnostics, edges);
}

template <typename Edge>
auto observe_graph_component_edges(GraphComponentTracker &component_tracker, const std::vector<Edge> &edges,
								  std::size_t record_count) -> void
{
	::mtrc::numeric::observe_graph_component_edges(component_tracker, edges, record_count);
}

inline auto make_graph_shortest_path_matrix(std::size_t record_count) -> std::vector<std::vector<double>>
{
	return ::mtrc::numeric::make_graph_shortest_path_matrix(record_count);
}

inline auto checked_graph_shortest_path_matrix_record_count(const std::vector<std::vector<double>> &shortest_paths)
	-> std::size_t
{
	return ::mtrc::numeric::checked_graph_shortest_path_matrix_record_count(shortest_paths);
}

template <typename Edge>
auto observe_graph_shortest_path_edge(std::vector<std::vector<double>> &shortest_paths, const Edge &edge, bool directed)
	-> void
{
	::mtrc::numeric::observe_graph_shortest_path_edge(shortest_paths, edge, directed);
}

template <typename Edge>
auto observe_graph_shortest_path_edges(std::vector<std::vector<double>> &shortest_paths, const std::vector<Edge> &edges,
									  bool directed) -> void
{
	::mtrc::numeric::observe_graph_shortest_path_edges(shortest_paths, edges, directed);
}

inline auto close_graph_shortest_paths(std::vector<std::vector<double>> &shortest_paths) -> void
{
	::mtrc::numeric::close_graph_shortest_paths(shortest_paths);
}

template <typename MetricDistance>
auto observe_graph_stretch_pair(GraphStretchAccumulator &stretch_accumulator,
								const std::vector<std::vector<double>> &shortest_paths,
								std::size_t source_index, std::size_t target_index,
								MetricDistance metric_distance_at) -> void
{
	::mtrc::numeric::observe_graph_stretch_pair(stretch_accumulator, shortest_paths, source_index, target_index,
												  metric_distance_at);
}

template <typename MetricDistance>
auto observe_graph_stretch_pairs(GraphStretchAccumulator &stretch_accumulator,
								 const std::vector<std::vector<double>> &shortest_paths,
								 const std::vector<std::pair<std::size_t, std::size_t>> &pairs,
								 MetricDistance metric_distance_at) -> void
{
	::mtrc::numeric::observe_graph_stretch_pairs(stretch_accumulator, shortest_paths, pairs, metric_distance_at);
}

template <typename Distance, typename RadiusValue>
auto graph_shortest_path_distances(const GraphConstructionResult<Distance, RadiusValue> &graph)
	-> std::vector<std::vector<double>>
{
	return ::mtrc::numeric::graph_shortest_path_distances(graph.metadata.record_count, graph.edges,
														   graph.metadata.directed);
}

inline auto graph_metric_pair_indices(std::size_t record_count, bool directed)
	-> std::vector<std::pair<std::size_t, std::size_t>>
{
	return ::mtrc::numeric::graph_index_pairs(record_count, directed);
}

template <typename Candidate>
auto select_graph_neighbor_candidates_by_distance(std::vector<Candidate> candidates, std::size_t neighbor_count)
	-> std::vector<Candidate>
{
	return ::mtrc::numeric::select_graph_distance_index_pairs_by_distance(std::move(candidates), neighbor_count);
}

template <typename Container, typename Metric>
auto pairwise_distance_matrix(const Container &records, Metric distance)
	-> std::vector<std::vector<typename detail::finite_space_t<Container, Metric>::distance_type>>
{
	(void)::mtrc::finite_space_detail::require_pairwise_matrix_budget(records.size(), ::mtrc::pairwise_matrix_options{});
	return ::mtrc::Space::from_records(records, std::move(distance)).pairwise_distances();
}

template <typename Container, typename Metric>
auto pairwise_distance_matrix(const Container &records, Metric distance, ::mtrc::pairwise_matrix_options options)
	-> std::vector<std::vector<typename detail::finite_space_t<Container, Metric>::distance_type>>
{
	(void)::mtrc::finite_space_detail::require_pairwise_matrix_budget(records.size(), options);
	return ::mtrc::Space::from_records(records, std::move(distance)).pairwise_distances(options);
}

template <typename Container, typename Metric>
auto nearest_neighbors(const Container &records, Metric distance, const detail::record_type_t<Container> &query,
					   unsigned k = 1)
{
	return ::mtrc::Space::from_records(records, std::move(distance)).neighbors(query, k);
}

template <typename Container, typename Metric>
auto range_neighbors(const Container &records, Metric distance, const detail::record_type_t<Container> &query,
					 typename detail::finite_space_t<Container, Metric>::distance_type radius)
{
	return ::mtrc::Space::from_records(records, std::move(distance)).within_radius(query, radius);
}

template <typename Container, typename Metric>
auto exact_knn_graph(const Container &records, Metric distance, std::size_t k, exact_graph_options options = {})
	-> GraphConstructionResult<typename detail::finite_space_t<Container, Metric>::distance_type>
{
	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;
	using edge_type = typename GraphConstructionResult<distance_type>::edge_type;

	std::vector<edge_type> edges;

	if (k == 0) {
		return make_exact_knn_graph_result<distance_type>(std::move(edges), records.size(), k);
	}

	const auto max_neighbors = records.empty() ? std::size_t{0} : records.size() - 1;
	if (k > max_neighbors) {
		throw std::invalid_argument("k cannot exceed the number of non-self neighbors");
	}

	(void)detail::require_graph_distance_budget(records.size(), true, options.max_distance_evaluations,
												"exact_knn_graph",
												"Pass exact_graph_options{0} only when unbounded exact graph "
												"construction is intentional.");

	const auto space = ::mtrc::Space::from_records(records, std::move(distance));
	edges.reserve(detail::checked_graph_size_product(records.size(), k,
													 "exact_knn_graph edge count exceeds size_t capacity"));

	for (std::size_t source_index = 0; source_index < records.size(); ++source_index) {
		std::vector<std::pair<distance_type, std::size_t>> candidates;
		candidates.reserve(records.size() - 1);

		for (std::size_t target_index = 0; target_index < records.size(); ++target_index) {
			if (source_index == target_index) {
				continue;
			}
			candidates.emplace_back(space.distance(source_index, target_index), target_index);
		}

		const auto selected_candidates = select_graph_neighbor_candidates_by_distance(std::move(candidates), k);
		for (const auto &candidate : selected_candidates) {
			edges.emplace_back(source_index, candidate.second, candidate.first);
		}
	}

	return make_exact_knn_graph_result<distance_type>(std::move(edges), records.size(), k);
}

template <typename Container, typename Metric>
auto exact_knn_graph_edges(const Container &records, Metric distance, std::size_t k,
						   exact_graph_options options = {}) -> std::vector<
	std::tuple<std::size_t, std::size_t, typename detail::finite_space_t<Container, Metric>::distance_type>>
{
	return exact_knn_graph(records, std::move(distance), k, options).edges;
}

template <typename Container, typename Metric, typename Radius>
auto exact_radius_graph(const Container &records, Metric distance, Radius radius, exact_graph_options options = {})
	-> GraphConstructionResult<
	typename detail::finite_space_t<Container, Metric>::distance_type,
	typename std::common_type<typename detail::finite_space_t<Container, Metric>::distance_type, Radius>::type>
{
	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;
	using edge_type = typename GraphConstructionResult<distance_type, comparison_type>::edge_type;

	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}

	std::vector<edge_type> edges;
	const auto threshold = static_cast<comparison_type>(radius);
	(void)detail::require_graph_distance_budget(records.size(), true, options.max_distance_evaluations,
												"exact_radius_graph",
												"Pass exact_graph_options{0} only when unbounded exact graph "
												"construction is intentional.");
	const auto space = ::mtrc::Space::from_records(records, std::move(distance));

	for (std::size_t source_index = 0; source_index < records.size(); ++source_index) {
		for (std::size_t target_index = 0; target_index < records.size(); ++target_index) {
			if (source_index == target_index) {
				continue;
			}
			const auto edge_distance = space.distance(source_index, target_index);
			if (static_cast<comparison_type>(edge_distance) <= threshold) {
				edges.emplace_back(source_index, target_index, edge_distance);
			}
		}
	}

	return make_exact_radius_graph_result<distance_type, comparison_type>(std::move(edges), records.size(), threshold);
}

template <typename Container, typename Metric, typename Radius>
auto exact_radius_graph_edges(const Container &records, Metric distance, Radius radius,
							  exact_graph_options options = {}) -> std::vector<
	std::tuple<std::size_t, std::size_t, typename detail::finite_space_t<Container, Metric>::distance_type>>
{
	return exact_radius_graph(records, std::move(distance), radius, options).edges;
}

template <typename Edge>
auto select_graph_out_edges_by_distance(std::vector<Edge> edges, std::size_t max_out_degree) -> std::vector<Edge>
{
	return ::mtrc::numeric::select_graph_edges_by_distance_then_target(std::move(edges), max_out_degree);
}

template <typename Edge>
auto group_graph_edges_by_source(const std::vector<Edge> &edges) -> std::map<std::size_t, std::vector<Edge>>
{
	return ::mtrc::numeric::group_graph_edges_by_source(edges);
}

template <typename Edge>
auto select_graph_out_edges_by_source(const std::vector<Edge> &edges, std::size_t max_out_degree) -> std::vector<Edge>
{
	return ::mtrc::numeric::select_graph_edges_by_source_then_distance(edges, max_out_degree);
}

inline auto checked_graph_symmetrization_policy(const std::string &policy) -> void
{
	::mtrc::numeric::checked_graph_symmetrization_policy(policy);
}

inline auto checked_graph_weighting_policy(const std::string &weighting) -> void
{
	::mtrc::numeric::checked_graph_weighting_policy(weighting);
}

template <typename Distance>
auto merge_graph_edge_weight(Distance lhs, Distance rhs, const std::string &weighting) -> Distance
{
	return ::mtrc::numeric::merge_graph_edge_weight(lhs, rhs, weighting);
}

template <typename Distance>
auto accumulate_graph_edge_weight(std::optional<Distance> &slot, Distance value, const std::string &weighting) -> void
{
	::mtrc::numeric::accumulate_graph_edge_weight(slot, value, weighting);
}

template <typename Distance>
using GraphSymmetricEdgeAccumulator = ::mtrc::numeric::GraphSymmetricEdgeAccumulator<Distance>;

template <typename Distance>
auto accumulate_graph_symmetric_edge(
	std::map<std::pair<std::size_t, std::size_t>, GraphSymmetricEdgeAccumulator<Distance>> &edge_accumulators,
	std::size_t source_index, std::size_t target_index, Distance distance, const std::string &weighting) -> void
{
	::mtrc::numeric::accumulate_graph_symmetric_edge(edge_accumulators, source_index, target_index, distance,
													   weighting);
}

template <typename Distance>
auto accumulate_graph_symmetric_edges(const std::vector<std::tuple<std::size_t, std::size_t, Distance>> &edges,
									  const std::string &weighting)
	-> std::map<std::pair<std::size_t, std::size_t>, GraphSymmetricEdgeAccumulator<Distance>>
{
	return ::mtrc::numeric::accumulate_graph_symmetric_edges(edges, weighting);
}

template <typename Distance>
auto materialize_graph_symmetric_edge(const std::pair<std::size_t, std::size_t> &key,
									  const GraphSymmetricEdgeAccumulator<Distance> &accumulator,
									  const std::string &policy, const std::string &weighting)
	-> std::optional<std::tuple<std::size_t, std::size_t, Distance>>
{
	return ::mtrc::numeric::materialize_graph_symmetric_edge(key, accumulator, policy, weighting);
}

template <typename Distance>
auto materialize_graph_symmetric_edges(
	const std::map<std::pair<std::size_t, std::size_t>, GraphSymmetricEdgeAccumulator<Distance>> &edge_accumulators,
	const std::string &policy, const std::string &weighting) -> std::vector<std::tuple<std::size_t, std::size_t, Distance>>
{
	return ::mtrc::numeric::materialize_graph_symmetric_edges(edge_accumulators, policy, weighting);
}

template <typename Distance, typename RadiusValue>
auto symmetrize_graph(const GraphConstructionResult<Distance, RadiusValue> &graph, const std::string &policy = "union",
					  const std::string &weighting = "minimum_distance")
	-> GraphConstructionResult<Distance, RadiusValue>
{
	checked_graph_symmetrization_policy(policy);
	checked_graph_weighting_policy(weighting);

	const auto edge_accumulators = accumulate_graph_symmetric_edges(graph.edges, weighting);

	auto edges = ::mtrc::space::index::materialize_graph_symmetric_edges(edge_accumulators, policy, weighting);

	return make_symmetrized_graph_result(graph, std::move(edges), policy, weighting);
}

template <typename Distance, typename RadiusValue>
auto prune_graph_out_degree(const GraphConstructionResult<Distance, RadiusValue> &graph, std::size_t max_out_degree)
	-> GraphConstructionResult<Distance, RadiusValue>
{
	if (!graph.metadata.directed) {
		throw std::invalid_argument("out-degree pruning requires a directed graph result");
	}

	auto edges = select_graph_out_edges_by_source(graph.edges, max_out_degree);

	return make_out_degree_pruned_graph_result(graph, std::move(edges), max_out_degree);
}

template <typename Distance, typename RadiusValue>
auto graph_degree_diagnostics(const GraphConstructionResult<Distance, RadiusValue> &graph) -> GraphDegreeDiagnostics
{
	auto result = make_graph_degree_diagnostics(graph.metadata.record_count, graph.edges.size(), graph.metadata.directed);

	::mtrc::space::index::observe_graph_degree_edges(result, graph.edges);

	const auto summary = summarize_graph_degrees(result.degrees);
	::mtrc::space::index::apply_graph_degree_summary(result, summary);

	return result;
}

template <typename Distance, typename RadiusValue>
auto graph_connectivity_diagnostics(const GraphConstructionResult<Distance, RadiusValue> &graph)
	-> GraphConnectivityDiagnostics
{
	auto result =
		make_graph_connectivity_diagnostics(graph.metadata.record_count, graph.edges.size(), graph.metadata.directed);

	GraphComponentTracker component_tracker(result.record_count);
	::mtrc::space::index::observe_graph_component_edges(component_tracker, graph.edges, result.record_count);

	const auto labeling = component_tracker.labeling();
	const auto summary = summarize_graph_components(labeling.component_sizes);
	::mtrc::space::index::apply_graph_connectivity_summary(result, labeling, summary);

	return result;
}

template <typename Container, typename Metric, typename Distance, typename RadiusValue>
auto graph_stretch_diagnostics(const Container &records, Metric distance,
							   const GraphConstructionResult<Distance, RadiusValue> &graph,
							   graph_stretch_options options = {}) -> GraphStretchDiagnostics
{
	auto result = make_graph_stretch_diagnostics(graph.metadata.record_count, graph.edges.size(), graph.metadata.directed);

	if (records.size() != result.record_count) {
		throw std::invalid_argument("graph metadata record_count must match records size");
	}

	detail::require_graph_shortest_path_budget(result.record_count, options, "graph_stretch_diagnostics");
	(void)detail::require_graph_distance_budget(
		result.record_count, graph.metadata.directed, options.max_metric_distance_evaluations,
		"graph_stretch_diagnostics",
		"Pass graph_stretch_options{0, 0, 0} only when unbounded stretch diagnostics are intentional.");

	const auto shortest_paths = graph_shortest_path_distances(graph);
	const auto space = ::mtrc::Space::from_records(records, std::move(distance));
	GraphStretchAccumulator stretch_accumulator;
	const auto metric_distance_at = [&](std::size_t source_index, std::size_t target_index) {
		return space.distance(source_index, target_index);
	};
	for (std::size_t source_index = 0; source_index < result.record_count; ++source_index) {
		const auto first_target = graph.metadata.directed ? std::size_t{0} : source_index + 1;
		for (std::size_t target_index = first_target; target_index < result.record_count; ++target_index) {
			if (source_index == target_index) {
				continue;
			}
			::mtrc::space::index::observe_graph_stretch_pair(stretch_accumulator, shortest_paths, source_index,
															 target_index, metric_distance_at);
		}
	}

	const auto stretch_summary = stretch_accumulator.summary();
	::mtrc::space::index::apply_graph_stretch_summary(result, stretch_summary);

	return result;
}

} // namespace mtrc::space::index

namespace mtrc::stats::structural_analysis {

namespace detail = ::mtrc::space::index::detail;

template <typename Container, typename Metric>
auto representative_indices(const Container &records, Metric distance, std::size_t k, std::size_t seed_index = 0)
	-> std::vector<std::size_t>
{
	if (k == 0) {
		return {};
	}
	if (records.empty()) {
		throw std::invalid_argument("cannot select representatives from an empty record set");
	}
	if (k > records.size()) {
		throw std::invalid_argument("k cannot exceed the number of records");
	}
	if (seed_index >= records.size()) {
		throw std::out_of_range("seed_index is outside the record set");
	}

	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;

	const auto space = ::mtrc::Space::from_records(records, std::move(distance));
	std::vector<std::size_t> selected = {seed_index};
	std::vector<bool> is_selected(records.size(), false);
	is_selected[seed_index] = true;

	std::vector<distance_type> nearest_selected_distances(records.size());
	for (std::size_t index = 0; index < records.size(); ++index) {
		nearest_selected_distances[index] = space.distance(index, seed_index);
	}

	while (selected.size() < k) {
		const auto next_index = ::mtrc::core::farthest_unselected_position(
			nearest_selected_distances, is_selected, "selected-record count does not match distance count",
			"failed to select the next representative");

		selected.push_back(next_index);
		is_selected[next_index] = true;
		for (std::size_t index = 0; index < nearest_selected_distances.size(); ++index) {
			nearest_selected_distances[index] =
				std::min(nearest_selected_distances[index], space.distance(index, next_index));
		}
	}

	return selected;
}

template <typename Container, typename Metric>
auto representatives(const Container &records, Metric distance, std::size_t k, std::size_t seed_index = 0)
	-> std::vector<detail::record_type_t<Container>>
{
	const auto selected = representative_indices(records, std::move(distance), k, seed_index);
	std::vector<detail::record_type_t<Container>> result;
	result.reserve(selected.size());
	for (const auto index : selected) {
		result.push_back(records[index]);
	}
	return result;
}

template <typename Container, typename Metric>
auto medoid_index(const Container &records, Metric distance) -> std::size_t
{
	if (records.empty()) {
		throw std::invalid_argument("cannot select a medoid from an empty record set");
	}

	const auto space = ::mtrc::Space::from_records(records, std::move(distance));
	const auto provider = detail::indexed_pairwise_distances(space);
	std::vector<::mtrc::RecordId> candidate_ids;
	candidate_ids.reserve(records.size());
	for (std::size_t index = 0; index < records.size(); ++index) {
		candidate_ids.push_back(provider.id(index));
	}

	const auto medoid_id = ::mtrc::core::minimum_total_distance_record_id(
		provider, candidate_ids, "cannot select a medoid from an empty record set",
		"candidate id is outside the record set");
	return provider.position_of(medoid_id);
}

template <typename Container, typename Metric>
auto medoid(const Container &records, Metric distance) -> detail::record_type_t<Container>
{
	return records[medoid_index(records, std::move(distance))];
}

template <typename Container, typename Metric, typename MinimumDistance>
auto separated_representative_indices(const Container &records, Metric distance, MinimumDistance minimum_distance)
	-> std::vector<std::size_t>
{
	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;
	using comparison_type = typename std::common_type<distance_type, MinimumDistance>::type;

	if (minimum_distance < MinimumDistance{}) {
		throw std::invalid_argument("minimum_distance must be non-negative");
	}
	if (records.empty()) {
		return {};
	}

	const auto threshold = static_cast<comparison_type>(minimum_distance);
	const auto space = ::mtrc::Space::from_records(records, std::move(distance));
	const auto provider = detail::indexed_pairwise_distances(space);
	std::vector<std::size_t> selected;
	std::vector<::mtrc::RecordId> selected_ids;

	for (std::size_t candidate_index = 0; candidate_index < records.size(); ++candidate_index) {
		const auto candidate_id = provider.id(candidate_index);
		if (::mtrc::core::record_is_separated_from_record_ids(
				provider, candidate_id, selected_ids, threshold, "candidate id is outside the record set",
				"selected representative id is outside the record set")) {
			selected.push_back(candidate_index);
			selected_ids.push_back(candidate_id);
		}
	}

	return selected;
}

template <typename Container, typename Metric, typename MinimumDistance>
auto separated_representatives(const Container &records, Metric distance, MinimumDistance minimum_distance)
	-> std::vector<detail::record_type_t<Container>>
{
	const auto selected = separated_representative_indices(records, std::move(distance), minimum_distance);
	std::vector<detail::record_type_t<Container>> result;
	result.reserve(selected.size());
	for (const auto index : selected) {
		result.push_back(records[index]);
	}
	return result;
}

template <typename Container, typename Metric, typename Radius>
auto coverage_representative_indices(const Container &records, Metric distance, Radius radius)
	-> std::vector<std::size_t>
{
	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;

	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}
	if (records.empty()) {
		return {};
	}

	const auto cover_radius = static_cast<distance_type>(radius);
	const auto space = ::mtrc::Space::from_records(records, std::move(distance));
	const auto provider = detail::indexed_pairwise_distances(space);
	std::vector<std::size_t> selected;
	std::vector<bool> covered(records.size(), false);
	std::size_t covered_count = 0;

	while (covered_count < records.size()) {
		const auto seed_index =
			::mtrc::core::first_unmarked_position(covered, "failed to select the next coverage representative");
		selected.push_back(seed_index);
		covered_count += ::mtrc::core::mark_records_within_radius(
			provider, provider.id(seed_index), cover_radius, covered, "coverage state count does not match record count",
			"coverage representative id is outside the record set");
	}

	return selected;
}

template <typename Container, typename Metric, typename Radius>
auto coverage_representatives(const Container &records, Metric distance, Radius radius)
	-> std::vector<detail::record_type_t<Container>>
{
	const auto selected = coverage_representative_indices(records, std::move(distance), radius);
	std::vector<detail::record_type_t<Container>> result;
	result.reserve(selected.size());
	for (const auto index : selected) {
		result.push_back(records[index]);
	}
	return result;
}

} // namespace mtrc::stats::structural_analysis

namespace mtrc::stats::properties {

namespace detail = ::mtrc::space::index::detail;

template <typename Container, typename Metric>
auto intrinsic_dimension(const Container &records, Metric distance) -> double
{
	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;

	const auto distances = ::mtrc::space::index::pairwise_distance_matrix(records, std::move(distance));
	const auto distance_at = [&distances](std::size_t row, std::size_t column) -> const distance_type & {
		return distances[row][column];
	};
	return ::mtrc::core::expansion_dimension(distance_at, records.size(), distance_type{});
}

} // namespace mtrc::stats::properties

#endif
