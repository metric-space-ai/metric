#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/engine.hpp>

namespace {

struct ProcessCurve {
	std::string id;
	std::string family;
	std::vector<double> values;
};

struct AlignedCurveDistance {
	double gap_cost{2.0};

	auto operator()(const ProcessCurve &lhs, const ProcessCurve &rhs) const -> double
	{
		std::vector<double> previous(rhs.values.size() + 1, 0.0);
		for (std::size_t index = 0; index < previous.size(); ++index) {
			previous[index] = static_cast<double>(index) * gap_cost;
		}

		for (std::size_t lhs_index = 1; lhs_index <= lhs.values.size(); ++lhs_index) {
			std::vector<double> current(rhs.values.size() + 1, 0.0);
			current[0] = static_cast<double>(lhs_index) * gap_cost;
			for (std::size_t rhs_index = 1; rhs_index <= rhs.values.size(); ++rhs_index) {
				const auto substitution =
					previous[rhs_index - 1] +
					std::min(std::abs(lhs.values[lhs_index - 1] - rhs.values[rhs_index - 1]), 2 * gap_cost);
				const auto deletion = previous[rhs_index] + gap_cost;
				const auto insertion = current[rhs_index - 1] + gap_cost;
				current[rhs_index] = std::min({substitution, deletion, insertion});
			}
			previous = std::move(current);
		}

		return previous.back();
	}
};

struct PointwisePaddedCurveDistance {
	auto operator()(const ProcessCurve &lhs, const ProcessCurve &rhs) const -> double
	{
		const auto size = std::max(lhs.values.size(), rhs.values.size());
		double squared_sum = 0.0;
		for (std::size_t index = 0; index < size; ++index) {
			const auto left = index < lhs.values.size() ? lhs.values[index] : 0.0;
			const auto right = index < rhs.values.size() ? rhs.values[index] : 0.0;
			const auto delta = left - right;
			squared_sum += delta * delta;
		}
		return std::sqrt(squared_sum);
	}
};

struct CurveCoordinateEncoder {
	std::size_t coordinate_count{0};

	auto operator()(const ProcessCurve &record) const -> std::vector<double>
	{
		if (coordinate_count == 0) {
			throw std::invalid_argument("curve coordinate encoder requires a positive coordinate count");
		}
		if (record.values.empty()) {
			throw std::invalid_argument("curve coordinate encoder requires non-empty curve values");
		}
		if (coordinate_count == 1 || record.values.size() == 1) {
			return std::vector<double>(coordinate_count, record.values.front());
		}

		std::vector<double> coordinates;
		coordinates.reserve(coordinate_count);
		for (std::size_t index = 0; index < coordinate_count; ++index) {
			const auto position = static_cast<double>(index) * static_cast<double>(record.values.size() - 1) /
								  static_cast<double>(coordinate_count - 1);
			const auto left = static_cast<std::size_t>(std::floor(position));
			const auto right = std::min(left + 1, record.values.size() - 1);
			const auto fraction = position - static_cast<double>(left);
			coordinates.push_back(record.values[left] * (1.0 - fraction) + record.values[right] * fraction);
		}
		return coordinates;
	}
};

auto gallery_records() -> std::vector<ProcessCurve>
{
	return {{"normal_reference", "normal", {0, 0, 1, 1, 1, 2, 3, 3}},
			{"flat_hold", "flat", {0, 0, 0, 0, 1, 1, 1, 1, 1, 1}},
			{"late_ramp", "late_ramp", {0, 0, 0, 1, 1, 1, 2, 3, 3}},
			{"spike", "spike", {0, 0, 1, 7, 1, 2, 3, 3}},
			{"early_ramp", "early_ramp", {0, 1, 1, 1, 2, 3, 3, 3}},
			{"normal_smooth", "normal", {0, 0, 1, 1, 2, 2, 3, 3}},
			{"flat_hold_long", "flat", {0, 0, 0, 0, 0, 1, 1, 1, 1, 1}},
			{"late_ramp_shifted", "late_ramp", {0, 0, 0, 0, 1, 1, 2, 3, 3}},
			{"spike_shifted", "spike", {0, 0, 0, 7, 1, 2, 3, 3}},
			{"early_ramp_short", "early_ramp", {0, 1, 1, 2, 2, 3, 3}},
			{"late_ramp_long_delay", "late_ramp", {0, 0, 0, 0, 0, 1, 2, 3, 3}},
			{"flat_hold_extra_long", "flat", {0, 0, 0, 0, 0, 0, 1, 1, 1, 1}},
			{"normal_soft_start", "normal", {0, 0, 1, 1, 1, 1, 2, 3, 3}},
			{"spike_wide", "spike", {0, 0, 0, 1, 8, 1, 2, 3, 3}},
			{"early_ramp_plateau", "early_ramp", {0, 1, 1, 1, 1, 2, 3, 3}}};
}

auto held_out_queries() -> std::vector<ProcessCurve>
{
	return {{"gallery_delay_A", "late_ramp", {0, 0, 0, 0, 1, 1, 1, 2, 3, 3}},
			{"gallery_delay_B", "late_ramp", {0, 0, 0, 0, 0, 1, 1, 2, 3, 3}},
			{"gallery_delay_C", "late_ramp", {0, 0, 0, 0, 1, 1, 2, 3}},
			{"gallery_delay_D", "late_ramp", {0, 0, 0, 0, 0, 1, 2, 3}},
			{"gallery_delay_E", "late_ramp", {0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3}},
			{"gallery_delay_F", "late_ramp", {0, 0, 0, 0, 1, 1, 1, 1, 2, 3, 3}}};
}

} // namespace

int main()
{
	const auto source_records = gallery_records();
	const auto queries = held_out_queries();
	auto space = mtrc::make_space(source_records, AlignedCurveDistance{});
	auto vector_baseline = mtrc::make_space(source_records, PointwisePaddedCurveDistance{});

	mtrc::modify::map::DiffusionCoordinateSpec<double> geometry;
	geometry.dimensions = 2;
	geometry.diffusion_steps = 2;
	geometry.kernel_scale = 1.0;

	constexpr std::size_t coordinate_count = 8;
	auto codec = mtrc::solve::parametric::make_coordinate_record_codec<ProcessCurve, double>(
		coordinate_count, CurveCoordinateEncoder{coordinate_count}, "process_curve_gallery_coordinate_codec");
	assert(codec.coordinate_count() == coordinate_count);
	assert(!codec.inverse_supported());

	mtrc::modify::map::CoordinateCalibrationSpec<double> calibration;
	calibration.steps = 220;
	calibration.batch_size = source_records.size();
	calibration.shuffle = false;
	calibration.seed = 41;
	calibration.gradient_clip_norm = 20.0;

	const auto pipeline_plan = mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_builder(0.02, 1.0)
								   .use_record_coordinate_codec()
								   .use_distance_table_pairwise_distances()
								   .use_exponential_affinity_kernel()
								   .use_lazy_row_normalized_diffusion_operator()
								   .plan();
	const auto hooks = mtrc::modify::compose::resolve_parametric_diffusion_coordinate_executable_hooks<double>(pipeline_plan);
	assert(hooks.executable());
	assert(hooks.codec == "record_coordinate_codec");

	const auto targets = mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
		space, geometry, hooks.pairwise_distances, hooks.affinity_kernel, hooks.diffusion_operator);
	assert(targets.record_count == source_records.size());
	assert(targets.dense_distance_evaluations == source_records.size() * source_records.size());

	mtrc::solve::parametric::LinearCoordinateSolverSpec<double> solver_spec;
	solver_spec.input_dimensions = coordinate_count;
	solver_spec.coordinate_dimensions = geometry.dimensions;
	solver_spec.initial_weight_scale = 0.01;
	solver_spec.learning_rate = 0.003;

	auto mapping = mtrc::modify::map::parametric_diffusion_coordinates_with_codec(
		mtrc::solve::parametric::make_linear_coordinate_solver(solver_spec), geometry, calibration, codec,
		pipeline_plan);
	auto mapping_artifact = mtrc::modify::map::derive_from(mapping, space);
	assert(mapping_artifact.pipeline_plan().has_component("codec", "record_coordinate_codec"));
	assert(mapping_artifact.codec().to_json().at("name") == "process_curve_gallery_coordinate_codec");

	const auto latent = mtrc::modify::map::transform(mapping_artifact, space);
	assert(latent.mapping == "parametric_diffusion_coordinates");
	assert(latent.strategy == "native_metric_diffusion_coordinate_solver");
	assert(!latent.inverse_supported);
	assert(latent.space.size() == source_records.size());
	assert(latent.space.record(latent.space.id(0)).size() == geometry.dimensions);

	auto query_space = mtrc::make_space(queries, AlignedCurveDistance{});
	const auto query_latent = mtrc::modify::map::transform(mapping_artifact, query_space);
	assert(query_latent.space.size() == queries.size());

	std::size_t metric_correct = 0;
	std::size_t vector_misses = 0;
	std::size_t latent_correct = 0;
	std::string first_metric_family;
	std::string first_vector_family;
	std::string first_latent_family;
	double metric_margin_sum = 0.0;
	std::vector<std::string> query_diagnostics;
	for (std::size_t index = 0; index < queries.size(); ++index) {
		const auto metric_neighbors = mtrc::find_neighbors(space, queries[index], mtrc::count{1});
		const auto vector_neighbors = mtrc::find_neighbors(vector_baseline, queries[index], mtrc::count{1});
		const auto latent_neighbors = mtrc::find_neighbors(
			latent.space, query_latent.space.record(query_latent.space.id(index)), mtrc::count{1});
		assert(metric_neighbors.size() == 1);
		assert(vector_neighbors.size() == 1);
		assert(latent_neighbors.size() == 1);

		const auto metric_family = source_records[metric_neighbors[0].id.index()].family;
		const auto vector_family = source_records[vector_neighbors[0].id.index()].family;
		const auto latent_family = source_records[latent_neighbors[0].id.index()].family;
		if (index == 0) {
			first_metric_family = metric_family;
			first_vector_family = vector_family;
			first_latent_family = latent_family;
		}
		if (metric_family == queries[index].family) {
			++metric_correct;
		}
		if (vector_family != queries[index].family) {
			++vector_misses;
		}
		if (latent_family == queries[index].family) {
			++latent_correct;
		}
		const auto vector_distance_under_metric =
			space.metric()(queries[index], source_records[vector_neighbors[0].id.index()]);
		const auto metric_margin = vector_distance_under_metric - metric_neighbors[0].distance;
		assert(metric_margin > 0.0);
		metric_margin_sum += metric_margin;
		query_diagnostics.push_back(queries[index].id + ":metric=" + metric_family + ",vector=" + vector_family +
									",latent=" + latent_family);
	}
	assert(metric_correct == queries.size());
	assert(vector_misses == queries.size());
	assert(latent_correct == queries.size());

	const auto preservation = mtrc::modify::map::neighbor_preservation(space, latent, 1);
	assert(preservation.source_record_count == source_records.size());
	assert(preservation.mapped_record_count == latent.space.size());
	assert(preservation.evaluated_neighbor_count == 1);

	const auto oos_stability = mtrc::modify::map::out_of_sample_neighbor_stability(mapping_artifact, space, query_space, 1);
	assert(oos_stability.transform_supported);
	assert(oos_stability.source_record_count == source_records.size());
	assert(oos_stability.query_record_count == queries.size());
	assert(oos_stability.evaluated_neighbor_count == 1);
	assert(oos_stability.average_mapped_best_source_rank >= 1.0);
	assert(oos_stability.maximum_mapped_best_source_rank >= 1);
	assert(oos_stability.average_best_distance_penalty >= 0.0);
	assert(oos_stability.maximum_best_distance_penalty >= 0.0);
	assert(std::abs(oos_stability.average_mapped_best_source_rank - (4.0 / 3.0)) < 1.0e-12);
	assert(oos_stability.maximum_mapped_best_source_rank == 3);
	assert(std::abs(oos_stability.average_best_distance_penalty - (1.0 / 6.0)) < 1.0e-12);
	assert(std::abs(oos_stability.maximum_best_distance_penalty - 1.0) < 1.0e-12);

	const auto &report = mapping_artifact.calibration_report();
	const auto calibrated_steps = mtrc::solve::parametric::coordinate_calibration_step_count(report);
	assert(calibrated_steps == calibration.steps);
	const auto initial_coordinate_error = mtrc::solve::parametric::coordinate_calibration_target_error(
		report, mtrc::solve::parametric::CoordinateCalibrationPoint::first);
	const auto final_coordinate_error = mtrc::solve::parametric::coordinate_calibration_target_error(
		report, mtrc::solve::parametric::CoordinateCalibrationPoint::last);
	assert(std::isfinite(initial_coordinate_error));
	assert(std::isfinite(final_coordinate_error));

	std::cout << "process diffusion-coordinate gallery records = " << source_records.size() << "\n";
	std::cout << "process diffusion-coordinate gallery queries = " << queries.size() << "\n";
	std::cout << "process diffusion-coordinate gallery source metric = aligned_curve_distance\n";
	std::cout << "process diffusion-coordinate gallery record-coordinate codec = "
			  << mapping_artifact.codec().to_json().at("name").template get<std::string>() << "\n";
	std::cout << "process diffusion-coordinate gallery pipeline components = " << mapping_artifact.pipeline_plan().component_count() << "\n";
	std::cout << "process diffusion-coordinate gallery distance provider = " << hooks.pairwise_distances << "\n";
	std::cout << "process diffusion-coordinate gallery affinity kernel = " << hooks.affinity_kernel << "\n";
	std::cout << "process diffusion-coordinate gallery diffusion operator = " << hooks.diffusion_operator << "\n";
	std::cout << "process diffusion-coordinate gallery target evaluations = " << targets.dense_distance_evaluations << "\n";
	std::cout << "process diffusion-coordinate gallery metric nearest family = " << first_metric_family << "\n";
	std::cout << "process diffusion-coordinate gallery vector baseline family = " << first_vector_family << "\n";
	std::cout << "process diffusion-coordinate gallery latent nearest family = " << first_latent_family << "\n";
	std::cout << "process diffusion-coordinate gallery metric correct = " << metric_correct << "/" << queries.size() << "\n";
	std::cout << "process diffusion-coordinate gallery vector misses = " << vector_misses << "/" << queries.size() << "\n";
	std::cout << "process diffusion-coordinate gallery latent correct = " << latent_correct << "/" << queries.size() << "\n";
	std::cout << "process diffusion-coordinate gallery average metric margin = "
			  << metric_margin_sum / static_cast<double>(queries.size()) << "\n";
	for (const auto &diagnostic : query_diagnostics) {
		std::cout << "process diffusion-coordinate gallery query = " << diagnostic << "\n";
	}
	std::cout << "process diffusion-coordinate gallery neighbor preservation = " << preservation.recall << "\n";
	std::cout << "process diffusion-coordinate gallery OOS anchor recall = " << oos_stability.anchor_recall << "\n";
	std::cout << "process diffusion-coordinate gallery first-anchor match rate = " << oos_stability.first_anchor_match_rate << "\n";
	std::cout << "process diffusion-coordinate gallery mapped-anchor source rank = " << oos_stability.average_mapped_best_source_rank
			  << "\n";
	std::cout << "process diffusion-coordinate gallery max mapped-anchor source rank = "
			  << oos_stability.maximum_mapped_best_source_rank << "\n";
	std::cout << "process diffusion-coordinate gallery original-metric distance penalty = "
			  << oos_stability.average_best_distance_penalty << "\n";
	std::cout << "process diffusion-coordinate gallery max original-metric distance penalty = "
			  << oos_stability.maximum_best_distance_penalty << "\n";
	std::cout << "process diffusion-coordinate gallery target error = " << initial_coordinate_error << " -> "
			  << final_coordinate_error << "\n";
	std::cout << "process diffusion-coordinate gallery inverse support = none_record_coordinate_codec\n";
	std::cout << "process diffusion-coordinate gallery deterministic seed = " << calibration.seed << "\n";

	return 0;
}
