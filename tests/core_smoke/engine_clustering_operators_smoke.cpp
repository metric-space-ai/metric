#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct DoubleAbsoluteDistance {
	auto operator()(double lhs, double rhs) const -> double
	{
		const auto difference = lhs - rhs;
		return difference < 0.0 ? -difference : difference;
	}
};

struct LargeCountingDoubleProvider {
	using distance_type = double;

	std::size_t count{};
	std::size_t *calls{};

	auto record_count() const -> std::size_t { return count; }
	auto id(std::size_t position) const -> mtrc::RecordId { return mtrc::RecordId::from_index(position); }
	auto contains(mtrc::RecordId id) const -> bool { return id.index() < count; }
	auto position_of(mtrc::RecordId id) const -> std::size_t
	{
		if (!contains(id)) {
			throw std::out_of_range("unknown large provider id");
		}
		return id.index();
	}
	auto version() const -> std::size_t { return 1; }
	auto is_stale() const -> bool { return false; }
	auto distance(mtrc::RecordId lhs, mtrc::RecordId rhs) const -> distance_type
	{
		++(*calls);
		const auto lhs_position = lhs.index();
		const auto rhs_position = rhs.index();
		return lhs_position > rhs_position ? static_cast<double>(lhs_position - rhs_position)
										   : static_cast<double>(rhs_position - lhs_position);
	}
};

int main()
{
	auto space = mtrc::make_space(std::vector<int>{0, 1, 10, 11}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);

	const auto clustered = mtrc::stats::structural_analysis::kmedoids(matrix, 2);
	static_assert(std::is_same<decltype(clustered)::distance_type, int>::value);
	assert(clustered.algorithm == "kmedoids");
	assert(clustered.representation == "pairwise_distances");
	assert(clustered.record_count == space.size());
	assert(clustered.cluster_count == 2);
	assert(clustered.converged);
	assert(clustered.iterations > 0);
	assert(clustered.assignments.size() == space.size());
	assert(clustered.cluster_sizes.size() == 2);
	assert(clustered.medoids.size() == 2);
	assert(clustered.medoids[0] == space.id(0));
	assert(clustered.medoids[1] == space.id(2));
	assert(clustered.assignments[0] == clustered.assignments[1]);
	assert(clustered.assignments[2] == clustered.assignments[3]);
	assert(clustered.assignments[0] != clustered.assignments[2]);
	assert(clustered.cluster_sizes[0] == 2);
	assert(clustered.cluster_sizes[1] == 2);

	const auto implicit = mtrc::stats::structural_analysis::kmedoids(space, 2);
	assert(implicit.representation == "metric_space");
	assert(implicit.medoids == clustered.medoids);
	assert(implicit.assignments == clustered.assignments);

	bool rejected_too_many_clusters = false;
	try {
		(void)mtrc::stats::structural_analysis::kmedoids(matrix, 5);
	} catch (const std::invalid_argument &) {
		rejected_too_many_clusters = true;
	}
	assert(rejected_too_many_clusters);

	std::size_t kmedoids_distance_calls = 0;
	const LargeCountingDoubleProvider large_kmedoids_provider{
		mtrc::stats::structural_analysis::engine_detail::max_default_exact_metric_space_clustering_records + 1,
		&kmedoids_distance_calls};
	bool refused_large_kmedoids_provider = false;
	try {
		(void)mtrc::stats::structural_analysis::kmedoids(large_kmedoids_provider, 2);
	} catch (const mtrc::RepresentationError &error) {
		refused_large_kmedoids_provider = true;
		const std::string message = error.what();
		assert(message.find("kmedoids") != std::string::npos);
		assert(message.find("max_exact_records") != std::string::npos);
	}
	assert(refused_large_kmedoids_provider);
	assert(kmedoids_distance_calls == 0);

	std::size_t dbscan_distance_calls = 0;
	const LargeCountingDoubleProvider large_dbscan_provider{
		mtrc::stats::structural_analysis::engine_detail::max_default_exact_metric_space_clustering_records + 1,
		&dbscan_distance_calls};
	bool refused_large_dbscan_provider = false;
	try {
		(void)mtrc::stats::structural_analysis::dbscan(large_dbscan_provider, 1.0, 2);
	} catch (const mtrc::RepresentationError &error) {
		refused_large_dbscan_provider = true;
		const std::string message = error.what();
		assert(message.find("dbscan") != std::string::npos);
		assert(message.find("max_exact_records") != std::string::npos);
	}
	assert(refused_large_dbscan_provider);
	assert(dbscan_distance_calls == 0);

	auto continuous_space = mtrc::make_space(std::vector<double>{0.0, 0.1, 10.0, 10.1}, DoubleAbsoluteDistance{});
	const auto affinity_groups = mtrc::stats::structural_analysis::affinity_propagation(continuous_space, 0.7);
	static_assert(std::is_same<decltype(affinity_groups)::distance_type, double>::value);
	assert(affinity_groups.algorithm == "affinity_propagation");
	assert(affinity_groups.representation == "metric_space");
	assert(affinity_groups.record_count == continuous_space.size());
	assert(affinity_groups.assignments.size() == continuous_space.size());
	assert(affinity_groups.cluster_count == affinity_groups.medoids.size());
	assert(affinity_groups.cluster_count == affinity_groups.cluster_sizes.size());
	assert(!affinity_groups.empty());
	assert(affinity_groups.iterations > 0);

	mtrc::space::storage::DistanceTable<decltype(continuous_space)> continuous_matrix(continuous_space);
	const auto cached_affinity_groups = mtrc::stats::structural_analysis::affinity_propagation(continuous_matrix, 0.7);
	assert(cached_affinity_groups.algorithm == "affinity_propagation");
	assert(cached_affinity_groups.representation == "pairwise_distances");
	assert(cached_affinity_groups.record_count == continuous_space.size());
	assert(cached_affinity_groups.assignments == affinity_groups.assignments);
	assert(cached_affinity_groups.medoids == affinity_groups.medoids);
	assert(cached_affinity_groups.cluster_sizes == affinity_groups.cluster_sizes);

	bool rejected_invalid_preference = false;
	try {
		(void)mtrc::stats::structural_analysis::affinity_propagation(continuous_space, 1.0);
	} catch (const std::invalid_argument &) {
		rejected_invalid_preference = true;
	}
	assert(rejected_invalid_preference);

	std::size_t affinity_distance_calls = 0;
	const LargeCountingDoubleProvider large_affinity_provider{
		mtrc::stats::structural_analysis::engine_detail::max_default_exact_metric_space_clustering_records + 1,
		&affinity_distance_calls};
	bool refused_large_affinity_provider = false;
	try {
		(void)mtrc::stats::structural_analysis::affinity_propagation(large_affinity_provider, 0.5);
	} catch (const mtrc::RepresentationError &error) {
		refused_large_affinity_provider = true;
		const std::string message = error.what();
		assert(message.find("affinity_propagation") != std::string::npos);
		assert(message.find("max_dense_records") != std::string::npos);
	}
	assert(refused_large_affinity_provider);
	assert(affinity_distance_calls == 0);

	return 0;
}
