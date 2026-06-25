#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"

namespace {

using record_type = std::vector<double>;

struct CountingVectorDistance {
	std::shared_ptr<std::size_t> calls;

	auto operator()(const record_type &lhs, const record_type &rhs) const -> double
	{
		++(*calls);
		double total = 0.0;
		for (std::size_t index = 0; index < lhs.size(); ++index) {
			const double delta = lhs[index] - rhs[index];
			total += delta * delta;
		}
		return std::sqrt(total);
	}
};

auto close_to(double lhs, double rhs, double tolerance = 1.0e-9) -> bool { return std::abs(lhs - rhs) <= tolerance; }

template <typename Targets> auto assert_same_targets(const Targets &lhs, const Targets &rhs) -> void
{
	assert(lhs.dimensions == rhs.dimensions);
	assert(lhs.diffusion_steps == rhs.diffusion_steps);
	assert(lhs.record_count == rhs.record_count);
	assert(lhs.dense_distance_evaluations == rhs.dense_distance_evaluations);
	assert(lhs.max_dense_records == rhs.max_dense_records);
	assert(lhs.kernel_scale == rhs.kernel_scale);
	assert(lhs.method == rhs.method);
	assert(lhs.pairwise_distances == rhs.pairwise_distances);
	assert(lhs.affinity_kernel == rhs.affinity_kernel);
	assert(lhs.diffusion_operator == rhs.diffusion_operator);
	assert(lhs.coordinates.size() == rhs.coordinates.size());

	for (const auto &entry : lhs.coordinates) {
		const auto found = rhs.coordinates.find(entry.first);
		assert(found != rhs.coordinates.end());
		assert(found->second == entry.second);
	}
}

template <typename Function> auto rejects(Function &&function) -> bool
{
	try {
		function();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

} // namespace

int main()
{
	const std::vector<record_type> records{{0.0, 0.0}, {0.5, 0.5}, {1.0, 1.0}, {2.0, 2.0}};
	auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});

	mtrc::modify::map::DiffusionCoordinateSpec<double> spec;
	spec.dimensions = 2;
	spec.diffusion_steps = 3;
	spec.kernel_scale = 1.0;
	spec.max_dense_records = records.size();

	const auto targets = mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, spec);
	const auto repeated = mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, spec);
	assert_same_targets(targets, repeated);
	const auto cached_targets = mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
		space, spec, "distance_table_pairwise_distances");
	const auto exponential_targets = mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
		space, spec, "exact_space_distances", "exponential_affinity_kernel");
	const auto lazy_diffusion_targets = mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
		space, spec, "exact_space_distances", "gaussian_affinity_kernel",
		"lazy_row_normalized_diffusion_operator");

	assert(targets.coordinates.size() == records.size());
	assert(targets.dimensions == spec.dimensions);
	assert(targets.diffusion_steps == spec.diffusion_steps);
	assert(targets.record_count == records.size());
	assert(targets.dense_distance_evaluations == records.size() * records.size());
	assert(targets.max_dense_records == records.size());
	assert(targets.kernel_scale == spec.kernel_scale);
	assert(targets.method == "diffusion_potential_anchor_coordinates");
	assert(targets.pairwise_distances == "exact_space_distances");
	assert(targets.affinity_kernel == "gaussian_affinity_kernel");
	assert(targets.diffusion_operator == "row_normalized_diffusion_operator");
	assert(cached_targets.pairwise_distances == "distance_table_pairwise_distances");
	assert(cached_targets.affinity_kernel == "gaussian_affinity_kernel");
	assert(cached_targets.diffusion_operator == "row_normalized_diffusion_operator");
	assert(cached_targets.coordinates == targets.coordinates);
	assert(cached_targets.dense_distance_evaluations == targets.dense_distance_evaluations);
	assert(exponential_targets.pairwise_distances == "exact_space_distances");
	assert(exponential_targets.affinity_kernel == "exponential_affinity_kernel");
	assert(exponential_targets.diffusion_operator == "row_normalized_diffusion_operator");
	assert(exponential_targets.coordinates.size() == targets.coordinates.size());
	assert(exponential_targets.dense_distance_evaluations == targets.dense_distance_evaluations);
	assert(lazy_diffusion_targets.pairwise_distances == "exact_space_distances");
	assert(lazy_diffusion_targets.affinity_kernel == "gaussian_affinity_kernel");
	assert(lazy_diffusion_targets.diffusion_operator == "lazy_row_normalized_diffusion_operator");
	assert(lazy_diffusion_targets.coordinates.size() == targets.coordinates.size());
	assert(lazy_diffusion_targets.dense_distance_evaluations == targets.dense_distance_evaluations);

	for (std::size_t index = 0; index < space.size(); ++index) {
		const auto found = targets.coordinates.find(space.id(index));
		assert(found != targets.coordinates.end());
		assert(found->second.size() == spec.dimensions);
		for (const auto coordinate : found->second) {
			assert(std::isfinite(coordinate));
		}
	}

	mtrc::modify::map::DiffusionCoordinateSpec<double> auto_scale_spec;
	auto_scale_spec.dimensions = 1;
	auto_scale_spec.diffusion_steps = 1;
	const auto auto_scale_targets =
		mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, auto_scale_spec);
	assert(auto_scale_targets.kernel_scale > 0.0);
	assert(auto_scale_targets.dense_distance_evaluations == records.size() * records.size());
	assert(auto_scale_targets.max_dense_records == mtrc::modify::map::default_diffusion_coordinate_max_dense_records);

	{
		const auto default_guard_calls = std::make_shared<std::size_t>(0);
		std::vector<record_type> oversized_records(
			mtrc::modify::map::default_diffusion_coordinate_max_dense_records + 1, record_type{0.0});
		auto oversized_space = mtrc::make_space(oversized_records, CountingVectorDistance{default_guard_calls});
		mtrc::modify::map::DiffusionCoordinateSpec<double> default_guard_spec;
		default_guard_spec.dimensions = 1;
		default_guard_spec.diffusion_steps = 1;
		assert(rejects([&] {
			(void)mtrc::modify::map::diffusion_coordinate_targets<decltype(oversized_space), double>(
				oversized_space, default_guard_spec);
		}));
		assert(*default_guard_calls == 0);
	}

	mtrc::modify::map::DiffusionCoordinateSpec<double> repeated_anchor_spec;
	repeated_anchor_spec.dimensions = records.size() + 2;
	repeated_anchor_spec.diffusion_steps = 1;
	repeated_anchor_spec.kernel_scale = 1.0;
	const auto repeated_anchor_targets =
		mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, repeated_anchor_spec);
	assert(repeated_anchor_targets.dimensions == repeated_anchor_spec.dimensions);
	for (std::size_t index = 0; index < space.size(); ++index) {
		const auto found = repeated_anchor_targets.coordinates.find(space.id(index));
		assert(found != repeated_anchor_targets.coordinates.end());
		assert(found->second.size() == repeated_anchor_spec.dimensions);
		assert(std::isfinite(found->second.back()));
	}

	auto invalid = spec;
	invalid.dimensions = 0;
	assert(rejects([&] { (void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, invalid); }));

	invalid = spec;
	invalid.diffusion_steps = 0;
	assert(rejects([&] { (void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, invalid); }));

	invalid = spec;
	invalid.epsilon = 0.0;
	assert(rejects([&] { (void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, invalid); }));

	invalid = spec;
	invalid.max_dense_records = records.size() - 1;
	assert(rejects([&] { (void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, invalid); }));

	assert(rejects([&] {
		(void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, spec, "unknown_provider");
	}));
	assert(rejects([&] {
		(void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
			space, spec, "exact_space_distances", "unknown_affinity_kernel");
	}));
	assert(rejects([&] {
		(void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
			space, spec, "exact_space_distances", "gaussian_affinity_kernel", "unknown_diffusion");
	}));

	const std::vector<record_type> no_records;
	auto empty_space = mtrc::make_space(no_records, mtrc::Euclidean<double>{});
	assert(rejects(
		[&] { (void)mtrc::modify::map::diffusion_coordinate_targets<decltype(empty_space), double>(empty_space, spec); }));

	// tg1: hand-checkable exact target values. For any two distinct records the
	// 2x2 diffused operator has the form [[p, q], [q, p]] with p + q == 1, so the
	// per-column -log potential, centering, and unit-max-abs scaling collapse each
	// anchor column to exactly {-1, +1} (and its mirror), independent of the
	// separation distance and of the diffusion-step count.
	{
		const std::vector<record_type> pair_records{{0.0}, {1.0}};
		auto pair_space = mtrc::make_space(pair_records, mtrc::Euclidean<double>{});
		mtrc::modify::map::DiffusionCoordinateSpec<double> pair_spec;
		pair_spec.dimensions = 2;
		pair_spec.diffusion_steps = 1;
		pair_spec.kernel_scale = 1.0;
		const auto pair_targets =
			mtrc::modify::map::diffusion_coordinate_targets<decltype(pair_space), double>(pair_space, pair_spec);
		const auto &near0 = pair_targets.coordinates.at(pair_space.id(0));
		const auto &near1 = pair_targets.coordinates.at(pair_space.id(1));
		assert(near0.size() == 2 && near1.size() == 2);
		assert(close_to(near0[0], -1.0) && close_to(near0[1], 1.0));
		assert(close_to(near1[0], 1.0) && close_to(near1[1], -1.0));

		const std::vector<record_type> far_records{{0.0}, {2.0}};
		auto far_space = mtrc::make_space(far_records, mtrc::Euclidean<double>{});
		const auto far_targets =
			mtrc::modify::map::diffusion_coordinate_targets<decltype(far_space), double>(far_space, pair_spec);
		assert(close_to(far_targets.coordinates.at(far_space.id(0))[0], -1.0));
		assert(close_to(far_targets.coordinates.at(far_space.id(1))[0], 1.0));
	}

	// tg2: every output column is zero-mean and unit-max-abs by construction. The
	// 4-record space at the top is asymmetric, so neither column is degenerate.
	for (std::size_t column = 0; column < spec.dimensions; ++column) {
		double column_sum = 0.0;
		double column_max_abs = 0.0;
		for (std::size_t index = 0; index < space.size(); ++index) {
			const auto value = targets.coordinates.at(space.id(index))[column];
			column_sum += value;
			column_max_abs = std::max(column_max_abs, std::abs(value));
		}
		assert(std::abs(column_sum) <= 1.0e-9);
		assert(close_to(column_max_abs, 1.0));
	}

	// tg3: identical records yield no positive pairwise distances, so kernel_scale
	// falls back to 1 and the constant potential collapses to exactly zero
	// coordinates after centering and scaling.
	{
		const std::vector<record_type> identical_records{{1.0, 1.0}, {1.0, 1.0}, {1.0, 1.0}};
		auto identical_space = mtrc::make_space(identical_records, mtrc::Euclidean<double>{});
		mtrc::modify::map::DiffusionCoordinateSpec<double> identical_spec;
		identical_spec.dimensions = 2;
		identical_spec.diffusion_steps = 2;
		const auto identical_targets =
			mtrc::modify::map::diffusion_coordinate_targets<decltype(identical_space), double>(identical_space, identical_spec);
		assert(identical_targets.kernel_scale == 1.0);
		for (std::size_t index = 0; index < identical_space.size(); ++index) {
			const auto &coordinate = identical_targets.coordinates.at(identical_space.id(index));
			assert(coordinate.size() == 2);
			for (const auto value : coordinate) {
				assert(value == 0.0);
			}
		}
	}

	// tg4: a single-record space is a valid boundary; the anchor-column clamp
	// std::min(dimension, record_count - 1) never reads an out-of-range column and
	// the lone record collapses to zero coordinates.
	{
		const std::vector<record_type> single_record{{0.0, 0.0}};
		auto single_space = mtrc::make_space(single_record, mtrc::Euclidean<double>{});
		mtrc::modify::map::DiffusionCoordinateSpec<double> single_spec;
		single_spec.dimensions = 3;
		single_spec.diffusion_steps = 2;
		single_spec.kernel_scale = 1.0;
		const auto single_targets =
			mtrc::modify::map::diffusion_coordinate_targets<decltype(single_space), double>(single_space, single_spec);
		assert(single_targets.coordinates.size() == 1);
		const auto &coordinate = single_targets.coordinates.at(single_space.id(0));
		assert(coordinate.size() == 3);
		for (const auto value : coordinate) {
			assert(value == 0.0);
		}
	}

	// tg5: a tiny kernel scale underflows every off-diagonal affinity to zero, but
	// the unit self-affinity keeps each row sum positive, so target construction
	// stays finite and deterministic rather than throwing a zero-row error.
	{
		mtrc::modify::map::DiffusionCoordinateSpec<double> tiny_scale_spec;
		tiny_scale_spec.dimensions = 2;
		tiny_scale_spec.diffusion_steps = 1;
		tiny_scale_spec.kernel_scale = 1.0e-9;
		const auto tiny_scale_targets =
			mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, tiny_scale_spec);
		const auto repeated_tiny_scale =
			mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, tiny_scale_spec);
		assert_same_targets(tiny_scale_targets, repeated_tiny_scale);
		for (std::size_t index = 0; index < space.size(); ++index) {
			for (const auto value : tiny_scale_targets.coordinates.at(space.id(index))) {
				assert(std::isfinite(value));
			}
		}
	}

	return 0;
}
