#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include <metric/core/metric_space.hpp>
#include <metric/modify/compose/pipeline.hpp>
#include <metric/modify/dynamics/diffusion.hpp>
#include <metric/modify/expand/generated.hpp>
#include <metric/modify/map/map.hpp>
#include <metric/modify/reduce/compress.hpp>
#include <metric/modify/resample/density_filter.hpp>
#include <metric/stats/structural_analysis/options.hpp>

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

} // namespace

int main()
{
	auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 8, 9, 30}, AbsoluteDistance{});

	const auto reduced = mtrc::modify::reduce::compress(space, 3);
	const auto resampled =
		mtrc::modify::resample::density_filter(space, mtrc::stats::structural_analysis::dbscan_options(1.5, 2));
	const auto expanded = mtrc::modify::expand::expand(
		space, std::vector<int>{5}, {{space.id(2), space.id(3)}});
	const auto mapped = mtrc::map(space, [](int value) { return static_cast<double>(value) / 10.0; },
								  [](double lhs, double rhs) { return std::abs(lhs - rhs); });

	mtrc::modify::dynamics::DiffusionOptions<double> dynamics;
	dynamics.diffusion_steps = 2;
	dynamics.kernel_scale = 1.0;
	const auto process = mtrc::modify::dynamics::diffusion_process<decltype(space), double>(space, dynamics);
	const auto evolved = mtrc::modify::dynamics::diffuse_distribution(
		process, std::vector<double>{1.0, 0.0, 0.0, 0.0, 0.0, 0.0});

	mtrc::modify::compose::PipelinePlan plan(
		"modify_space_components", {{"reduce", "representatives"},
									{"resample", "density_filter"},
									{"expand", "generated_records"},
									{"dynamics", "diffusion_process"},
									{"map", "deterministic_transform"}});

	assert(reduced.space.size() == 3);
	assert(resampled.space.size() == 5);
	assert(expanded.space.size() == 7);
	assert(mapped.space.size() == space.size());
	assert(process.record_count == space.size());
	assert(std::abs(std::accumulate(evolved.begin(), evolved.end(), 0.0) - 1.0) < 1.0e-9);
	assert(plan.has_component("dynamics", "diffusion_process"));

	std::cout << "source records = " << space.size() << "\n";
	std::cout << "reduce representatives = " << reduced.space.size() << "\n";
	std::cout << "resample kept records = " << resampled.space.size() << "\n";
	std::cout << "expand records = " << expanded.space.size() << "\n";
	std::cout << "map records = " << mapped.space.size() << "\n";
	std::cout << "dynamics diffusion steps = " << process.diffusion_steps << "\n";
	std::cout << "dynamics evolved mass = " << std::accumulate(evolved.begin(), evolved.end(), 0.0) << "\n";
	std::cout << "compose plan components = " << plan.component_count() << "\n";

	return 0;
}
