#include <cmath>
#include <cstdio>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/metric/catalog/vector/Standards.hpp"
#include "metric/modify/compose/pipeline.hpp"
#include "metric/modify/dynamics/dynamics.hpp"
#include "metric/modify/dynamics/diffusion.hpp"
#include "metric/modify/dynamics/finite_dynamics.hpp"
#include "metric/modify/expand/generated.hpp"
#include "metric/modify/map/map.hpp"
#include "metric/modify/reduce/compress.hpp"
#include "metric/modify/resample/density_filter.hpp"
#include "metric/stats/structural_analysis/options.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct CountingAbsoluteDistance {
	std::shared_ptr<std::size_t> calls;

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

struct CountingVectorDistance {
	std::shared_ptr<std::size_t> calls;

	auto operator()(const std::vector<double> &lhs, const std::vector<double> &rhs) const -> double
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

auto close_to(double lhs, double rhs, double tolerance = 1.0e-9) -> bool
{
	return std::abs(lhs - rhs) <= tolerance;
}

#define REQUIRE(condition)                                                                                              \
	do {                                                                                                                \
		if (!(condition)) {                                                                                            \
			std::fprintf(stderr, "%s:%d: requirement failed: %s\n", __FILE__, __LINE__, #condition);                  \
			return 1;                                                                                                  \
		}                                                                                                             \
	} while (false)

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
	REQUIRE(compressed.compression == "representatives");
	REQUIRE(compressed.source_record_count == space.size());
	REQUIRE(compressed.compressed_record_count == 3);
	REQUIRE(compressed.space.size() == 3);
	REQUIRE(compressed.validity.find("record-set cardinality reduction") != std::string::npos);

	const auto filtered =
		mtrc::modify::resample::density_filter(space, mtrc::stats::structural_analysis::dbscan_options(1.5, 2));
	REQUIRE(filtered.mapping == "density_filter");
	REQUIRE(filtered.source_record_count == space.size());
	REQUIRE(filtered.space.size() == 5);
	REQUIRE(filtered.space.record(filtered.space.id(0)) == 0);
	REQUIRE(filtered.space.record(filtered.space.id(4)) == 9);
	REQUIRE(filtered.validity.find("density-based record filtering") != std::string::npos);

	const auto expanded = mtrc::modify::expand::append_generated_records(
		space, std::vector<int>{5, 6}, {{space.id(2), space.id(3)}, {space.id(3)}});
	REQUIRE(expanded.mapping == "generated_record_expansion");
	REQUIRE(expanded.source_record_count == space.size());
	REQUIRE(expanded.space.size() == space.size() + 2);
	REQUIRE(expanded.space.record(expanded.space.id(6)) == 5);
	REQUIRE(expanded.space.record(expanded.space.id(7)) == 6);
	REQUIRE(expanded.source_records[6].size() == 2);
	REQUIRE(expanded.source_records[6][0] == space.id(2));
	REQUIRE(expanded.source_records[6][1] == space.id(3));
	REQUIRE(expanded.representative_records[7] == space.id(3));
	REQUIRE(expanded.validity.find("record-set expansion") != std::string::npos);
	REQUIRE(rejects([&] {
		(void)mtrc::modify::expand::append_generated_records(space, std::vector<int>{4}, {});
	}));
	REQUIRE(rejects([&] {
		(void)mtrc::modify::expand::append_generated_records(
			space, std::vector<int>{4}, {{mtrc::RecordId::from_index(100)}});
	}));

	auto mapped = mtrc::map(space, [](int value) { return static_cast<double>(value) / 10.0; },
							[](double lhs, double rhs) { return std::abs(lhs - rhs); });
	REQUIRE(mapped.mapping == "deterministic_transform");
	REQUIRE(mapped.source_record_count == space.size());
	REQUIRE(mapped.space.size() == space.size());
	REQUIRE(close_to(mapped.space.record(mapped.space.id(3)), 0.8));
	REQUIRE(mapped.out_of_sample_supported);

	mtrc::modify::dynamics::DiffusionOptions<double> dynamics;
	dynamics.diffusion_steps = 2;
	dynamics.kernel_scale = 1.0;
	const auto process = mtrc::modify::dynamics::diffusion_process<decltype(space), double>(space, dynamics);
	REQUIRE(process.record_count == space.size());
	REQUIRE(process.diffusion_steps == 2);
	REQUIRE(process.dense_distance_evaluations == space.size() * space.size());
	REQUIRE(process.max_dense_records == mtrc::modify::dynamics::default_diffusion_max_dense_records);
	REQUIRE(process.pairwise_distances == "exact_space_distances");
	for (std::size_t row = 0; row < process.record_count; ++row) {
		double row_sum = 0.0;
		for (std::size_t column = 0; column < process.record_count; ++column) {
			REQUIRE(process.transition(row, column) >= 0.0);
			REQUIRE(std::isfinite(process.potential(row, column)));
			row_sum += process.transition(row, column);
		}
		REQUIRE(close_to(row_sum, 1.0));
	}

	const auto guarded_calls = std::make_shared<std::size_t>(0);
	auto guarded_space = mtrc::make_space(
		std::vector<int>{0, 1, 2, 3, 4, 5},
		CountingAbsoluteDistance{guarded_calls});
	auto guarded_options = dynamics;
	guarded_options.max_dense_records = guarded_space.size() - 1;
	REQUIRE(rejects([&] {
		(void)mtrc::modify::dynamics::diffusion_process<decltype(guarded_space), double>(
			guarded_space,
			guarded_options);
	}));
	REQUIRE(*guarded_calls == 0);

	{
		const auto default_guard_calls = std::make_shared<std::size_t>(0);
		std::vector<std::vector<double>> oversized_records(
			mtrc::modify::dynamics::default_diffuse_max_dense_records + 1, std::vector<double>{0.0});
		auto oversized_space = mtrc::make_space(oversized_records, CountingVectorDistance{default_guard_calls});
		REQUIRE(rejects([&] { (void)mtrc::modify::dynamics::diffuse(oversized_space, 1); }));
		REQUIRE(*default_guard_calls == 0);

		*default_guard_calls = 0;
		auto low_eval_space = mtrc::make_space(
			std::vector<std::vector<double>>{{0.0}, {1.0}}, CountingVectorDistance{default_guard_calls});
		REQUIRE(rejects([&] {
			(void)mtrc::modify::dynamics::diffuse(
				low_eval_space, 1, 0.0, false,
				mtrc::modify::dynamics::default_diffuse_max_dense_records,
				mtrc::modify::dynamics::default_diffuse_max_memory_bytes, 3);
		}));
		REQUIRE(*default_guard_calls == 0);
	}

	const auto evolved = mtrc::modify::dynamics::diffuse_distribution(process, {1.0, 0.0, 0.0, 0.0, 0.0, 0.0});
	REQUIRE(evolved.size() == space.size());
	REQUIRE(close_to(std::accumulate(evolved.begin(), evolved.end(), 0.0), 1.0));
	REQUIRE(evolved[0] > evolved[5]);
	REQUIRE(rejects([&] { (void)mtrc::modify::dynamics::diffuse_distribution(process, {1.0}); }));
	REQUIRE(rejects([&] {
		(void)mtrc::modify::dynamics::diffuse_distribution(process, {1.0, -1.0, 0.0, 0.0, 0.0, 0.0});
	}));

	const auto coordinates = mtrc::modify::dynamics::diffusion_potential_anchor_coordinates(process, 2);
	REQUIRE(coordinates.rows() == space.size());
	REQUIRE(coordinates.columns() == 2);
	REQUIRE(std::isfinite(coordinates(0, 0)));
	REQUIRE(rejects([&] { (void)mtrc::modify::dynamics::diffusion_potential_anchor_coordinates(process, 0); }));

	auto pair_space = mtrc::make_space(std::vector<int>{0, 1}, AbsoluteDistance{});
	mtrc::modify::dynamics::DiffusionOptions<double> pair_options;
	pair_options.diffusion_steps = 1;
	pair_options.kernel_scale = 1.0;
	const auto pair_process =
		mtrc::modify::dynamics::diffusion_process<decltype(pair_space), double>(pair_space, pair_options);
	const auto pair_coordinates = mtrc::modify::dynamics::diffusion_potential_anchor_coordinates(pair_process, 2);
	REQUIRE(close_to(pair_coordinates(0, 0), -1.0));
	REQUIRE(close_to(pair_coordinates(0, 1), 1.0));
	REQUIRE(close_to(pair_coordinates(1, 0), 1.0));
	REQUIRE(close_to(pair_coordinates(1, 1), -1.0));

	mtrc::modify::compose::PipelinePlan plan(
		"modify_level2_components", {{"reduce", "representatives"},
									 {"resample", "density_filter"},
									 {"expand", "generated_records"},
									 {"dynamics", "diffusion_process"},
									 {"map", "deterministic_transform"},
									 {"compose", "pipeline_plan"}});
	REQUIRE(plan.component_count() == 6);
	REQUIRE(plan.has_component("dynamics", "diffusion_process"));
	REQUIRE(plan.has_component("expand", "generated_records"));

	{
		const auto transition_guard_calls = std::make_shared<std::size_t>(0);
		std::vector<int> oversized_records(mtrc::modify::dynamics::default_metric_transition_max_dense_records + 1);
		std::iota(oversized_records.begin(), oversized_records.end(), 0);
		auto oversized_space =
			mtrc::make_space(oversized_records, CountingAbsoluteDistance{transition_guard_calls});
		const mtrc::modify::dynamics::dynamics_schedule default_schedule;
		REQUIRE(rejects([&] { (void)mtrc::metric_transition(oversized_space, default_schedule); }));
		REQUIRE(*transition_guard_calls == 0);

		*transition_guard_calls = 0;
		auto low_eval_space =
			mtrc::make_space(std::vector<int>{0, 1, 2}, CountingAbsoluteDistance{transition_guard_calls});
		auto low_eval_schedule = default_schedule;
		low_eval_schedule.max_distance_evaluations = 2;
		REQUIRE(rejects([&] { (void)mtrc::metric_transition(low_eval_space, low_eval_schedule); }));
		REQUIRE(*transition_guard_calls == 0);
	}

	// modify::dynamics finite_dynamics size guard: the explicit-transition
	// overloads of metric_diffuse/metric_reconstruct evolve a node signal sized by
	// the space but iterate transition.node_count. A transition built from a
	// differently-sized space must be rejected, not read/written out of bounds.
	{
		const std::vector<std::vector<double>> big_records{{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};
		const std::vector<std::vector<double>> small_records{{0.0, 0.0}, {2.0, 2.0}, {4.0, 0.0}};
		auto big = mtrc::make_space(big_records, mtrc::Euclidean<double>{});
		auto small = mtrc::make_space(small_records, mtrc::Euclidean<double>{});

		mtrc::modify::dynamics::dynamics_schedule schedule;
		schedule.neighbors = 2;
		schedule.steps = 1;
		const auto big_transition = mtrc::metric_transition(big, schedule); // node_count == 4
		REQUIRE(big_transition.size() == big.size());

		REQUIRE(rejects([&] { (void)mtrc::metric_diffuse(small, schedule, big_transition); }));
		REQUIRE(rejects([&] { (void)mtrc::metric_reconstruct(small, schedule, big_transition); }));

		// A matching-size transition still runs and produces steps + 1 frames.
		const auto forward = mtrc::metric_diffuse(big, schedule, big_transition);
		REQUIRE(forward.size() == big.size());
		REQUIRE(forward.frames.size() == schedule.steps + 1);
	}

	return 0;
}
