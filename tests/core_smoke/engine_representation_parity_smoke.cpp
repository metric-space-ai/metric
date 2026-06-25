#include <cassert>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

namespace mtrc::core {
template <> struct metric_traits<AbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};
} // namespace mtrc::core

namespace {

auto close(double actual, double expected, double tolerance = 1e-12) -> bool
{
	return std::abs(actual - expected) < tolerance;
}

template <typename Result> auto assert_same_neighbors(const Result &actual, const Result &expected) -> void
{
	assert(actual.record_count == expected.record_count);
	assert(actual.requested_count == expected.requested_count);
	assert(actual.exact == expected.exact);
	assert(actual.operator_name == expected.operator_name);
	assert(actual.size() == expected.size());
	for (std::size_t index = 0; index < expected.size(); ++index) {
		assert(actual[index].id == expected[index].id);
		assert(actual[index].distance == expected[index].distance);
	}
}

template <typename Result> auto assert_same_groups(const Result &actual, const Result &expected) -> void
{
	assert(actual.assignments == expected.assignments);
	assert(actual.medoids == expected.medoids);
	assert(actual.core_records == expected.core_records);
	assert(actual.unassigned_records == expected.unassigned_records);
	assert(actual.cluster_sizes == expected.cluster_sizes);
	assert(actual.record_count == expected.record_count);
	assert(actual.cluster_count == expected.cluster_count);
	assert(actual.unassigned_count == expected.unassigned_count);
	assert(actual.converged == expected.converged);
	assert(actual.algorithm == expected.algorithm);
}

template <typename Result> auto assert_same_representatives(const Result &actual, const Result &expected) -> void
{
	assert(actual.representatives == expected.representatives);
	assert(actual.nearest_representative_distances == expected.nearest_representative_distances);
	assert(actual.record_count == expected.record_count);
	assert(actual.requested_count == expected.requested_count);
	assert(actual.coverage_radius == expected.coverage_radius);
	assert(close(actual.average_nearest_distance, expected.average_nearest_distance));
	assert(actual.exact == expected.exact);
	assert(actual.operator_name == expected.operator_name);
	assert(actual.strategy == expected.strategy);
}

template <typename Result> auto assert_same_compression(const Result &actual, const Result &expected) -> void
{
	assert(actual.source_record_ids == expected.source_record_ids);
	assert(actual.assignments == expected.assignments);
	assert(actual.nearest_representative_distances == expected.nearest_representative_distances);
	assert(actual.representative_multiplicities == expected.representative_multiplicities);
	assert(actual.representative_weights.size() == expected.representative_weights.size());
	for (std::size_t index = 0; index < expected.representative_weights.size(); ++index) {
		assert(close(actual.representative_weights[index], expected.representative_weights[index]));
	}
	assert(actual.source_record_count == expected.source_record_count);
	assert(actual.compressed_record_count == expected.compressed_record_count);
	assert(close(actual.compression_ratio, expected.compression_ratio));
	assert(actual.exact == expected.exact);
	assert(actual.operator_name == expected.operator_name);
	assert(actual.compression == expected.compression);
	assert(actual.strategy == expected.strategy);
	assert(actual.lossy == expected.lossy);
	assert(actual.inverse_supported == expected.inverse_supported);
	assert(actual.size() == expected.size());
	for (std::size_t index = 0; index < expected.size(); ++index) {
		assert(actual.space.record(actual.space.id(index)) == expected.space.record(expected.space.id(index)));
	}
}

template <typename Result> auto assert_same_outliers(const Result &actual, const Result &expected) -> void
{
	assert(actual.record_count == expected.record_count);
	assert(actual.cluster_count == expected.cluster_count);
	assert(actual.unassigned_count == expected.unassigned_count);
	assert(actual.exact == expected.exact);
	assert(actual.operator_name == expected.operator_name);
	assert(actual.strategy == expected.strategy);
	assert(actual.size() == expected.size());
	for (std::size_t index = 0; index < expected.size(); ++index) {
		assert(actual[index].id == expected[index].id);
		assert(actual[index].score == expected[index].score);
	}
}

template <typename Result> auto assert_same_mapping(const Result &actual, const Result &expected) -> void
{
	assert(actual.source_records == expected.source_records);
	assert(actual.representative_records == expected.representative_records);
	assert(actual.source_record_count == expected.source_record_count);
	assert(actual.inverse_supported == expected.inverse_supported);
	assert(actual.mapping == expected.mapping);
	assert(actual.strategy == expected.strategy);
	assert(actual.size() == expected.size());
	for (std::size_t index = 0; index < expected.size(); ++index) {
		assert(actual.space.record(actual.space.id(index)) == expected.space.record(expected.space.id(index)));
	}
}

template <typename Callable> auto assert_invalid_runtime_policy(Callable &&call, const std::string &expected) -> void
{
	bool rejected = false;
	try {
		call();
	} catch (const mtrc::InvalidRuntimePolicyError &error) {
		rejected = std::string(error.what()).find(expected) != std::string::npos;
	}
	assert(rejected);
}

template <typename Result> auto assert_same_structure(const Result &actual, const Result &expected) -> void
{
	assert(actual.record_count == expected.record_count);
	assert(actual.pair_count == expected.pair_count);
	assert(actual.zero_distance_pair_count == expected.zero_distance_pair_count);
	assert(actual.minimum_nonzero_distance == expected.minimum_nonzero_distance);
	assert(actual.maximum_distance == expected.maximum_distance);
	assert(close(actual.average_distance, expected.average_distance));
	assert(close(actual.intrinsic_dimension, expected.intrinsic_dimension));
	assert(actual.has_nonzero_distances == expected.has_nonzero_distances);
	assert(actual.exact == expected.exact);
	assert(actual.operator_name == expected.operator_name);
	assert(actual.strategy == expected.strategy);
}

} // namespace

int main()
{
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 10, 11, 30}, AbsoluteDistance{});
	const auto lazy_policy = mtrc::space::storage::exact();
	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());

	const auto query_neighbors = mtrc::find_neighbors(space, 2, 3, lazy_policy);
	assert(query_neighbors.representation == "metric_space");
	const auto tree_query_neighbors = mtrc::find_neighbors(space, 2, 3, mtrc::space::storage::using_cover_tree());
	assert(tree_query_neighbors.representation == "cover_tree_index");
	assert_same_neighbors(tree_query_neighbors, query_neighbors);
	const auto graph_query_neighbors = mtrc::find_neighbors(space, 2, 3, mtrc::space::storage::using_knn_graph(3));
	assert(graph_query_neighbors.representation == "knn_graph_index");
	assert_same_neighbors(graph_query_neighbors, query_neighbors);

	const auto id_neighbors = mtrc::find_neighbors(space, space.id(2), 2, lazy_policy);
	const auto matrix_id_neighbors = mtrc::find_neighbors(space, space.id(2), 2, materialized_policy);
	assert(matrix_id_neighbors.representation == "distance_table");
	assert_same_neighbors(matrix_id_neighbors, id_neighbors);
	const auto tree_id_neighbors = mtrc::find_neighbors(space, space.id(2), 2, mtrc::space::storage::using_cover_tree());
	assert(tree_id_neighbors.representation == "cover_tree_index");
	assert_same_neighbors(tree_id_neighbors, id_neighbors);
	const auto graph_id_neighbors = mtrc::find_neighbors(space, space.id(2), 2, mtrc::space::storage::using_knn_graph(2));
	assert(graph_id_neighbors.representation == "knn_graph_index");
	assert_same_neighbors(graph_id_neighbors, id_neighbors);

	bool rejected_underprovisioned_graph = false;
	try {
		(void)mtrc::find_neighbors(space, space.id(2), 2, mtrc::space::storage::using_knn_graph(1));
	} catch (const mtrc::RepresentationError &) {
		rejected_underprovisioned_graph = true;
	}
	assert(rejected_underprovisioned_graph);

	const auto groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2), lazy_policy);
	const auto matrix_groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2), materialized_policy);
	assert(matrix_groups.representation == "distance_table");
	assert_same_groups(matrix_groups, groups);

	const auto density_groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2), lazy_policy);
	const auto matrix_density_groups =
		mtrc::find_groups(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2), materialized_policy);
	assert(matrix_density_groups.representation == "distance_table");
	assert_same_groups(matrix_density_groups, density_groups);

	const auto outliers = mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2), lazy_policy);
	const auto matrix_outliers = mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2), materialized_policy);
	assert(matrix_outliers.representation == "distance_table");
	assert_same_outliers(matrix_outliers, outliers);

	const auto density_filtered =
		mtrc::density_filter(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2), lazy_policy);
	const auto matrix_density_filtered =
		mtrc::density_filter(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2), materialized_policy);
	assert(matrix_density_filtered.representation == "distance_table");
	assert_same_mapping(matrix_density_filtered, density_filtered);

	const auto representatives = mtrc::find_representatives(space, 3, lazy_policy);
	const auto matrix_representatives = mtrc::find_representatives(space, 3, materialized_policy);
	assert(matrix_representatives.representation == "distance_table");
	assert_same_representatives(matrix_representatives, representatives);

	const auto compressed = mtrc::compress(space, 3, lazy_policy);
	const auto matrix_compressed = mtrc::compress(space, 3, materialized_policy);
	assert(matrix_compressed.representation == "distance_table");
	assert_same_compression(matrix_compressed, compressed);

	const auto structure = mtrc::describe_structure(space, lazy_policy);
	const auto matrix_structure = mtrc::describe_structure(space, materialized_policy);
	assert(matrix_structure.representation == "distance_table");
	assert_same_structure(matrix_structure, structure);

	const auto mapped = mtrc::map(space, [](int record) { return record / 10; }, AbsoluteDistance{}, lazy_policy);
	assert(mapped.representation == "metric_space");
	assert(mapped.space.record(mapped.space.id(4)) == 3);
	const auto implicit_mapped = mtrc::map(
		space, [](int record) { return record / 10; }, AbsoluteDistance{}, mtrc::space::storage::using_implicit());
	assert(implicit_mapped.representation == "implicit");
	assert_same_mapping(implicit_mapped, mapped);
	assert_invalid_runtime_policy(
		[&space, &materialized_policy]() {
			(void)mtrc::map(space, [](int record) { return record / 10; }, AbsoluteDistance{}, materialized_policy);
		},
		"materialized mapping");
	assert_invalid_runtime_policy(
		[&space]() {
			(void)mtrc::map(
				space, [](int record) { return record / 10; }, AbsoluteDistance{}, mtrc::space::storage::approximate());
		},
		"approximate mapping");

	using vector_record = std::vector<double>;
	const auto vector_space = mtrc::make_space(
		std::vector<vector_record>{{0.0, 1.0, 2.0}, {0.0, 1.0, 3.0}, {0.0, 1.0, 4.0}, {5.0, 2.0, 1.0}},
		mtrc::Euclidean<double>{});
	const auto pcfa = mtrc::modify::map::pcfa_options(2);

	const auto reduced = mtrc::reduce(vector_space, pcfa);
	assert(reduced.representation == "metric_space");
	const auto runtime_reduced = mtrc::reduce(vector_space, pcfa, lazy_policy);
	assert(runtime_reduced.representation == "metric_space");
	assert_same_mapping(runtime_reduced, reduced);
	const auto implicit_reduced = mtrc::reduce(vector_space, pcfa, mtrc::space::storage::using_implicit());
	assert(implicit_reduced.representation == "implicit");
	assert_same_mapping(implicit_reduced, reduced);
	const auto direct_reduced = mtrc::reduce(vector_space, 2);
	assert_same_mapping(direct_reduced, reduced);
	const auto runtime_direct_reduced = mtrc::reduce(vector_space, 2, lazy_policy);
	assert_same_mapping(runtime_direct_reduced, reduced);

	assert_invalid_runtime_policy(
		[&vector_space, &pcfa, &materialized_policy]() {
			(void)mtrc::reduce(vector_space, pcfa, materialized_policy);
		},
		"materialized reduction");
	assert_invalid_runtime_policy(
		[&vector_space, &pcfa]() { (void)mtrc::reduce(vector_space, pcfa, mtrc::space::storage::approximate()); },
		"approximate reduction");

	const auto embedded = mtrc::embed(vector_space, pcfa);
	assert(embedded.mapping == "pcfa_embedding");
	assert(embedded.representation == "metric_space");
	const auto runtime_embedded = mtrc::embed(vector_space, pcfa, lazy_policy);
	assert(runtime_embedded.representation == "metric_space");
	assert_same_mapping(runtime_embedded, embedded);
	const auto implicit_embedded = mtrc::embed(vector_space, pcfa, mtrc::space::storage::using_implicit());
	assert(implicit_embedded.representation == "implicit");
	assert_same_mapping(implicit_embedded, embedded);

	assert_invalid_runtime_policy(
		[&vector_space, &pcfa, &materialized_policy]() {
			(void)mtrc::embed(vector_space, pcfa, materialized_policy);
		},
		"materialized embedding");
	assert_invalid_runtime_policy(
		[&vector_space, &pcfa]() { (void)mtrc::embed(vector_space, pcfa, mtrc::space::storage::approximate()); },
		"approximate embedding");

	const auto pcfa_mapping = mtrc::modify::map::pcfa(2);
	const auto fitted_mapped = mtrc::map(vector_space, pcfa_mapping);
	assert(fitted_mapped.mapping == "pcfa");
	assert(fitted_mapped.representation == "metric_space");
	assert_same_mapping(fitted_mapped, reduced);
	const auto runtime_fitted_mapped = mtrc::map(vector_space, pcfa_mapping, lazy_policy);
	assert(runtime_fitted_mapped.representation == "metric_space");
	assert_same_mapping(runtime_fitted_mapped, fitted_mapped);
	const auto implicit_fitted_mapped = mtrc::map(vector_space, pcfa_mapping, mtrc::space::storage::using_implicit());
	assert(implicit_fitted_mapped.representation == "implicit");
	assert_same_mapping(implicit_fitted_mapped, fitted_mapped);

	assert_invalid_runtime_policy(
		[&vector_space, &pcfa_mapping, &materialized_policy]() {
			(void)mtrc::map(vector_space, pcfa_mapping, materialized_policy);
		},
		"materialized mapping");
	assert_invalid_runtime_policy(
		[&vector_space, &pcfa_mapping]() {
			(void)mtrc::map(vector_space, pcfa_mapping, mtrc::space::storage::approximate());
		},
		"approximate mapping");

	const auto shifted = mtrc::make_space(std::vector<int>{0, 1, 9, 12, 29}, AbsoluteDistance{});
	const auto compared = mtrc::compare(space, shifted, lazy_policy);
	const auto matrix_compared = mtrc::compare(space, shifted, materialized_policy);
	assert(matrix_compared.left_representation == "distance_table");
	assert(matrix_compared.right_representation == "distance_table");
	assert(compared.left_record_count == matrix_compared.left_record_count);
	assert(compared.right_record_count == matrix_compared.right_record_count);
	assert(compared.exact == matrix_compared.exact);
	assert(compared.algorithm == matrix_compared.algorithm);
	assert(close(compared.value, matrix_compared.value));

	return 0;
}
