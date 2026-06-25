// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_RESULT_HPP
#define _METRIC_CORE_RESULT_HPP

#include <algorithm>
#include <cstddef>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "metric_traits.hpp"
#include "neighbor.hpp"
#include <metric/record/id.hpp>

namespace mtrc::core {

enum class search_route_kind {
	unspecified,
	source_metric,
	pairwise_provider,
	mapped_space,
	approximate_sampled_candidates,
	custom,
};

enum class result_domain_kind {
	unspecified,
	records,
	metric_space,
	pairwise_provider,
	neighbor_index,
	sampled_metric_space,
	mapped_space,
	custom,
};

inline auto search_route_kind_name(search_route_kind route) -> const char *
{
	switch (route) {
	case search_route_kind::source_metric:
		return "source_metric";
	case search_route_kind::pairwise_provider:
		return "pairwise_provider";
	case search_route_kind::mapped_space:
		return "mapped_space";
	case search_route_kind::approximate_sampled_candidates:
		return "approximate_sampled_candidates";
	case search_route_kind::custom:
		return "custom";
	case search_route_kind::unspecified:
		return "";
	}
	return "";
}

inline auto result_domain_kind_name(result_domain_kind domain) -> const char *
{
	switch (domain) {
	case result_domain_kind::records:
		return "records";
	case result_domain_kind::metric_space:
		return "metric_space";
	case result_domain_kind::pairwise_provider:
		return "pairwise_provider";
	case result_domain_kind::neighbor_index:
		return "neighbor_index";
	case result_domain_kind::sampled_metric_space:
		return "sampled_metric_space";
	case result_domain_kind::mapped_space:
		return "mapped_space";
	case result_domain_kind::custom:
		return "custom";
	case result_domain_kind::unspecified:
		return "";
	}
	return "";
}

struct SearchProvenance {
	std::string route;
	std::string mapping;
	std::string mapping_strategy;
	std::vector<RecordId> source_neighbor_ids;
	std::vector<std::vector<RecordId>> source_neighbor_lineage;
	std::vector<RecordId> representative_neighbor_ids;
	RecordId query_source_id{};
	bool has_query_source_id{false};
	bool representative_route{false};
	search_route_kind route_kind{search_route_kind::unspecified};
	result_domain_kind domain_kind{result_domain_kind::unspecified};
};

struct ApproximationQuality {
	std::string diagnostic;
	std::string candidate_policy;
	std::size_t candidate_count{};
	std::size_t candidate_universe{};
	std::size_t distance_evaluations{};
	std::size_t requested_count{};
	std::size_t sample_count{};
	std::size_t sample_universe{};
	std::size_t recall_reference_count{};
	std::size_t recall_matched_count{};
	std::size_t recall_distance_evaluations{};
	std::size_t recall_sample_query_count{};
	double candidate_fraction{1.0};
	double sample_fraction{1.0};
	double standard_error{};
	double confidence_radius_95{};
	bool recall_measured{false};
	double recall{};
	std::string reason;
};

inline auto default_search_route_kind_for_representation(const std::string &representation) -> search_route_kind
{
	if (representation == "pairwise_distances" || representation == "distance_table" ||
		representation == "symmetric_distance_table" || representation == "chunked_distance_table" ||
		representation == "blocked_distance_table") {
		return search_route_kind::pairwise_provider;
	}
	if (representation == "sampled_metric_space") {
		return search_route_kind::approximate_sampled_candidates;
	}
	if (representation == "landmark_index") {
		return search_route_kind::approximate_sampled_candidates;
	}
	if (representation == "mapped_metric_space") {
		return search_route_kind::mapped_space;
	}
	if (representation.empty()) {
		return search_route_kind::unspecified;
	}
	return search_route_kind::source_metric;
}

inline auto default_result_domain_kind_for_representation(const std::string &representation) -> result_domain_kind
{
	if (representation.empty()) {
		return result_domain_kind::unspecified;
	}
	if (representation == "records") {
		return result_domain_kind::records;
	}
	if (representation == "metric_space" || representation == "implicit") {
		return result_domain_kind::metric_space;
	}
	if (representation == "pairwise_distances" || representation == "distance_table" ||
		representation == "symmetric_distance_table" || representation == "chunked_distance_table" ||
		representation == "blocked_distance_table") {
		return result_domain_kind::pairwise_provider;
	}
	if (representation == "exact_scan_index" || representation == "cover_tree_index" ||
		representation == "knn_graph_index" || representation == "neighbor_index" ||
		representation == "landmark_index" || representation == "ball_tree_index") {
		return result_domain_kind::neighbor_index;
	}
	if (representation == "sampled_metric_space") {
		return result_domain_kind::sampled_metric_space;
	}
	if (representation == "mapped_metric_space") {
		return result_domain_kind::mapped_space;
	}
	return result_domain_kind::custom;
}

inline auto default_search_route_for_representation(const std::string &representation) -> std::string
{
	return search_route_kind_name(default_search_route_kind_for_representation(representation));
}

inline auto set_search_route(SearchProvenance &provenance, search_route_kind route) -> void
{
	provenance.route_kind = route;
	provenance.route = search_route_kind_name(route);
}

inline auto set_search_domain(SearchProvenance &provenance, result_domain_kind domain) -> void
{
	provenance.domain_kind = domain;
}

inline auto set_default_search_provenance(SearchProvenance &provenance, const std::string &representation) -> void
{
	set_search_route(provenance, default_search_route_kind_for_representation(representation));
	set_search_domain(provenance, default_result_domain_kind_for_representation(representation));
}

template <typename Distance> struct NeighborSet {
	using distance_type = Distance;
	using neighbor_type = Neighbor<Distance>;

	std::vector<neighbor_type> neighbors;
	std::size_t record_count{};
	std::size_t requested_count{};
	bool exact{true};
	std::string operator_name;
	std::string representation;
	SearchProvenance provenance;
	ApproximationQuality approximation_quality;

	auto size() const -> std::size_t { return neighbors.size(); }
	auto empty() const -> bool { return neighbors.empty(); }
	auto begin() const -> typename std::vector<neighbor_type>::const_iterator { return neighbors.begin(); }
	auto end() const -> typename std::vector<neighbor_type>::const_iterator { return neighbors.end(); }
	auto operator[](std::size_t index) const -> const neighbor_type & { return neighbors[index]; }
};

template <typename Distance>
auto make_neighbor_set(std::vector<Neighbor<Distance>> neighbors, std::size_t record_count,
					   std::size_t requested_count, std::string operator_name, std::string representation,
					   bool exact = true) -> NeighborSet<Distance>
{
	NeighborSet<Distance> result;
	result.neighbors = std::move(neighbors);
	result.record_count = record_count;
	result.requested_count = requested_count;
	result.exact = exact;
	result.operator_name = std::move(operator_name);
	result.representation = std::move(representation);
	set_default_search_provenance(result.provenance, result.representation);
	return result;
}

template <typename Distance>
auto nearest_neighbor_set(std::vector<Neighbor<Distance>> neighbors, std::size_t requested_count,
						  std::size_t record_count, std::string representation) -> NeighborSet<Distance>
{
	return make_neighbor_set(take_nearest_neighbors(std::move(neighbors), requested_count), record_count,
							 requested_count, "knn", std::move(representation));
}

template <typename Distance>
auto range_neighbor_set(std::vector<Neighbor<Distance>> neighbors, std::size_t record_count,
						std::string representation) -> NeighborSet<Distance>
{
	sort_neighbors(neighbors);
	const auto requested_count = neighbors.size();
	return make_neighbor_set(std::move(neighbors), record_count, requested_count, "range", std::move(representation));
}

template <typename Distance> struct ClusteringResult {
	using distance_type = Distance;
	static constexpr std::size_t unassigned_label = static_cast<std::size_t>(-1);

	std::vector<std::size_t> assignments;
	std::vector<RecordId> medoids;
	std::vector<RecordId> core_records;
	std::vector<RecordId> unassigned_records;
	std::vector<std::size_t> cluster_sizes;
	std::size_t record_count{};
	std::size_t cluster_count{};
	std::size_t unassigned_count{};
	std::size_t iterations{};
	bool converged{false};
	std::string algorithm;
	std::string representation;
	bool exact{true};

	auto empty() const -> bool { return assignments.empty(); }
};

template <typename Distance>
auto make_clustering_result(std::vector<std::size_t> assignments, std::vector<RecordId> medoids,
	std::vector<RecordId> core_records, std::vector<RecordId> unassigned_records,
	std::vector<std::size_t> cluster_sizes, std::size_t iterations, bool converged,
	std::string algorithm, std::string representation, bool exact = true) -> ClusteringResult<Distance>
{
	const auto record_count = assignments.size();
	const auto cluster_count = cluster_sizes.size();
	const auto unassigned_count = unassigned_records.size();

	auto result = ClusteringResult<Distance>{std::move(assignments),
											 std::move(medoids),
											 std::move(core_records),
											 std::move(unassigned_records),
											 std::move(cluster_sizes),
											 record_count,
											 cluster_count,
											 unassigned_count,
											 iterations,
											 converged,
											 std::move(algorithm),
											 std::move(representation)};
	result.exact = exact;
	return result;
}

template <typename Distance>
auto require_clustering_result_shape(const ClusteringResult<Distance> &clustering, std::size_t expected_record_count,
									 const char *record_count_message, const char *assignments_count_message) -> void
{
	if (clustering.record_count != expected_record_count) {
		throw std::invalid_argument(record_count_message);
	}
	if (clustering.assignments.size() != clustering.record_count) {
		throw std::invalid_argument(assignments_count_message);
	}
}

template <typename Source>
auto record_ids_excluding_assignment(const Source &source, const std::vector<std::size_t> &assignments,
									 std::size_t excluded_label) -> std::vector<RecordId>
{
	std::vector<RecordId> ids;
	ids.reserve(assignments.size());
	for (std::size_t position = 0; position < assignments.size(); ++position) {
		if (assignments[position] != excluded_label) {
			ids.push_back(source.id(position));
		}
	}
	return ids;
}

template <typename Source, typename Values, typename Value>
auto record_ids_matching_value(const Source &source, const Values &values, const Value &selected_value)
	-> std::vector<RecordId>
{
	std::vector<RecordId> ids;
	ids.reserve(values.size());
	for (std::size_t position = 0; position < values.size(); ++position) {
		if (values[position] == selected_value) {
			ids.push_back(source.id(position));
		}
	}
	return ids;
}

template <typename Source>
auto record_id_buckets_for_assignments(const Source &source, const std::vector<std::size_t> &assignments,
									   std::size_t bucket_count,
									   const char *out_of_range_message =
										   "record_id_buckets_for_assignments assignment is out of range")
	-> std::vector<std::vector<RecordId>>
{
	std::vector<std::vector<RecordId>> buckets(bucket_count);
	for (std::size_t position = 0; position < assignments.size(); ++position) {
		const auto label = assignments[position];
		if (label >= bucket_count) {
			throw std::invalid_argument(out_of_range_message);
		}
		buckets[label].push_back(source.id(position));
	}
	return buckets;
}

template <typename Source>
auto record_id_buckets_excluding_assignment(const Source &source, const std::vector<std::size_t> &assignments,
											std::size_t bucket_count, std::size_t excluded_label,
											const char *out_of_range_message =
												"record_id_buckets_excluding_assignment assignment is out of range")
	-> std::vector<std::vector<RecordId>>
{
	std::vector<std::vector<RecordId>> buckets(bucket_count);
	for (std::size_t position = 0; position < assignments.size(); ++position) {
		const auto label = assignments[position];
		if (label == excluded_label) {
			continue;
		}
		if (label >= bucket_count) {
			throw std::invalid_argument(out_of_range_message);
		}
		buckets[label].push_back(source.id(position));
	}
	return buckets;
}

template <typename Source, typename Distance>
auto unassigned_record_ids(const Source &source, const ClusteringResult<Distance> &clustering) -> std::vector<RecordId>
{
	return record_ids_matching_value(source, clustering.assignments, ClusteringResult<Distance>::unassigned_label);
}

template <typename Source, typename Distance>
auto cluster_member_record_ids(const Source &source, const ClusteringResult<Distance> &clustering,
							   const char *out_of_range_message =
								   "clustering assignment references an unknown cluster")
	-> std::vector<std::vector<RecordId>>
{
	return record_id_buckets_excluding_assignment(source, clustering.assignments, clustering.cluster_count,
												  ClusteringResult<Distance>::unassigned_label, out_of_range_message);
}

template <typename Source, typename Distance>
auto assigned_record_ids(const Source &source, const ClusteringResult<Distance> &clustering)
	-> std::vector<RecordId>
{
	return record_ids_excluding_assignment(source, clustering.assignments, ClusteringResult<Distance>::unassigned_label);
}

template <typename Distance> struct RepresentativeSet {
	using distance_type = Distance;

	std::vector<RecordId> representatives;
	std::vector<Distance> nearest_representative_distances;
	std::size_t record_count{};
	std::size_t requested_count{};
	Distance coverage_radius{};
	double average_nearest_distance{};
	bool exact{true};
	std::string operator_name;
	std::string strategy;
	std::string representation;

	auto size() const -> std::size_t { return representatives.size(); }
	auto empty() const -> bool { return representatives.empty(); }
	auto begin() const -> typename std::vector<RecordId>::const_iterator { return representatives.begin(); }
	auto end() const -> typename std::vector<RecordId>::const_iterator { return representatives.end(); }
	auto operator[](std::size_t index) const -> RecordId { return representatives[index]; }
};

template <typename Distance>
auto make_representative_set(std::vector<RecordId> representatives,
							 std::vector<Distance> nearest_representative_distances, std::size_t record_count,
							 std::size_t requested_count, std::string strategy, std::string representation,
							 bool exact = true) -> RepresentativeSet<Distance>
{
	RepresentativeSet<Distance> result;
	result.representatives = std::move(representatives);
	result.nearest_representative_distances = std::move(nearest_representative_distances);
	result.record_count = record_count;
	result.requested_count = requested_count;
	result.exact = exact;
	result.operator_name = "find_representatives";
	result.strategy = std::move(strategy);
	result.representation = std::move(representation);

	if (!result.nearest_representative_distances.empty()) {
		auto coverage_radius = result.nearest_representative_distances.front();
		double distance_sum = 0.0;
		for (const auto distance : result.nearest_representative_distances) {
			if (coverage_radius < distance) {
				coverage_radius = distance;
			}
			distance_sum += static_cast<double>(distance);
		}
		result.coverage_radius = coverage_radius;
		result.average_nearest_distance =
			distance_sum / static_cast<double>(result.nearest_representative_distances.size());
	}

	return result;
}

template <typename Space> struct CompressionResult {
	using space_type = Space;
	using distance_type = typename Space::distance_type;

	Space space;
	std::vector<RecordId> source_record_ids;
	std::vector<std::size_t> assignments;
	std::vector<distance_type> nearest_representative_distances;
	std::vector<std::size_t> representative_multiplicities;
	std::vector<double> representative_weights;
	std::size_t source_record_count{};
	std::size_t compressed_record_count{};
	double compression_ratio{};
	bool exact{true};
	std::string operator_name;
	std::string compression;
	std::string strategy;
	std::string representation;
	bool lossy{true};
	bool inverse_supported{false};
	// Provenance of the coarsened finite metric space. compression here means
	// record-set cardinality reduction (coverage-based coarsening), never
	// coordinate/dimension reduction. metric_status reports the metric law that
	// still holds over the kept records; validity describes the coverage bounds.
	metric_law metric_status{metric_law::unknown};
	std::string validity;

	auto size() const -> std::size_t { return space.size(); }
	auto empty() const -> bool { return space.empty(); }
};

inline auto compression_representative_multiplicities(const std::vector<std::size_t> &assignments,
													  std::size_t representative_count)
	-> std::vector<std::size_t>
{
	std::vector<std::size_t> multiplicities(representative_count, 0);
	for (const auto assignment : assignments) {
		if (assignment >= representative_count) {
			throw std::invalid_argument("compression assignment references an unknown representative");
		}
		++multiplicities[assignment];
	}
	return multiplicities;
}

inline auto compression_representative_weights(const std::vector<std::size_t> &representative_multiplicities,
											   std::size_t source_record_count) -> std::vector<double>
{
	std::vector<double> weights;
	weights.reserve(representative_multiplicities.size());
	if (source_record_count == 0) {
		weights.assign(representative_multiplicities.size(), 0.0);
		return weights;
	}
	for (const auto multiplicity : representative_multiplicities) {
		weights.push_back(static_cast<double>(multiplicity) / static_cast<double>(source_record_count));
	}
	return weights;
}

template <typename Space>
auto make_compression_result(Space space, std::vector<RecordId> source_record_ids,
							 std::vector<std::size_t> assignments,
							 std::vector<typename Space::distance_type> nearest_representative_distances,
							 std::size_t source_record_count, std::string compression, std::string strategy,
							 std::string representation, bool exact = true, bool lossy = true,
							 bool inverse_supported = false, metric_law metric_status = metric_law::unknown,
							 std::string validity = {}) -> CompressionResult<Space>
{
	const auto compressed_record_count = space.size();
	if (assignments.size() != source_record_count) {
		throw std::invalid_argument("compression assignments must match source record count");
	}
	if (nearest_representative_distances.size() != source_record_count) {
		throw std::invalid_argument("compression nearest distances must match source record count");
	}
	auto representative_multiplicities =
		compression_representative_multiplicities(assignments, compressed_record_count);
	auto representative_weights =
		compression_representative_weights(representative_multiplicities, source_record_count);
	const auto compression_ratio = source_record_count == 0
									   ? 0.0
									   : static_cast<double>(compressed_record_count) /
											 static_cast<double>(source_record_count);

	return CompressionResult<Space>{std::move(space),
									std::move(source_record_ids),
									std::move(assignments),
									std::move(nearest_representative_distances),
									std::move(representative_multiplicities),
									std::move(representative_weights),
									source_record_count,
									compressed_record_count,
									compression_ratio,
									exact,
									"compress",
									std::move(compression),
									std::move(strategy),
									std::move(representation),
									lossy,
									inverse_supported,
									metric_status,
									std::move(validity)};
}

template <typename Score> struct Outlier {
	using score_type = Score;

	RecordId id;
	Score score{};
};

template <typename Score> struct OutlierResult {
	using score_type = Score;
	using outlier_type = Outlier<Score>;

	std::vector<outlier_type> outliers;
	std::size_t record_count{};
	std::size_t cluster_count{};
	std::size_t unassigned_count{};
	bool exact{true};
	std::string operator_name;
	std::string strategy;
	std::string representation;
	ApproximationQuality approximation_quality;

	auto size() const -> std::size_t { return outliers.size(); }
	auto empty() const -> bool { return outliers.empty(); }
	auto begin() const -> typename std::vector<outlier_type>::const_iterator { return outliers.begin(); }
	auto end() const -> typename std::vector<outlier_type>::const_iterator { return outliers.end(); }
	auto operator[](std::size_t index) const -> const outlier_type & { return outliers[index]; }
};

template <typename Score>
auto make_outlier_result(std::vector<Outlier<Score>> outliers, std::size_t record_count, std::size_t cluster_count,
						 std::size_t unassigned_count, std::string strategy, std::string representation, bool exact = true)
	-> OutlierResult<Score>
{
	OutlierResult<Score> result;
	result.outliers = std::move(outliers);
	result.record_count = record_count;
	result.cluster_count = cluster_count;
	result.unassigned_count = unassigned_count;
	result.exact = exact;
	result.operator_name = "find_outliers";
	result.strategy = std::move(strategy);
	result.representation = std::move(representation);
	return result;
}

template <typename Score> auto sort_outliers(std::vector<Outlier<Score>> &outliers) -> void
{
	std::sort(outliers.begin(), outliers.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.score == rhs.score) {
			return lhs.id < rhs.id;
		}
		return lhs.score > rhs.score;
	});
}

template <typename Score, typename ScoreFor>
auto scored_outliers(const std::vector<RecordId> &ids, ScoreFor score_for) -> std::vector<Outlier<Score>>
{
	std::vector<Outlier<Score>> outliers;
	outliers.reserve(ids.size());
	for (const auto id : ids) {
		outliers.push_back(Outlier<Score>{id, static_cast<Score>(score_for(id))});
	}
	sort_outliers(outliers);
	return outliers;
}

// Explicit outcome of a differential-entropy estimate over a finite metric space. A
// negative differential entropy is a VALID result (estimator_status::valid), never a
// failure. Failures are distinguished so a caller can react: too_few_records (n < 4,
// below the estimator's minimum), degenerate (a valid-sized but information-free space,
// e.g. all-identical / zero-diameter records, where no local Gaussian could be calibrated), and
// estimator_failure (a non-finite estimate that is neither of the above).
enum class entropy_status { valid, too_few_records, degenerate, estimator_failure };

// Audit-friendly name for an entropy_status value, used by the result summaries
// below so a printed EntropyResult reports WHY a number is (or is not) valid.
inline auto entropy_status_name(entropy_status status) -> const char *
{
	switch (status) {
	case entropy_status::valid:
		return "valid";
	case entropy_status::too_few_records:
		return "too_few_records";
	case entropy_status::degenerate:
		return "degenerate";
	case entropy_status::estimator_failure:
		return "estimator_failure";
	}
	return "unknown";
}

template <typename Value = double> struct EntropyResult {
	using value_type = Value;

	Value value{};
	std::size_t record_count{};
	std::size_t neighbor_count{};
	std::size_t approximation_order{};
	// Effective neighbor count (k) and local approximation order (p) actually used by the
	// estimator after it clamps the requested values for small finite spaces. These can be
	// smaller than neighbor_count / approximation_order; they are 0 when no estimation was
	// attempted (too_few_records).
	std::size_t effective_neighbor_count{};
	std::size_t effective_approximation_order{};
	bool exponentiated{false};
	bool exact{true};
	entropy_status status{entropy_status::valid};
	std::string algorithm;
	std::string representation;

	auto succeeded() const -> bool { return status == entropy_status::valid; }
	auto failed() const -> bool { return status != entropy_status::valid; }
};

template <typename Value = double>
auto make_entropy_result(Value value, std::size_t record_count, std::size_t neighbor_count,
						 std::size_t approximation_order, bool exponentiated, std::string representation,
						 bool exact = true) -> EntropyResult<Value>
{
	EntropyResult<Value> result;
	result.value = value;
	result.record_count = record_count;
	result.neighbor_count = neighbor_count;
	result.approximation_order = approximation_order;
	result.effective_neighbor_count = neighbor_count;
	result.effective_approximation_order = approximation_order;
	result.exponentiated = exponentiated;
	result.exact = exact;
	result.status = entropy_status::valid;
	result.algorithm = "entropy";
	result.representation = std::move(representation);
	return result;
}

template <typename Value = double> struct CorrelationResult {
	using value_type = Value;

	Value value{};
	std::size_t left_record_count{};
	std::size_t right_record_count{};
	bool exact{true};
	std::string algorithm;
	std::string left_representation;
	std::string right_representation;
	std::size_t sample_count{};
	std::size_t sample_iterations{};
	std::string approximation_reason;
};

template <typename Value = double>
auto make_correlation_result(Value value, std::size_t left_record_count, std::size_t right_record_count,
							 std::string algorithm, std::string left_representation,
							 std::string right_representation, bool exact = true) -> CorrelationResult<Value>
{
	return CorrelationResult<Value>{value,
									left_record_count,
									right_record_count,
									exact,
									std::move(algorithm),
									std::move(left_representation),
									std::move(right_representation),
									0,
									0,
									{}};
}

template <typename Distance, typename Value = double> struct StructureDescription {
	using distance_type = Distance;
	using value_type = Value;

	std::size_t record_count{};
	std::size_t pair_count{};
	std::size_t zero_distance_pair_count{};
	Distance minimum_nonzero_distance{};
	Distance maximum_distance{};
	Value average_distance{};
	Value intrinsic_dimension{};
	bool has_nonzero_distances{false};
	bool exact{true};
	std::string operator_name;
	std::string strategy;
	std::string representation;
};

template <typename Distance, typename Value = double>
auto make_structure_description(std::size_t record_count, std::size_t pair_count,
								std::size_t zero_distance_pair_count, Distance minimum_nonzero_distance,
								Distance maximum_distance, Value average_distance, Value intrinsic_dimension,
								bool has_nonzero_distances, std::string representation, bool exact = true)
	-> StructureDescription<Distance, Value>
{
	return StructureDescription<Distance, Value>{record_count,
												pair_count,
												zero_distance_pair_count,
												minimum_nonzero_distance,
												maximum_distance,
												average_distance,
												intrinsic_dimension,
												has_nonzero_distances,
												exact,
												"describe_structure",
												"exact_all_pairs",
												std::move(representation)};
}

struct ModificationDiagnostics {
	std::string diagnostic;
	std::string policy;
	std::size_t source_record_count{};
	std::size_t target_record_count{};
	double radius{};
	double coverage_radius{};
	double average_assignment_distance{};
	double source_average_nearest_neighbor_distance{};
	double target_average_nearest_neighbor_distance{};
	double local_density_drift{};
	double local_volume_radius{};
	double source_average_local_volume_count{};
	double target_average_local_volume_count{};
	double local_volume_count_drift{};
	double source_average_local_volume_density{};
	double target_average_local_volume_density{};
	double local_volume_density_drift{};
	bool empirical_density_preserved{true};
	bool populated{false};
};

template <typename Space> struct MappingResult {
	using space_type = Space;
	using distance_type = typename Space::distance_type;

	Space space;
	std::vector<std::vector<RecordId>> source_records;
	std::vector<RecordId> representative_records;
	std::vector<std::size_t> assignments;
	std::vector<distance_type> nearest_representative_distances;
	std::vector<std::size_t> representative_multiplicities;
	std::vector<double> representative_weights;
	distance_type coverage_radius{};
	double average_assignment_distance{};
	bool has_assignment_summary{false};
	std::size_t source_record_count{};
	bool inverse_supported{false};
	std::string mapping;
	std::string strategy;
	std::string representation;
	// Provenance of the derived finite metric space:
	//  - metric_status: the metric law of the derived space's own metric
		//    (whether the derived records are still related by a true metric, a
		//    pseudometric, or only an ordered distance), reported so callers never
		//    have to assume a derived coordinate space is isometric to its source.
		//  - out_of_sample_supported: whether the mapping artifact can transform
	//    records that were not part of the source space.
	//  - validity: human/audit description of the validity bounds and admissible
	//    domain of the derived space (e.g. "selected records only", coordinate
	//    approximation, in-sample only).
	metric_law metric_status{metric_law::unknown};
	bool out_of_sample_supported{false};
	std::string validity;
	ModificationDiagnostics diagnostics;

	auto size() const -> std::size_t { return space.size(); }
	auto empty() const -> bool { return space.empty(); }
};

template <typename Space>
auto make_mapping_result(Space space, std::vector<std::vector<RecordId>> source_records,
						 std::vector<RecordId> representative_records, std::size_t source_record_count,
						 bool inverse_supported, std::string mapping, std::string strategy, std::string representation,
						 metric_law metric_status = metric_law::unknown, bool out_of_sample_supported = false,
						 std::string validity = {}, ModificationDiagnostics diagnostics = {},
						 std::vector<std::size_t> assignments = {},
						 std::vector<typename Space::distance_type> nearest_representative_distances = {},
						 std::vector<std::size_t> representative_multiplicities = {},
						 std::vector<double> representative_weights = {},
						 typename Space::distance_type coverage_radius = typename Space::distance_type{},
						 double average_assignment_distance = 0.0, bool has_assignment_summary = false)
	-> MappingResult<Space>
{
	return MappingResult<Space>{std::move(space),
								std::move(source_records),
								std::move(representative_records),
								std::move(assignments),
								std::move(nearest_representative_distances),
								std::move(representative_multiplicities),
								std::move(representative_weights),
								coverage_radius,
								average_assignment_distance,
								has_assignment_summary,
								source_record_count,
								inverse_supported,
								std::move(mapping),
								std::move(strategy),
								std::move(representation),
								metric_status,
								out_of_sample_supported,
								std::move(validity),
								std::move(diagnostics)};
}

template <typename Space>
auto require_mapping_result_contract(const MappingResult<Space> &result, const char *context = "mapping result") -> void
{
	const auto is_blank = [](const std::string &value) {
		return value.find_first_not_of(" \t\n\r") == std::string::npos;
	};
	const auto contains_id = [](const std::vector<RecordId> &ids, RecordId needle) {
		for (const auto id : ids) {
			if (id == needle) {
				return true;
			}
		}
		return false;
	};
	const auto has_duplicate_ids = [&contains_id](const std::vector<RecordId> &ids) {
		std::vector<RecordId> seen;
		seen.reserve(ids.size());
		for (const auto id : ids) {
			if (contains_id(seen, id)) {
				return true;
			}
			seen.push_back(id);
		}
		return false;
	};
	const auto derived_count = result.space.size();
	if (result.source_records.size() != derived_count) {
		throw std::invalid_argument(std::string(context) + ": source lineage count must match derived record count");
	}
	if (result.representative_records.size() != derived_count) {
		throw std::invalid_argument(std::string(context) + ": representative lineage count must match derived record count");
	}
	if (derived_count > 0 && result.source_record_count == 0) {
		throw std::invalid_argument(std::string(context) + ": non-empty derived space requires source record count");
	}
	std::vector<RecordId> unique_source_ids;
	for (std::size_t row_index = 0; row_index < result.source_records.size(); ++row_index) {
		const auto &row = result.source_records[row_index];
		if (row.empty()) {
			throw std::invalid_argument(std::string(context) + ": source lineage rows must not be empty");
		}
		if (has_duplicate_ids(row)) {
			throw std::invalid_argument(std::string(context) +
										": source lineage rows must not repeat a source record id");
		}
		if (!contains_id(row, result.representative_records[row_index])) {
			throw std::invalid_argument(std::string(context) +
										": representative record id must be included in the source lineage row");
		}
		for (const auto id : row) {
			if (!contains_id(unique_source_ids, id)) {
				unique_source_ids.push_back(id);
			}
		}
	}
	if (unique_source_ids.size() > result.source_record_count) {
		throw std::invalid_argument(std::string(context) +
									": unique source lineage count must not exceed source record count");
	}
	if (result.has_assignment_summary) {
		if (result.assignments.size() != result.source_record_count) {
			throw std::invalid_argument(std::string(context) +
										": assignment summary must match source record count");
		}
		if (result.nearest_representative_distances.size() != result.source_record_count) {
			throw std::invalid_argument(std::string(context) +
										": assignment distances must match source record count");
		}
		if (result.representative_multiplicities.size() != derived_count) {
			throw std::invalid_argument(std::string(context) +
										": representative multiplicities must match derived record count");
		}
		if (result.representative_weights.size() != derived_count) {
			throw std::invalid_argument(std::string(context) +
										": representative weights must match derived record count");
		}
	}
	if (is_blank(result.mapping)) {
		throw std::invalid_argument(std::string(context) + ": mapping name is required");
	}
	if (is_blank(result.strategy)) {
		throw std::invalid_argument(std::string(context) + ": mapping strategy is required");
	}
	if (is_blank(result.representation)) {
		throw std::invalid_argument(std::string(context) + ": derived representation is required");
	}
	if (is_blank(result.validity)) {
		throw std::invalid_argument(std::string(context) + ": validity description is required");
	}
}

// ---------------------------------------------------------------------------
// Human-readable one-line summaries for every result type.
//
// Each *Result struct already carries its own provenance (operator_name,
// representation, exact / metric_status / validity). These free operator<<
// overloads surface that provenance so a caller can answer "what did I just get
// back?" with a single `std::cout << result;` instead of hand-writing a field
// dump. They live in mtrc::core so they are found by ADL for the result types
// (which are aliased into mtrc). A generic `summary()` / `to_string()` returns
// the same text as a std::string for logging and test assertions.
// ---------------------------------------------------------------------------

inline auto exact_label(bool exact) -> const char * { return exact ? "exact" : "approx"; }

template <typename Distance>
auto operator<<(std::ostream &os, const NeighborSet<Distance> &result) -> std::ostream &
{
	os << "NeighborSet(op=" << result.operator_name << ", representation=" << result.representation
	   << ", size=" << result.size() << ", requested=" << result.requested_count
	   << ", records=" << result.record_count;
	if (!result.provenance.route.empty()) {
		os << ", route=" << result.provenance.route;
	}
	if (!result.exact && !result.approximation_quality.diagnostic.empty()) {
		os << ", candidates=" << result.approximation_quality.candidate_count << '/'
		   << result.approximation_quality.candidate_universe;
		if (result.approximation_quality.recall_measured) {
			os << ", recall=" << result.approximation_quality.recall;
		}
	}
	os << ", " << exact_label(result.exact) << ')';
	return os;
}

template <typename Distance>
auto operator<<(std::ostream &os, const ClusteringResult<Distance> &result) -> std::ostream &
{
	os << "ClusteringResult(algorithm=" << result.algorithm << ", representation=" << result.representation
	   << ", clusters=" << result.cluster_count << ", unassigned=" << result.unassigned_count
	   << ", records=" << result.record_count << ", iterations=" << result.iterations
	   << ", converged=" << (result.converged ? "yes" : "no") << ')';
	return os;
}

template <typename Distance>
auto operator<<(std::ostream &os, const RepresentativeSet<Distance> &result) -> std::ostream &
{
	os << "RepresentativeSet(op=" << result.operator_name << ", strategy=" << result.strategy
	   << ", representation=" << result.representation << ", size=" << result.size()
	   << ", requested=" << result.requested_count << ", records=" << result.record_count
	   << ", coverage_radius=" << result.coverage_radius
	   << ", average_nearest_distance=" << result.average_nearest_distance << ", " << exact_label(result.exact)
	   << ')';
	return os;
}

template <typename Space>
auto operator<<(std::ostream &os, const CompressionResult<Space> &result) -> std::ostream &
{
	os << "CompressionResult(op=" << result.operator_name << ", compression=" << result.compression
	   << ", strategy=" << result.strategy << ", representation=" << result.representation
	   << ", source_records=" << result.source_record_count << ", compressed_records=" << result.compressed_record_count
	   << ", ratio=" << result.compression_ratio << ", weights=" << result.representative_weights.size() << ", "
	   << (result.lossy ? "lossy" : "lossless")
	   << ", inverse=" << (result.inverse_supported ? "yes" : "no")
	   << ", metric_status=" << metric_law_name(result.metric_status) << ", " << exact_label(result.exact) << ')';
	return os;
}

template <typename Score> auto operator<<(std::ostream &os, const Outlier<Score> &outlier) -> std::ostream &
{
	os << "Outlier(id=" << outlier.id.index() << ", score=" << outlier.score << ')';
	return os;
}

template <typename Score> auto operator<<(std::ostream &os, const OutlierResult<Score> &result) -> std::ostream &
{
	os << "OutlierResult(op=" << result.operator_name << ", strategy=" << result.strategy
	   << ", representation=" << result.representation << ", outliers=" << result.size()
	   << ", records=" << result.record_count << ", clusters=" << result.cluster_count
	   << ", unassigned=" << result.unassigned_count;
	if (!result.exact && !result.approximation_quality.diagnostic.empty()) {
		os << ", candidates=" << result.approximation_quality.candidate_count << '/'
		   << result.approximation_quality.candidate_universe;
	}
	os << ", " << exact_label(result.exact) << ')';
	return os;
}

template <typename Value> auto operator<<(std::ostream &os, const EntropyResult<Value> &result) -> std::ostream &
{
	os << "EntropyResult(algorithm=" << result.algorithm << ", value=" << result.value
	   << ", status=" << entropy_status_name(result.status) << ", records=" << result.record_count
	   << ", k=" << result.neighbor_count << ", p=" << result.approximation_order
	   << ", effective_k=" << result.effective_neighbor_count << ", effective_p=" << result.effective_approximation_order
	   << ", " << (result.exponentiated ? "exponentiated" : "raw") << ", representation=" << result.representation
	   << ", " << exact_label(result.exact) << ')';
	return os;
}

template <typename Value> auto operator<<(std::ostream &os, const CorrelationResult<Value> &result) -> std::ostream &
{
	os << "CorrelationResult(algorithm=" << result.algorithm << ", value=" << result.value
	   << ", left_records=" << result.left_record_count << ", right_records=" << result.right_record_count
	   << ", left_representation=" << result.left_representation << ", right_representation=" << result.right_representation
	   << ", " << exact_label(result.exact);
	if (!result.exact && result.sample_count > 0) {
		os << ", sample_count=" << result.sample_count << ", sample_iterations=" << result.sample_iterations;
	}
	if (!result.approximation_reason.empty()) {
		os << ", reason=" << result.approximation_reason;
	}
	os << ')';
	return os;
}

template <typename Distance, typename Value>
auto operator<<(std::ostream &os, const StructureDescription<Distance, Value> &result) -> std::ostream &
{
	os << "StructureDescription(op=" << result.operator_name << ", representation=" << result.representation
	   << ", records=" << result.record_count << ", pairs=" << result.pair_count
	   << ", zero_distance_pairs=" << result.zero_distance_pair_count
	   << ", min_nonzero_distance=" << result.minimum_nonzero_distance
	   << ", max_distance=" << result.maximum_distance << ", average_distance=" << result.average_distance
	   << ", intrinsic_dimension=" << result.intrinsic_dimension << ", " << exact_label(result.exact) << ')';
	return os;
}

template <typename Space> auto operator<<(std::ostream &os, const MappingResult<Space> &result) -> std::ostream &
{
	os << "MappingResult(mapping=" << result.mapping << ", strategy=" << result.strategy
	   << ", representation=" << result.representation << ", derived_records=" << result.size()
	   << ", source_records=" << result.source_record_count
	   << ", inverse=" << (result.inverse_supported ? "yes" : "no")
	   << ", out_of_sample=" << (result.out_of_sample_supported ? "yes" : "no")
	   << ", metric_status=" << metric_law_name(result.metric_status) << ')';
	return os;
}

// Same text as operator<< but returned as a std::string, for logging and test
// assertions. SFINAE-constrained to streamable result types so it never becomes
// a catch-all; reached by ADL for the mtrc::core result types.
template <typename Result>
auto summary(const Result &result)
	-> decltype(std::declval<std::ostream &>() << result, std::string{})
{
	std::ostringstream stream;
	stream << result;
	return stream.str();
}

template <typename Result> auto to_string(const Result &result) -> decltype(summary(result)) { return summary(result); }

} // namespace mtrc::core

namespace mtrc {
	using core::summary;
	using core::to_string;
	using core::ApproximationQuality;
	template <typename Distance> using NeighborSet = core::NeighborSet<Distance>;
	using core::SearchProvenance;
using core::default_result_domain_kind_for_representation;
using core::default_search_route_for_representation;
using core::default_search_route_kind_for_representation;
using core::make_neighbor_set;
using core::nearest_neighbor_set;
using core::range_neighbor_set;
using core::result_domain_kind;
using core::result_domain_kind_name;
using core::search_route_kind;
using core::search_route_kind_name;
using core::set_default_search_provenance;
using core::set_search_domain;
using core::set_search_route;
template <typename Distance> using ClusteringResult = core::ClusteringResult<Distance>;
using core::cluster_member_record_ids;
using core::make_clustering_result;
using core::assigned_record_ids;
using core::record_id_buckets_excluding_assignment;
using core::record_id_buckets_for_assignments;
using core::record_ids_excluding_assignment;
using core::record_ids_matching_value;
using core::require_clustering_result_shape;
using core::unassigned_record_ids;
template <typename Distance> using RepresentativeSet = core::RepresentativeSet<Distance>;
using core::make_representative_set;
template <typename Space> using CompressionResult = core::CompressionResult<Space>;
using core::make_compression_result;
template <typename Score> using Outlier = core::Outlier<Score>;
template <typename Score> using OutlierResult = core::OutlierResult<Score>;
using core::make_outlier_result;
using core::scored_outliers;
using core::sort_outliers;
using core::entropy_status;
template <typename Value = double> using EntropyResult = core::EntropyResult<Value>;
using core::make_entropy_result;
template <typename Value = double> using CorrelationResult = core::CorrelationResult<Value>;
using core::make_correlation_result;
template <typename Distance, typename Value = double>
using StructureDescription = core::StructureDescription<Distance, Value>;
using core::make_structure_description;
template <typename Space> using MappingResult = core::MappingResult<Space>;
using core::make_mapping_result;
using core::require_mapping_result_contract;
} // namespace mtrc

#endif
