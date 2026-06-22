#include <cassert>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/metric/catalog/vector/Standards.hpp"
#include "metric/modify/compose/pipeline.hpp"
#include "metric/modify/dynamics/diffusion.hpp"
#include "metric/modify/expand/generated.hpp"
#include "metric/modify/map/map.hpp"
#include "metric/modify/reduce/compress.hpp"
#include "metric/modify/resample/denoise.hpp"
#include "metric/stats/structural_analysis/options.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

auto close_to(double lhs, double rhs, double tolerance = 1.0e-9) -> bool
{
	return std::abs(lhs - rhs) <= tolerance;
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
	auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 8, 9, 30}, AbsoluteDistance{});

	const auto compressed = mtrc::modify::reduce::compress(space, 3);
	assert(compressed.compression == "representatives");
	assert(compressed.source_record_count == space.size());
	assert(compressed.compressed_record_count == 3);
	assert(compressed.space.size() == 3);
	assert(compressed.validity.find("record-set cardinality reduction") != std::string::npos);

	const auto denoised =
		mtrc::modify::resample::denoise(space, mtrc::stats::structural_analysis::dbscan_options(1.5, 2));
	assert(denoised.mapping == "density_denoise");
	assert(denoised.source_record_count == space.size());
	assert(denoised.space.size() == 5);
	assert(denoised.space.record(denoised.space.id(0)) == 0);
	assert(denoised.space.record(denoised.space.id(4)) == 9);
	assert(denoised.validity.find("uneven-sampling correction") != std::string::npos);

	const auto expanded = mtrc::modify::expand::append_generated_records(
		space, std::vector<int>{5, 6}, {{space.id(2), space.id(3)}, {space.id(3)}});
	assert(expanded.mapping == "generated_record_expansion");
	assert(expanded.source_record_count == space.size());
	assert(expanded.space.size() == space.size() + 2);
	assert(expanded.space.record(expanded.space.id(6)) == 5);
	assert(expanded.space.record(expanded.space.id(7)) == 6);
	assert(expanded.source_records[6].size() == 2);
	assert(expanded.source_records[6][0] == space.id(2));
	assert(expanded.source_records[6][1] == space.id(3));
	assert(expanded.representative_records[7] == space.id(3));
	assert(expanded.validity.find("record-set expansion") != std::string::npos);
	assert(rejects([&] {
		(void)mtrc::modify::expand::append_generated_records(space, std::vector<int>{4}, {});
	}));
	assert(rejects([&] {
		(void)mtrc::modify::expand::append_generated_records(
			space, std::vector<int>{4}, {{mtrc::RecordId::from_index(100)}});
	}));

	auto mapped = mtrc::map(space, [](int value) { return static_cast<double>(value) / 10.0; },
							[](double lhs, double rhs) { return std::abs(lhs - rhs); });
	assert(mapped.mapping == "deterministic_transform");
	assert(mapped.source_record_count == space.size());
	assert(mapped.space.size() == space.size());
	assert(close_to(mapped.space.record(mapped.space.id(3)), 0.8));
	assert(mapped.out_of_sample_supported);

	mtrc::modify::dynamics::DiffusionOptions<double> dynamics;
	dynamics.diffusion_steps = 2;
	dynamics.kernel_scale = 1.0;
	const auto process = mtrc::modify::dynamics::diffusion_process<decltype(space), double>(space, dynamics);
	assert(process.record_count == space.size());
	assert(process.diffusion_steps == 2);
	assert(process.dense_distance_evaluations == space.size() * space.size());
	assert(process.pairwise_distances == "exact_space_distances");
	for (std::size_t row = 0; row < process.record_count; ++row) {
		double row_sum = 0.0;
		for (std::size_t column = 0; column < process.record_count; ++column) {
			assert(process.transition(row, column) >= 0.0);
			assert(std::isfinite(process.potential(row, column)));
			row_sum += process.transition(row, column);
		}
		assert(close_to(row_sum, 1.0));
	}

	const auto evolved = mtrc::modify::dynamics::diffuse_distribution(process, {1.0, 0.0, 0.0, 0.0, 0.0, 0.0});
	assert(evolved.size() == space.size());
	assert(close_to(std::accumulate(evolved.begin(), evolved.end(), 0.0), 1.0));
	assert(evolved[0] > evolved[5]);
	assert(rejects([&] { (void)mtrc::modify::dynamics::diffuse_distribution(process, {1.0}); }));
	assert(rejects([&] {
		(void)mtrc::modify::dynamics::diffuse_distribution(process, {1.0, -1.0, 0.0, 0.0, 0.0, 0.0});
	}));

	const auto coordinates = mtrc::modify::dynamics::diffusion_potential_anchor_coordinates(process, 2);
	assert(coordinates.rows() == space.size());
	assert(coordinates.columns() == 2);
	assert(std::isfinite(coordinates(0, 0)));
	assert(rejects([&] { (void)mtrc::modify::dynamics::diffusion_potential_anchor_coordinates(process, 0); }));

	auto pair_space = mtrc::make_space(std::vector<int>{0, 1}, AbsoluteDistance{});
	mtrc::modify::dynamics::DiffusionOptions<double> pair_options;
	pair_options.diffusion_steps = 1;
	pair_options.kernel_scale = 1.0;
	const auto pair_process =
		mtrc::modify::dynamics::diffusion_process<decltype(pair_space), double>(pair_space, pair_options);
	const auto pair_coordinates = mtrc::modify::dynamics::diffusion_potential_anchor_coordinates(pair_process, 2);
	assert(close_to(pair_coordinates(0, 0), -1.0));
	assert(close_to(pair_coordinates(0, 1), 1.0));
	assert(close_to(pair_coordinates(1, 0), 1.0));
	assert(close_to(pair_coordinates(1, 1), -1.0));

	mtrc::modify::compose::PipelinePlan plan(
		"modify_level2_components", {{"reduce", "representatives"},
									 {"resample", "density_denoise"},
									 {"expand", "generated_records"},
									 {"dynamics", "diffusion_process"},
									 {"map", "deterministic_transform"},
									 {"compose", "pipeline_plan"}});
	assert(plan.component_count() == 6);
	assert(plan.has_component("dynamics", "diffusion_process"));
	assert(plan.has_component("expand", "generated_records"));

	return 0;
}
