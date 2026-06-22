#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "metric/engine.hpp"

namespace {

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

template <typename Function> auto throws_invalid_argument(Function run) -> bool
{
	try {
		run();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

void dbscan_fixtures()
{
	namespace structural = mtrc::stats::structural_analysis;
	using IntGroups = mtrc::ClusteringResult<int>;

	auto empty_space = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(empty_space)> empty_table(empty_space);
	assert(throws_invalid_argument([&] { (void)structural::dbscan(empty_space, 1, 1); }));
	assert(throws_invalid_argument([&] { (void)structural::dbscan(empty_table, 1, 1); }));

	auto single_space = mtrc::make_space(std::vector<int>{7}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(single_space)> single_table(single_space);
	const auto single = structural::dbscan(single_table, 0, 1);
	assert(single.algorithm == "dbscan");
	assert(single.representation == "pairwise_distances");
	assert(single.record_count == 1);
	assert(single.cluster_count == 1);
	assert(single.noise_count == 0);
	assert(single.converged);
	assert((single.assignments == std::vector<std::size_t>{0}));
	assert((single.cluster_sizes == std::vector<std::size_t>{1}));
	assert((single.medoids == std::vector<mtrc::RecordId>{single_space.id(0)}));
	assert((single.core_records == std::vector<mtrc::RecordId>{single_space.id(0)}));
	assert(single.noise_records.empty());

	const auto single_live = structural::dbscan(single_space, 0, 1);
	assert(single_live.representation == "metric_space");
	assert(single_live.assignments == single.assignments);
	assert(single_live.medoids == single.medoids);

	auto duplicate_space = mtrc::make_space(std::vector<int>{0, 0, 10, 10}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(duplicate_space)> duplicate_table(duplicate_space);
	const auto duplicates = structural::dbscan(duplicate_table, 0, 2);
	assert((duplicates.assignments == std::vector<std::size_t>{0, 0, 1, 1}));
	assert((duplicates.cluster_sizes == std::vector<std::size_t>{2, 2}));
	assert((duplicates.medoids == std::vector<mtrc::RecordId>{duplicate_space.id(0), duplicate_space.id(2)}));
	assert((duplicates.core_records == std::vector<mtrc::RecordId>{
										 duplicate_space.id(0), duplicate_space.id(1), duplicate_space.id(2),
										 duplicate_space.id(3)}));
	assert(duplicates.noise_records.empty());

	auto sparse_space = mtrc::make_space(std::vector<int>{0, 5, 10}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(sparse_space)> sparse_table(sparse_space);
	const auto all_noise = structural::dbscan(sparse_table, 1, 2);
	assert(all_noise.cluster_count == 0);
	assert(all_noise.noise_count == 3);
	assert((all_noise.assignments == std::vector<std::size_t>{IntGroups::noise_label, IntGroups::noise_label,
															  IntGroups::noise_label}));
	assert(all_noise.cluster_sizes.empty());
	assert(all_noise.medoids.empty());
	assert(all_noise.core_records.empty());
	assert((all_noise.noise_records == std::vector<mtrc::RecordId>{
										 sparse_space.id(0), sparse_space.id(1), sparse_space.id(2)}));

	auto two_cluster_space = mtrc::make_space(std::vector<int>{0, 1, 10, 11}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(two_cluster_space)> two_cluster_table(two_cluster_space);
	const auto two_clusters = structural::dbscan(two_cluster_table, 1, 2);
	assert((two_clusters.assignments == std::vector<std::size_t>{0, 0, 1, 1}));
	assert((two_clusters.cluster_sizes == std::vector<std::size_t>{2, 2}));
	assert((two_clusters.medoids == std::vector<mtrc::RecordId>{two_cluster_space.id(0), two_cluster_space.id(2)}));
	assert(two_clusters.noise_records.empty());
}

void kmedoids_fixtures()
{
	namespace structural = mtrc::stats::structural_analysis;

	auto empty_space = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(empty_space)> empty_table(empty_space);
	assert(throws_invalid_argument([&] { (void)structural::kmedoids(empty_space, 1); }));
	assert(throws_invalid_argument([&] { (void)structural::kmedoids(empty_table, 1); }));

	auto single_space = mtrc::make_space(std::vector<int>{7}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(single_space)> single_table(single_space);
	const auto single = structural::kmedoids(single_table, 1);
	assert(single.algorithm == "kmedoids");
	assert(single.representation == "pairwise_distances");
	assert(single.record_count == 1);
	assert(single.cluster_count == 1);
	assert(single.converged);
	assert((single.assignments == std::vector<std::size_t>{0}));
	assert((single.cluster_sizes == std::vector<std::size_t>{1}));
	assert((single.medoids == std::vector<mtrc::RecordId>{single_space.id(0)}));

	const auto single_live = structural::kmedoids(single_space, 1);
	assert(single_live.representation == "metric_space");
	assert(single_live.assignments == single.assignments);
	assert(single_live.medoids == single.medoids);

	auto duplicate_space = mtrc::make_space(std::vector<int>{0, 0, 10, 10}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(duplicate_space)> duplicate_table(duplicate_space);
	const auto duplicates = structural::kmedoids(duplicate_table, 2);
	assert((duplicates.assignments == std::vector<std::size_t>{0, 0, 1, 1}));
	assert((duplicates.cluster_sizes == std::vector<std::size_t>{2, 2}));
	assert((duplicates.medoids == std::vector<mtrc::RecordId>{duplicate_space.id(0), duplicate_space.id(2)}));

	auto two_cluster_space = mtrc::make_space(std::vector<int>{0, 1, 10, 11}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(two_cluster_space)> two_cluster_table(two_cluster_space);
	const auto two_clusters = structural::kmedoids(two_cluster_table, 2);
	assert((two_clusters.assignments == std::vector<std::size_t>{0, 0, 1, 1}));
	assert((two_clusters.cluster_sizes == std::vector<std::size_t>{2, 2}));
	assert((two_clusters.medoids == std::vector<mtrc::RecordId>{two_cluster_space.id(0), two_cluster_space.id(2)}));

	auto tied_space = mtrc::make_space(std::vector<int>{0, 2, 4}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(tied_space)> tied_table(tied_space);
	const auto tied = structural::kmedoids(tied_table, 2);
	assert((tied.assignments == std::vector<std::size_t>{0, 1, 1}));
	assert((tied.cluster_sizes == std::vector<std::size_t>{1, 2}));
	assert((tied.medoids == std::vector<mtrc::RecordId>{tied_space.id(0), tied_space.id(1)}));
}

void affinity_propagation_fixtures()
{
	namespace structural = mtrc::stats::structural_analysis;

	auto empty_space = mtrc::make_space(std::vector<double>{}, DoubleAbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(empty_space)> empty_table(empty_space);
	assert(throws_invalid_argument([&] { (void)structural::affinity_propagation(empty_space); }));
	assert(throws_invalid_argument([&] { (void)structural::affinity_propagation(empty_table); }));

	auto single_space = mtrc::make_space(std::vector<double>{7.0}, DoubleAbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(single_space)> single_table(single_space);
	const auto single = structural::affinity_propagation(single_table);
	static_assert(std::is_same<decltype(single)::distance_type, double>::value);
	assert(single.algorithm == "affinity_propagation");
	assert(single.representation == "pairwise_distances");
	assert(single.record_count == 1);
	assert(single.cluster_count == 1);
	assert(single.iterations == 0);
	assert(single.converged);
	assert((single.assignments == std::vector<std::size_t>{0}));
	assert((single.cluster_sizes == std::vector<std::size_t>{1}));
	assert((single.medoids == std::vector<mtrc::RecordId>{single_space.id(0)}));

	const auto single_live = structural::affinity_propagation(single_space);
	assert(single_live.representation == "metric_space");
	assert(single_live.assignments == single.assignments);
	assert(single_live.medoids == single.medoids);

	auto two_cluster_space = mtrc::make_space(std::vector<double>{0.0, 0.1, 10.0, 10.1},
											 DoubleAbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(two_cluster_space)> two_cluster_table(two_cluster_space);
	const auto two_clusters = structural::affinity_propagation(two_cluster_table, 0.0);
	assert(two_clusters.converged);
	assert(two_clusters.cluster_count == 2);
	assert((two_clusters.assignments == std::vector<std::size_t>{0, 0, 1, 1}));
	assert((two_clusters.cluster_sizes == std::vector<std::size_t>{2, 2}));
	assert((two_clusters.medoids == std::vector<mtrc::RecordId>{two_cluster_space.id(1), two_cluster_space.id(2)}));
}

} // namespace

int main()
{
	dbscan_fixtures();
	kmedoids_fixtures();
	affinity_propagation_fixtures();
	return 0;
}
