#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/metric/catalog.hpp"
#include "metric/stats/correlate/correlation.hpp"
#include "metric/stats/properties/describe.hpp"
#include "metric/stats/properties/intrinsic_dimension.hpp"
#include "metric/stats/properties/local_volume.hpp"
#include "metric/stats/sample/sample.hpp"
#include "metric/stats/structural_analysis/clustering.hpp"
#include "metric/stats/structural_analysis/outliers.hpp"
#include "metric/stats/structural_analysis/representatives.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		const auto difference = lhs - rhs;
		return difference < 0 ? -difference : difference;
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

template <typename Function> auto throws_out_of_range(Function run) -> bool
{
	try {
		run();
	} catch (const std::out_of_range &) {
		return true;
	}
	return false;
}

auto close(double lhs, double rhs, double tolerance = 1.0e-12) -> bool
{
	return std::abs(lhs - rhs) < tolerance;
}

} // namespace

int main()
{
	namespace properties = mtrc::stats::properties;
	namespace sample = mtrc::stats::sample;
	namespace structural = mtrc::stats::structural_analysis;

	const auto empty_space = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
	const auto empty_volume = properties::local_volume(empty_space, 0);
	assert(empty_volume.record_count == 0);
	assert(empty_volume.empty());
	assert(empty_volume.average_density == 0.0);
	assert(properties::intrinsic_dimension(empty_space) == 0.0);
	assert(throws_invalid_argument([&] { (void)sample::farthest_first(empty_space, 1); }));
	assert(throws_invalid_argument([&] { (void)sample::metric_walk(empty_space, 1); }));

	const auto single_space = mtrc::make_space(std::vector<int>{7}, AbsoluteDistance{});
	const auto single_volume = properties::local_volume(single_space, 0);
	assert((single_volume.counts == std::vector<std::size_t>{1}));
	assert((single_volume.densities == std::vector<double>{1.0}));
	assert(single_volume.minimum_count == 1);
	assert(single_volume.maximum_count == 1);
	assert(close(single_volume.average_count, 1.0));
	assert(properties::intrinsic_dimension(single_space) == 0.0);

	const auto identical_space = mtrc::make_space(std::vector<int>{3, 3, 3, 3}, AbsoluteDistance{});
	const auto identical_description = mtrc::stats::properties::describe_structure(identical_space);
	assert(identical_description.record_count == 4);
	assert(identical_description.pair_count == 6);
	assert(identical_description.zero_distance_pair_count == 6);
	assert(!identical_description.has_nonzero_distances);
	assert(identical_description.intrinsic_dimension == 0.0);

	const auto identical_volume = properties::density(identical_space, 0);
	assert(identical_volume.algorithm == "density");
	assert((identical_volume.counts == std::vector<std::size_t>{4, 4, 4, 4}));
	assert((identical_volume.densities == std::vector<double>{1.0, 1.0, 1.0, 1.0}));
	assert(identical_volume.minimum_count == 4);
	assert(identical_volume.maximum_count == 4);
	assert(close(identical_volume.average_density, 1.0));

	const auto uneven_space = mtrc::make_space(std::vector<int>{0, 0, 0, 1, 100}, AbsoluteDistance{});
	const auto zero_radius = properties::local_volume(uneven_space, 0);
	assert((zero_radius.counts == std::vector<std::size_t>{3, 3, 3, 1, 1}));
	assert(close(zero_radius.minimum_density, 0.2));
	assert(close(zero_radius.maximum_density, 0.6));
	assert(close(zero_radius.average_count, 2.2));

	const auto one_radius = properties::local_volume(uneven_space, 1);
	assert((one_radius.counts == std::vector<std::size_t>{4, 4, 4, 4, 1}));
	assert(close(one_radius.average_density, 17.0 / 25.0));

	const auto farthest = sample::farthest_first(uneven_space, 3);
	const auto farthest_again = sample::farthest_first(uneven_space, 3);
	assert(farthest.algorithm == "farthest_first");
	assert(farthest.representation == "metric_space");
	assert((farthest.positions == std::vector<std::size_t>{0, 4, 3}));
	assert(farthest.positions == farthest_again.positions);
	assert(farthest.record_ids == farthest_again.record_ids);
	assert(farthest.coverage_radius == 0);

	const auto walk = sample::metric_walk(uneven_space, 4);
	const auto walk_again = sample::metric_walk(uneven_space, 4);
	assert(walk.algorithm == "metric_walk");
	assert((walk.positions == std::vector<std::size_t>{0, 1, 2, 3}));
	assert(walk.positions == walk_again.positions);
	assert(walk.coverage_radius == 99);

	const auto regular = sample::regular_sample_positions(5, 3);
	assert((regular.positions == std::vector<std::size_t>{0, 1, 3}));
	const auto regular_offset = sample::regular_sample_positions(5, 3, 2);
	assert((regular_offset.positions == std::vector<std::size_t>{2, 3, 0}));
	assert(throws_invalid_argument([] { (void)sample::regular_sample_positions(0, 1); }));
	assert(throws_invalid_argument([] { (void)sample::regular_sample_positions(3, 4); }));
	assert(throws_out_of_range([] { (void)sample::regular_sample_positions(3, 1, 3); }));
	assert(throws_invalid_argument([&] { (void)sample::farthest_first(uneven_space, 6); }));
	assert(throws_out_of_range([&] {
		(void)sample::metric_walk(uneven_space, 1, sample::metric_walk_options(5));
	}));

	const std::vector<int> uneven_records = {0, 0, 0, 1, 100};
	assert((structural::representative_indices(uneven_records, AbsoluteDistance{}, 3) ==
			std::vector<std::size_t>{0, 4, 3}));
	assert(structural::medoid_index(uneven_records, AbsoluteDistance{}) == 0);
	assert(structural::medoid(uneven_records, AbsoluteDistance{}) == 0);
	assert((structural::separated_representative_indices(uneven_records, AbsoluteDistance{}, 2) ==
			std::vector<std::size_t>{0, 4}));
	assert((structural::coverage_representative_indices(uneven_records, AbsoluteDistance{}, 1) ==
			std::vector<std::size_t>{0, 4}));
	assert(throws_invalid_argument([&] {
		(void)structural::separated_representative_indices(uneven_records, AbsoluteDistance{}, -1);
	}));
	assert(throws_invalid_argument([&] {
		(void)structural::coverage_representative_indices(uneven_records, AbsoluteDistance{}, -1);
	}));
	assert(throws_invalid_argument([&] {
		const std::vector<int> empty_records;
		(void)structural::medoid_index(empty_records, AbsoluteDistance{});
	}));

	const auto dbscan_duplicates = structural::dbscan(identical_space, 0, 2);
	assert(dbscan_duplicates.cluster_count == 1);
	assert(dbscan_duplicates.noise_count == 0);
	assert((dbscan_duplicates.cluster_sizes == std::vector<std::size_t>{4}));
	assert(dbscan_duplicates.core_records.size() == 4);

	const auto all_noise = structural::find_outliers(uneven_space, structural::dbscan_options(1.0, 2));
	assert(all_noise.noise_count == 1);
	assert(all_noise.size() == 1);
	assert(all_noise[0].id == uneven_space.id(4));
	assert(all_noise[0].score == 99);

	assert(throws_invalid_argument([&] { (void)properties::local_volume(uneven_space, -1); }));
	assert(throws_invalid_argument([&] {
		(void)properties::local_volume(uneven_space, std::numeric_limits<double>::quiet_NaN());
	}));

	const std::vector<std::vector<double>> left = {{0.0}, {1.0}, {2.0}, {3.0}};
	const std::vector<std::vector<double>> right = {{0.0}, {1.0}, {4.0}, {9.0}};
	const auto first_mgc = mtrc::stats::correlate::mgc(left, mtrc::Euclidean<double>(), right, mtrc::Euclidean<double>());
	const auto second_mgc = mtrc::stats::correlate::mgc(left, mtrc::Euclidean<double>(), right, mtrc::Euclidean<double>());
	assert(close(first_mgc.value, second_mgc.value));
	assert(first_mgc.left_record_count == 4);
	assert(first_mgc.right_record_count == 4);
	assert(throws_invalid_argument([&] {
		const std::vector<std::vector<double>> too_small = {{0.0}};
		(void)mtrc::stats::correlate::mgc(too_small, mtrc::Euclidean<double>(), too_small, mtrc::Euclidean<double>());
	}));

	return 0;
}
