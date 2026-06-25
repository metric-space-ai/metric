#include <cassert>
#include <cstddef>
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

struct CountingAbsoluteDistance {
	std::size_t *calls;

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

struct CountingDoubleAbsoluteDistance {
	std::size_t *calls;

	auto operator()(double lhs, double rhs) const -> double
	{
		++(*calls);
		const auto difference = lhs - rhs;
		return difference < 0.0 ? -difference : difference;
	}
};

struct PositionCountingProvider {
	using distance_type = int;

	std::vector<mtrc::RecordId> ids{mtrc::RecordId::from_index(10), mtrc::RecordId::from_index(20),
									  mtrc::RecordId::from_index(30), mtrc::RecordId::from_index(40)};
	std::size_t *id_distance_calls{};
	std::size_t *position_distance_calls{};
	std::size_t *guarded_position_distance_calls{};

	auto distance(mtrc::RecordId lhs, mtrc::RecordId rhs) const -> distance_type
	{
		++(*id_distance_calls);
		return distance_for_positions(position_of(lhs), position_of(rhs));
	}

	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position) const -> distance_type
	{
		++(*position_distance_calls);
		return distance_for_positions(lhs_position, rhs_position);
	}

	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position,
							  mtrc::space::storage::runtime_guard runtime) const -> distance_type
	{
		runtime.throw_if_cancelled("position-counting structural analysis distance");
		++(*guarded_position_distance_calls);
		return distance_for_positions(lhs_position, rhs_position);
	}

	auto record_count() const -> std::size_t { return ids.size(); }
	auto id(std::size_t position) const -> mtrc::RecordId { return ids.at(position); }
	auto position_of(mtrc::RecordId id) const -> std::size_t
	{
		for (std::size_t index = 0; index < ids.size(); ++index) {
			if (ids[index] == id) {
				return index;
			}
		}
		throw std::out_of_range("unknown position-counting provider id");
	}
	auto contains(mtrc::RecordId id) const -> bool
	{
		for (const auto candidate : ids) {
			if (candidate == id) {
				return true;
			}
		}
		return false;
	}
	auto version() const -> std::size_t { return 19; }
	auto is_stale() const -> bool { return false; }

	static auto distance_for_positions(std::size_t lhs_position, std::size_t rhs_position) -> distance_type
	{
		return lhs_position > rhs_position
				   ? static_cast<distance_type>(lhs_position - rhs_position)
				   : static_cast<distance_type>(rhs_position - lhs_position);
	}
};

auto integer_records(std::size_t count) -> std::vector<int>
{
	std::vector<int> records;
	records.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		records.push_back(static_cast<int>(index));
	}
	return records;
}

auto double_records(std::size_t count) -> std::vector<double>
{
	std::vector<double> records;
	records.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		records.push_back(static_cast<double>(index));
	}
	return records;
}

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
	assert(single.unassigned_count == 0);
	assert(single.converged);
	assert((single.assignments == std::vector<std::size_t>{0}));
	assert((single.cluster_sizes == std::vector<std::size_t>{1}));
	assert((single.medoids == std::vector<mtrc::RecordId>{single_space.id(0)}));
	assert((single.core_records == std::vector<mtrc::RecordId>{single_space.id(0)}));
	assert(single.unassigned_records.empty());

	const auto single_live = structural::dbscan(single_space, 0, 1);
	assert(single_live.representation == "metric_space");
	assert(single_live.exact);
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
	assert(duplicates.unassigned_records.empty());

	auto sparse_space = mtrc::make_space(std::vector<int>{0, 5, 10}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(sparse_space)> sparse_table(sparse_space);
	const auto all_unassigned = structural::dbscan(sparse_table, 1, 2);
	assert(all_unassigned.cluster_count == 0);
	assert(all_unassigned.unassigned_count == 3);
	assert((all_unassigned.assignments == std::vector<std::size_t>{IntGroups::unassigned_label, IntGroups::unassigned_label,
															  IntGroups::unassigned_label}));
	assert(all_unassigned.cluster_sizes.empty());
	assert(all_unassigned.medoids.empty());
	assert(all_unassigned.core_records.empty());
	assert((all_unassigned.unassigned_records == std::vector<mtrc::RecordId>{
											 sparse_space.id(0), sparse_space.id(1), sparse_space.id(2)}));

	auto two_cluster_space = mtrc::make_space(std::vector<int>{0, 1, 10, 11}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(two_cluster_space)> two_cluster_table(two_cluster_space);
	const auto two_clusters = structural::dbscan(two_cluster_table, 1, 2);
	assert((two_clusters.assignments == std::vector<std::size_t>{0, 0, 1, 1}));
	assert((two_clusters.cluster_sizes == std::vector<std::size_t>{2, 2}));
	assert((two_clusters.medoids == std::vector<mtrc::RecordId>{two_cluster_space.id(0), two_cluster_space.id(2)}));
	assert(two_clusters.unassigned_records.empty());
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
	assert(single_live.exact);
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

void position_based_hot_loop_provider_fixtures()
{
	namespace structural = mtrc::stats::structural_analysis;

	static_assert(mtrc::PairwiseDistances_v<PositionCountingProvider>);

	std::size_t id_distance_calls = 0;
	std::size_t position_distance_calls = 0;
	std::size_t guarded_position_distance_calls = 0;
	PositionCountingProvider provider{{mtrc::RecordId::from_index(10), mtrc::RecordId::from_index(20),
										mtrc::RecordId::from_index(30), mtrc::RecordId::from_index(40)},
									   &id_distance_calls, &position_distance_calls,
									   &guarded_position_distance_calls};

	auto reset_counts = [&] {
		id_distance_calls = 0;
		position_distance_calls = 0;
		guarded_position_distance_calls = 0;
	};
	auto assert_only_position_distances = [&] {
		assert(id_distance_calls == 0);
		assert(position_distance_calls == 0);
		assert(guarded_position_distance_calls > 0);
	};

	const auto medoid_groups = mtrc::find_groups(provider, structural::k_medoids_options(2));
	assert(medoid_groups.algorithm == "kmedoids");
	assert(medoid_groups.representation == "pairwise_distances");
	assert_only_position_distances();

	reset_counts();
	const auto density_groups = mtrc::find_groups(provider, structural::dbscan_options(1, 2));
	assert(density_groups.algorithm == "dbscan");
	assert(density_groups.representation == "pairwise_distances");
	assert_only_position_distances();

	reset_counts();
	const auto nearest_outliers = mtrc::nearest_neighbor_outliers(provider, 1);
	assert(nearest_outliers.strategy == "nearest_neighbor_distance");
	assert(nearest_outliers.outliers.size() == provider.record_count());
	assert_only_position_distances();

	reset_counts();
	const auto noise_outliers = mtrc::find_outliers(provider, structural::dbscan_options(0, 2));
	assert(noise_outliers.strategy == "dbscan_density_outlier");
	assert(noise_outliers.unassigned_count == provider.record_count());
	assert_only_position_distances();
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
	assert(single_live.exact);
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

void direct_large_space_preflight_refusals()
{
	namespace structural = mtrc::stats::structural_analysis;

	constexpr std::size_t large_record_count = 4097;

	std::size_t integer_distance_calls = 0;
	auto integer_space = mtrc::make_space(integer_records(large_record_count),
										  CountingAbsoluteDistance{&integer_distance_calls});
	assert(throws_invalid_argument([&] { (void)structural::kmedoids(integer_space, 2); }));
	assert(integer_distance_calls == 0);
	assert(throws_invalid_argument([&] { (void)structural::dbscan(integer_space, 1, 2); }));
	assert(integer_distance_calls == 0);

	std::size_t double_distance_calls = 0;
	auto double_space = mtrc::make_space(double_records(large_record_count),
										 CountingDoubleAbsoluteDistance{&double_distance_calls});
	assert(throws_invalid_argument([&] { (void)structural::affinity_propagation(double_space); }));
	assert(double_distance_calls == 0);
}

} // namespace

int main()
{
	dbscan_fixtures();
	kmedoids_fixtures();
	position_based_hot_loop_provider_fixtures();
	affinity_propagation_fixtures();
	direct_large_space_preflight_refusals();
	return 0;
}
