// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_GRAPHPRIMITIVES_H
#define METRIC_NUMERIC_MATH_GRAPHPRIMITIVES_H
#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace mtrc::numeric {

inline auto checked_graph_index_pair(std::size_t source_index, std::size_t target_index, std::size_t record_count,
									 const char *message = "graph index exceeds record_count")
	-> std::pair<std::size_t, std::size_t>
{
	if (source_index >= record_count || target_index >= record_count) {
		throw std::invalid_argument(message);
	}
	return {source_index, target_index};
}

template <typename Edge>
auto checked_graph_edge_indices(const Edge &edge, std::size_t record_count,
								const char *message = "graph edge index exceeds record_count")
	-> std::pair<std::size_t, std::size_t>
{
	return checked_graph_index_pair(std::get<0>(edge), std::get<1>(edge), record_count, message);
}

inline auto graph_index_pairs(std::size_t record_count, bool directed)
	-> std::vector<std::pair<std::size_t, std::size_t>>
{
	std::vector<std::pair<std::size_t, std::size_t>> pairs;
	if (record_count < 2) {
		return pairs;
	}

	if (directed) {
		pairs.reserve(record_count * (record_count - 1));
		for (std::size_t source_index = 0; source_index < record_count; ++source_index) {
			for (std::size_t target_index = 0; target_index < record_count; ++target_index) {
				if (source_index != target_index) {
					pairs.emplace_back(source_index, target_index);
				}
			}
		}
		return pairs;
	}

	pairs.reserve((record_count * (record_count - 1)) / 2);
	for (std::size_t source_index = 0; source_index < record_count; ++source_index) {
		for (std::size_t target_index = source_index + 1; target_index < record_count; ++target_index) {
			pairs.emplace_back(source_index, target_index);
		}
	}
	return pairs;
}

template <typename Candidate>
auto select_graph_distance_index_pairs_by_distance(std::vector<Candidate> candidates, std::size_t limit)
	-> std::vector<Candidate>
{
	std::sort(candidates.begin(), candidates.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.first < rhs.first) {
			return true;
		}
		if (rhs.first < lhs.first) {
			return false;
		}
		return lhs.second < rhs.second;
	});

	if (candidates.size() > limit) {
		candidates.resize(limit);
	}
	return candidates;
}

template <typename Edge>
auto select_graph_edges_by_distance_then_target(std::vector<Edge> edges, std::size_t limit) -> std::vector<Edge>
{
	std::sort(edges.begin(), edges.end(), [](const auto &lhs, const auto &rhs) {
		const auto lhs_distance = std::get<2>(lhs);
		const auto rhs_distance = std::get<2>(rhs);
		if (lhs_distance < rhs_distance) {
			return true;
		}
		if (rhs_distance < lhs_distance) {
			return false;
		}
		return std::get<1>(lhs) < std::get<1>(rhs);
	});

	if (edges.size() > limit) {
		edges.resize(limit);
	}
	return edges;
}

template <typename Edge>
auto group_graph_edges_by_source(const std::vector<Edge> &edges) -> std::map<std::size_t, std::vector<Edge>>
{
	std::map<std::size_t, std::vector<Edge>> edges_by_source;
	for (const auto &edge : edges) {
		edges_by_source[std::get<0>(edge)].push_back(edge);
	}
	return edges_by_source;
}

template <typename Edge>
auto select_graph_edges_by_source_then_distance(const std::vector<Edge> &edges, std::size_t limit) -> std::vector<Edge>
{
	std::vector<Edge> selected_edges;
	if (limit == 0) {
		return selected_edges;
	}

	auto edges_by_source = group_graph_edges_by_source(edges);
	for (auto &entry : edges_by_source) {
		const auto source_edges = select_graph_edges_by_distance_then_target(std::move(entry.second), limit);
		for (const auto &edge : source_edges) {
			selected_edges.push_back(edge);
		}
	}
	return selected_edges;
}

struct GraphDegreeSummary {
	std::size_t isolated_count{};
	std::size_t max_degree{};
	double average_degree{};
};

template <typename DegreeDiagnostics>
auto observe_graph_degree_edge(DegreeDiagnostics &diagnostics, std::size_t source_index, std::size_t target_index)
	-> void
{
	if (diagnostics.directed) {
		++diagnostics.out_degrees[source_index];
		++diagnostics.in_degrees[target_index];
		++diagnostics.degrees[source_index];
		++diagnostics.degrees[target_index];
		return;
	}

	++diagnostics.degrees[source_index];
	++diagnostics.degrees[target_index];
}

template <typename DegreeDiagnostics, typename Edge>
auto observe_graph_degree_edges(DegreeDiagnostics &diagnostics, const std::vector<Edge> &edges) -> void
{
	for (const auto &edge : edges) {
		const auto [source_index, target_index] =
			::mtrc::numeric::checked_graph_edge_indices(edge, diagnostics.record_count);
		::mtrc::numeric::observe_graph_degree_edge(diagnostics, source_index, target_index);
	}
}

struct GraphConnectivitySummary {
	std::size_t component_count{};
	std::size_t largest_component_size{};
	bool connected{true};
};

struct GraphConnectivityLabeling {
	std::vector<std::size_t> component_labels;
	std::vector<std::size_t> component_sizes;
	std::size_t isolated_count{};
};

struct GraphStretchSummary {
	std::size_t pair_count{};
	std::size_t reachable_pair_count{};
	std::size_t unreachable_pair_count{};
	// Counts pairs whose direct (reference) distance is zero. The field name is retained for the
	// stable serialization contract exposed by downstream adapters; the numeric core stays neutral.
	std::size_t zero_metric_pair_count{};
	double max_stretch{};
	double average_stretch{};
};

class GraphStretchAccumulator {
  public:
	auto observe_zero_metric_pair() -> void { ++zero_metric_pair_count_; }

	auto observe_unreachable_pair() -> void
	{
		++pair_count_;
		++unreachable_pair_count_;
	}

	auto observe_reachable_pair(double stretch) -> void
	{
		++pair_count_;
		++reachable_pair_count_;
		total_stretch_ += stretch;
		max_stretch_ = std::max(max_stretch_, stretch);
	}

	auto summary() const -> GraphStretchSummary
	{
		GraphStretchSummary result;
		result.pair_count = pair_count_;
		result.reachable_pair_count = reachable_pair_count_;
		result.unreachable_pair_count = unreachable_pair_count_;
		result.zero_metric_pair_count = zero_metric_pair_count_;
		result.max_stretch = max_stretch_;
		if (reachable_pair_count_ != 0) {
			result.average_stretch = total_stretch_ / static_cast<double>(reachable_pair_count_);
		}
		return result;
	}

  private:
	std::size_t pair_count_{};
	std::size_t reachable_pair_count_{};
	std::size_t unreachable_pair_count_{};
	std::size_t zero_metric_pair_count_{};
	double total_stretch_{};
	double max_stretch_{};
};

class GraphComponentTracker {
  public:
	explicit GraphComponentTracker(std::size_t record_count) : parents_(record_count), has_incident_edge_(record_count)
	{
		for (std::size_t index = 0; index < parents_.size(); ++index) {
			parents_[index] = index;
		}
	}

	auto observe_edge(std::size_t source_index, std::size_t target_index) -> void
	{
		has_incident_edge_[source_index] = true;
		has_incident_edge_[target_index] = true;
		union_components(source_index, target_index);
	}

	auto labeling() -> GraphConnectivityLabeling
	{
		GraphConnectivityLabeling result;
		result.component_labels.assign(parents_.size(), 0);

		std::map<std::size_t, std::size_t> root_labels;
		for (std::size_t index = 0; index < parents_.size(); ++index) {
			const auto root = find_root(index);
			auto label_it = root_labels.find(root);
			if (label_it == root_labels.end()) {
				const auto label = root_labels.size();
				label_it = root_labels.emplace(root, label).first;
				result.component_sizes.push_back(0);
			}

			const auto label = label_it->second;
			result.component_labels[index] = label;
			++result.component_sizes[label];
			if (!has_incident_edge_[index]) {
				++result.isolated_count;
			}
		}

		return result;
	}

  private:
	auto find_root(std::size_t index) -> std::size_t
	{
		while (parents_[index] != index) {
			parents_[index] = parents_[parents_[index]];
			index = parents_[index];
		}
		return index;
	}

	auto union_components(std::size_t lhs, std::size_t rhs) -> void
	{
		const auto lhs_root = find_root(lhs);
		const auto rhs_root = find_root(rhs);
		if (lhs_root == rhs_root) {
			return;
		}
		if (lhs_root < rhs_root) {
			parents_[rhs_root] = lhs_root;
		} else {
			parents_[lhs_root] = rhs_root;
		}
	}

	std::vector<std::size_t> parents_;
	std::vector<bool> has_incident_edge_;
};

template <typename Edge>
auto observe_graph_component_edges(GraphComponentTracker &component_tracker, const std::vector<Edge> &edges,
								  std::size_t record_count) -> void
{
	for (const auto &edge : edges) {
		const auto [source_index, target_index] = ::mtrc::numeric::checked_graph_edge_indices(edge, record_count);
		component_tracker.observe_edge(source_index, target_index);
	}
}

inline auto summarize_graph_degrees(const std::vector<std::size_t> &degrees) -> GraphDegreeSummary
{
	GraphDegreeSummary summary;
	std::size_t total_degree = 0;
	for (const auto degree : degrees) {
		total_degree += degree;
		summary.max_degree = std::max(summary.max_degree, degree);
		if (degree == 0) {
			++summary.isolated_count;
		}
	}
	if (!degrees.empty()) {
		summary.average_degree = static_cast<double>(total_degree) / static_cast<double>(degrees.size());
	}
	return summary;
}

inline auto summarize_graph_components(const std::vector<std::size_t> &component_sizes) -> GraphConnectivitySummary
{
	GraphConnectivitySummary summary;
	summary.component_count = component_sizes.size();
	for (const auto component_size : component_sizes) {
		summary.largest_component_size = std::max(summary.largest_component_size, component_size);
	}
	summary.connected = summary.component_count <= 1;
	return summary;
}

template <typename DegreeDiagnostics>
auto apply_graph_degree_summary(DegreeDiagnostics &diagnostics, const GraphDegreeSummary &summary) -> void
{
	diagnostics.isolated_count = summary.isolated_count;
	diagnostics.max_degree = summary.max_degree;
	diagnostics.average_degree = summary.average_degree;
}

template <typename ConnectivityDiagnostics>
auto apply_graph_connectivity_summary(ConnectivityDiagnostics &diagnostics,
									  const GraphConnectivityLabeling &labeling,
									  const GraphConnectivitySummary &summary) -> void
{
	diagnostics.component_labels = labeling.component_labels;
	diagnostics.isolated_count = labeling.isolated_count;
	diagnostics.component_count = summary.component_count;
	diagnostics.largest_component_size = summary.largest_component_size;
	diagnostics.connected = summary.connected;
}

template <typename StretchDiagnostics>
auto apply_graph_stretch_summary(StretchDiagnostics &diagnostics, const GraphStretchSummary &summary) -> void
{
	diagnostics.pair_count = summary.pair_count;
	diagnostics.reachable_pair_count = summary.reachable_pair_count;
	diagnostics.unreachable_pair_count = summary.unreachable_pair_count;
	diagnostics.zero_metric_pair_count = summary.zero_metric_pair_count;
	diagnostics.max_stretch = summary.max_stretch;
	diagnostics.average_stretch = summary.average_stretch;
}

inline auto checked_graph_symmetrization_policy(const std::string &policy) -> void
{
	if (policy != "union" && policy != "mutual") {
		throw std::invalid_argument("symmetrization policy must be 'union' or 'mutual'");
	}
}

inline auto checked_graph_weighting_policy(const std::string &weighting) -> void
{
	if (weighting != "minimum_distance" && weighting != "maximum_distance") {
		throw std::invalid_argument("weighting policy must be 'minimum_distance' or 'maximum_distance'");
	}
}

template <typename Distance> auto merge_graph_edge_weight(Distance lhs, Distance rhs, const std::string &weighting)
	-> Distance
{
	checked_graph_weighting_policy(weighting);
	if (weighting == "minimum_distance") {
		return std::min(lhs, rhs);
	}
	return std::max(lhs, rhs);
}

template <typename Distance>
auto accumulate_graph_edge_weight(std::optional<Distance> &slot, Distance value, const std::string &weighting) -> void
{
	checked_graph_weighting_policy(weighting);
	if (slot.has_value()) {
		slot = merge_graph_edge_weight(slot.value(), value, weighting);
	} else {
		slot = value;
	}
}

template <typename Distance> struct GraphSymmetricEdgeAccumulator {
	std::optional<Distance> forward;
	std::optional<Distance> reverse;
};

template <typename Distance>
auto accumulate_graph_symmetric_edge(
	std::map<std::pair<std::size_t, std::size_t>, GraphSymmetricEdgeAccumulator<Distance>> &edge_accumulators,
	std::size_t source_index, std::size_t target_index, Distance distance, const std::string &weighting) -> void
{
	if (source_index == target_index) {
		return;
	}

	const auto lower_index = std::min(source_index, target_index);
	const auto upper_index = std::max(source_index, target_index);
	auto &accumulator = edge_accumulators[{lower_index, upper_index}];
	if (source_index == lower_index) {
		accumulate_graph_edge_weight(accumulator.forward, distance, weighting);
	} else {
		accumulate_graph_edge_weight(accumulator.reverse, distance, weighting);
	}
}

template <typename Distance>
auto accumulate_graph_symmetric_edges(const std::vector<std::tuple<std::size_t, std::size_t, Distance>> &edges,
									  const std::string &weighting)
	-> std::map<std::pair<std::size_t, std::size_t>, GraphSymmetricEdgeAccumulator<Distance>>
{
	checked_graph_weighting_policy(weighting);

	std::map<std::pair<std::size_t, std::size_t>, GraphSymmetricEdgeAccumulator<Distance>> edge_accumulators;
	for (const auto &edge : edges) {
		const auto source_index = std::get<0>(edge);
		const auto target_index = std::get<1>(edge);
		const auto distance = std::get<2>(edge);
		accumulate_graph_symmetric_edge(edge_accumulators, source_index, target_index, distance, weighting);
	}
	return edge_accumulators;
}

template <typename Distance>
auto materialize_graph_symmetric_edge(const std::pair<std::size_t, std::size_t> &key,
									  const GraphSymmetricEdgeAccumulator<Distance> &accumulator,
									  const std::string &policy, const std::string &weighting)
	-> std::optional<std::tuple<std::size_t, std::size_t, Distance>>
{
	checked_graph_symmetrization_policy(policy);
	checked_graph_weighting_policy(weighting);

	const auto has_forward = accumulator.forward.has_value();
	const auto has_reverse = accumulator.reverse.has_value();

	if (policy == "mutual" && !(has_forward && has_reverse)) {
		return std::nullopt;
	}

	if (has_forward && has_reverse) {
		return std::make_tuple(key.first, key.second,
							   merge_graph_edge_weight(accumulator.forward.value(), accumulator.reverse.value(),
													   weighting));
	}
	if (has_forward) {
		return std::make_tuple(key.first, key.second, accumulator.forward.value());
	}
	if (has_reverse) {
		return std::make_tuple(key.first, key.second, accumulator.reverse.value());
	}
	return std::nullopt;
}

template <typename Distance>
auto materialize_graph_symmetric_edges(
	const std::map<std::pair<std::size_t, std::size_t>, GraphSymmetricEdgeAccumulator<Distance>> &edge_accumulators,
	const std::string &policy, const std::string &weighting) -> std::vector<std::tuple<std::size_t, std::size_t, Distance>>
{
	checked_graph_symmetrization_policy(policy);
	checked_graph_weighting_policy(weighting);

	std::vector<std::tuple<std::size_t, std::size_t, Distance>> edges;
	for (const auto &entry : edge_accumulators) {
		const auto edge = materialize_graph_symmetric_edge(entry.first, entry.second, policy, weighting);
		if (edge.has_value()) {
			edges.push_back(edge.value());
		}
	}
	return edges;
}

inline auto make_graph_shortest_path_matrix(std::size_t record_count) -> std::vector<std::vector<double>>
{
	const auto infinity = std::numeric_limits<double>::infinity();
	std::vector<std::vector<double>> shortest_paths(record_count, std::vector<double>(record_count, infinity));
	for (std::size_t index = 0; index < record_count; ++index) {
		shortest_paths[index][index] = 0.0;
	}
	return shortest_paths;
}

inline auto checked_graph_shortest_path_matrix_record_count(const std::vector<std::vector<double>> &shortest_paths)
	-> std::size_t
{
	const auto record_count = shortest_paths.size();
	for (const auto &row : shortest_paths) {
		if (row.size() != record_count) {
			throw std::invalid_argument("shortest path matrix must be square");
		}
	}
	return record_count;
}

template <typename Distance>
auto observe_graph_shortest_path_edge(std::vector<std::vector<double>> &shortest_paths, std::size_t source_index,
									  std::size_t target_index, Distance distance, bool directed) -> void
{
	const auto record_count = checked_graph_shortest_path_matrix_record_count(shortest_paths);
	checked_graph_index_pair(source_index, target_index, record_count,
							 "graph edge index exceeds shortest path matrix size");
	const auto edge_distance = static_cast<double>(distance);
	shortest_paths[source_index][target_index] = std::min(shortest_paths[source_index][target_index], edge_distance);
	if (!directed) {
		shortest_paths[target_index][source_index] = std::min(shortest_paths[target_index][source_index], edge_distance);
	}
}

template <typename Edge>
auto observe_graph_shortest_path_edge(std::vector<std::vector<double>> &shortest_paths, const Edge &edge, bool directed)
	-> void
{
	const auto record_count = checked_graph_shortest_path_matrix_record_count(shortest_paths);
	const auto [source_index, target_index] = ::mtrc::numeric::checked_graph_edge_indices(
		edge, record_count, "graph edge index exceeds shortest path matrix size");
	::mtrc::numeric::observe_graph_shortest_path_edge(shortest_paths, source_index, target_index, std::get<2>(edge),
														directed);
}

template <typename Edge>
auto observe_graph_shortest_path_edges(std::vector<std::vector<double>> &shortest_paths,
									  const std::vector<Edge> &edges, bool directed) -> void
{
	for (const auto &edge : edges) {
		::mtrc::numeric::observe_graph_shortest_path_edge(shortest_paths, edge, directed);
	}
}

inline auto close_graph_shortest_paths(std::vector<std::vector<double>> &shortest_paths) -> void
{
	const auto record_count = checked_graph_shortest_path_matrix_record_count(shortest_paths);
	const auto infinity = std::numeric_limits<double>::infinity();
	for (std::size_t through = 0; through < record_count; ++through) {
		for (std::size_t source_index = 0; source_index < record_count; ++source_index) {
			if (shortest_paths[source_index][through] == infinity) {
				continue;
			}
			for (std::size_t target_index = 0; target_index < record_count; ++target_index) {
				if (shortest_paths[through][target_index] == infinity) {
					continue;
				}
				shortest_paths[source_index][target_index] =
					std::min(shortest_paths[source_index][target_index],
							 shortest_paths[source_index][through] + shortest_paths[through][target_index]);
			}
		}
	}
}

template <typename Edge>
auto graph_shortest_path_distances(std::size_t record_count, const std::vector<Edge> &edges, bool directed)
	-> std::vector<std::vector<double>>
{
	auto shortest_paths = ::mtrc::numeric::make_graph_shortest_path_matrix(record_count);
	::mtrc::numeric::observe_graph_shortest_path_edges(shortest_paths, edges, directed);
	::mtrc::numeric::close_graph_shortest_paths(shortest_paths);
	return shortest_paths;
}

template <typename DirectDistance>
auto observe_graph_stretch_pair(GraphStretchAccumulator &stretch_accumulator,
								const std::vector<std::vector<double>> &shortest_paths,
								std::size_t source_index, std::size_t target_index,
								DirectDistance direct_distance_at) -> void
{
	const auto record_count = checked_graph_shortest_path_matrix_record_count(shortest_paths);
	checked_graph_index_pair(source_index, target_index, record_count,
							 "graph stretch pair index exceeds shortest path matrix size");

	const auto direct_distance = static_cast<double>(direct_distance_at(source_index, target_index));
	if (direct_distance == 0.0) {
		stretch_accumulator.observe_zero_metric_pair();
		return;
	}

	const auto path_distance = shortest_paths[source_index][target_index];
	if (path_distance == std::numeric_limits<double>::infinity()) {
		stretch_accumulator.observe_unreachable_pair();
		return;
	}

	const auto stretch = path_distance / direct_distance;
	stretch_accumulator.observe_reachable_pair(stretch);
}

template <typename DirectDistance>
auto observe_graph_stretch_pairs(GraphStretchAccumulator &stretch_accumulator,
								 const std::vector<std::vector<double>> &shortest_paths,
								 const std::vector<std::pair<std::size_t, std::size_t>> &pairs,
								 DirectDistance direct_distance_at) -> void
{
	for (const auto &pair : pairs) {
		::mtrc::numeric::observe_graph_stretch_pair(stretch_accumulator, shortest_paths, pair.first, pair.second,
													  direct_distance_at);
	}
}

} // namespace mtrc::numeric

#endif
