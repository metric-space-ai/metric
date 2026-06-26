// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_PARAMETRIC_DIFFUSION_COORDINATES_HPP
#define _METRIC_MAPPINGS_PARAMETRIC_DIFFUSION_COORDINATES_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/metadata.hpp>

#include <metric/core/concepts.hpp>
#include <metric/record/id.hpp>
#include <metric/modify/compose/parametric_diffusion_coordinates_plan.hpp>
#include <metric/modify/dynamics/diffusion.hpp>
#include <metric/space/storage/distance_matrix.hpp>
#include <metric/solve/parametric/dnn/coordinate_solver.hpp>
#include "parametric_coordinate_artifact.hpp"
#include <metric/numeric/Math.h>

namespace mtrc::modify::map {

inline constexpr std::size_t default_diffusion_coordinate_max_dense_records = 4096;
inline constexpr std::size_t default_diffusion_coordinate_max_memory_bytes =
	::mtrc::modify::dynamics::default_diffusion_max_memory_bytes;
inline constexpr std::size_t default_diffusion_coordinate_max_distance_evaluations =
	::mtrc::modify::dynamics::default_diffusion_max_distance_evaluations;

template <typename Scalar> struct DiffusionCoordinateSpec {
	std::size_t dimensions{1};
	std::size_t diffusion_steps{1};
	std::size_t max_dense_records{default_diffusion_coordinate_max_dense_records};
	std::size_t max_memory_bytes{default_diffusion_coordinate_max_memory_bytes};
	std::size_t max_distance_evaluations{default_diffusion_coordinate_max_distance_evaluations};
	Scalar kernel_scale{0};
	Scalar epsilon{Scalar(1.0e-12)};
};

template <typename Scalar> struct CoordinateCalibrationSpec {
	std::size_t steps{100};
	std::size_t batch_size{32};
	bool shuffle{true};
	std::uint64_t seed{23};
	Scalar gradient_clip_norm{0};
};

template <typename Scalar> struct DiffusionCoordinateTargets {
	using target_table_type = solve::parametric::CoordinateTargetTable<Scalar>;

	target_table_type coordinates;
	std::size_t dimensions{0};
	std::size_t diffusion_steps{0};
	std::size_t record_count{0};
	std::size_t dense_distance_evaluations{0};
	std::size_t max_dense_records{0};
	std::size_t max_memory_bytes{0};
	std::size_t max_distance_evaluations{0};
	Scalar kernel_scale{0};
	std::string method{"diffusion_potential_anchor_coordinates"};
	std::string pairwise_distances{"exact_space_distances"};
	std::string affinity_kernel{"gaussian_affinity_kernel"};
	std::string diffusion_operator{"row_normalized_diffusion_operator"};
};

template <typename Scalar> auto diffusion_coordinate_spec_to_json(const DiffusionCoordinateSpec<Scalar> &spec) -> mtrc::core::Metadata
{
		return {{"dimensions", spec.dimensions},
				{"diffusion_steps", spec.diffusion_steps},
				{"max_dense_records", spec.max_dense_records},
				{"max_memory_bytes", spec.max_memory_bytes},
				{"max_distance_evaluations", spec.max_distance_evaluations},
				{"kernel_scale", spec.kernel_scale},
				{"epsilon", spec.epsilon}};
}

template <typename Scalar>
auto diffusion_coordinate_targets_to_json(const DiffusionCoordinateTargets<Scalar> &targets) -> mtrc::core::Metadata
{
	return {{"dimensions", targets.dimensions},
			{"diffusion_steps", targets.diffusion_steps},
				{"record_count", targets.record_count},
				{"dense_distance_evaluations", targets.dense_distance_evaluations},
				{"max_dense_records", targets.max_dense_records},
				{"max_memory_bytes", targets.max_memory_bytes},
				{"max_distance_evaluations", targets.max_distance_evaluations},
				{"kernel_scale", targets.kernel_scale},
			{"method", targets.method},
			{"pairwise_distances", targets.pairwise_distances},
			{"affinity_kernel", targets.affinity_kernel},
			{"diffusion_operator", targets.diffusion_operator},
			{"target_count", targets.coordinates.size()}};
}

inline auto parametric_diffusion_coordinate_pipeline_components_to_json() -> mtrc::core::Metadata
{
	const auto plan = modify::compose::parametric_diffusion_coordinate_pipeline_plan(1.0, 1.0);
	return modify::compose::pipeline_components_to_json(plan.components());
}

namespace detail {

inline auto is_supported_diffusion_coordinate_pairwise_distances(const std::string &name) -> bool
{
	return modify::dynamics::is_supported_pairwise_distances(name);
}

inline auto is_supported_diffusion_coordinate_affinity_kernel(const std::string &name) -> bool
{
	return modify::dynamics::is_supported_affinity_kernel(name);
}

inline auto is_supported_diffusion_coordinate_diffusion_operator(const std::string &name) -> bool
{
	return modify::dynamics::is_supported_diffusion_operator(name);
}

template <typename Scalar>
auto parametric_diffusion_coordinate_pipeline_plan_for_execution(
	Scalar reconstruction_weight, Scalar geometry_weight, const std::string &pairwise_distances,
	const std::string &affinity_kernel = "gaussian_affinity_kernel",
	const std::string &diffusion_operator = "row_normalized_diffusion_operator",
	const std::string &codec = "vector_record_codec") -> modify::compose::PipelinePlan
{
	if (codec != "vector_record_codec" && codec != "record_coordinate_codec") {
		throw std::invalid_argument("unsupported parametric diffusion coordinate codec");
	}
	if (!is_supported_diffusion_coordinate_pairwise_distances(pairwise_distances)) {
		throw std::invalid_argument("unsupported parametric diffusion coordinate distance provider");
	}
	if (!is_supported_diffusion_coordinate_affinity_kernel(affinity_kernel)) {
		throw std::invalid_argument("unsupported parametric diffusion coordinate affinity kernel");
	}
	if (!is_supported_diffusion_coordinate_diffusion_operator(diffusion_operator)) {
		throw std::invalid_argument("unsupported parametric diffusion coordinate diffusion operator");
	}
	auto builder = modify::compose::parametric_diffusion_coordinate_pipeline_builder(reconstruction_weight, geometry_weight);
	if (codec == "record_coordinate_codec") {
		builder.use_record_coordinate_codec();
	}
	if (pairwise_distances == "distance_table_pairwise_distances") {
		builder.use_distance_table_pairwise_distances();
	}
	if (affinity_kernel == "exponential_affinity_kernel") {
		builder.use_exponential_affinity_kernel();
	}
	if (diffusion_operator == "lazy_row_normalized_diffusion_operator") {
		builder.use_lazy_row_normalized_diffusion_operator();
	}
	return builder.plan();
}

} // namespace detail

template <typename Space, typename Scalar = double, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto diffusion_coordinate_targets(const Space &space, DiffusionCoordinateSpec<Scalar> spec = {},
							std::string pairwise_distances = "exact_space_distances",
							std::string affinity_kernel = "gaussian_affinity_kernel",
							std::string diffusion_operator = "row_normalized_diffusion_operator")
	-> DiffusionCoordinateTargets<Scalar>
{
	if (space.empty()) {
		throw std::invalid_argument("diffusion coordinate targets require a non-empty space");
	}
	if (!detail::is_supported_diffusion_coordinate_pairwise_distances(pairwise_distances)) {
		throw std::invalid_argument("unsupported diffusion coordinate distance provider");
	}
	if (!detail::is_supported_diffusion_coordinate_affinity_kernel(affinity_kernel)) {
		throw std::invalid_argument("unsupported diffusion coordinate affinity kernel");
	}
	if (!detail::is_supported_diffusion_coordinate_diffusion_operator(diffusion_operator)) {
		throw std::invalid_argument("unsupported diffusion coordinate diffusion operator");
	}
	if (spec.dimensions == 0) {
		throw std::invalid_argument("diffusion coordinate target dimensions must be positive");
	}
	if (spec.diffusion_steps == 0) {
		throw std::invalid_argument("diffusion coordinate steps must be positive");
	}
	if (spec.epsilon <= Scalar(0)) {
		throw std::invalid_argument("diffusion coordinate epsilon must be positive");
	}
	if (spec.max_dense_records > 0 && space.size() > spec.max_dense_records) {
		throw std::invalid_argument("diffusion coordinate dense construction exceeds max_dense_records: records=" +
									std::to_string(space.size()) + " max_dense_records=" +
									std::to_string(spec.max_dense_records));
	}

	modify::dynamics::DiffusionOptions<Scalar> diffusion_options;
		diffusion_options.diffusion_steps = spec.diffusion_steps;
		diffusion_options.max_dense_records = spec.max_dense_records;
		diffusion_options.max_memory_bytes = spec.max_memory_bytes;
		diffusion_options.max_distance_evaluations = spec.max_distance_evaluations;
		diffusion_options.kernel_scale = spec.kernel_scale;
	diffusion_options.epsilon = spec.epsilon;
	diffusion_options.pairwise_distances = std::move(pairwise_distances);
	diffusion_options.affinity_kernel = std::move(affinity_kernel);
	diffusion_options.diffusion_operator = std::move(diffusion_operator);

	const auto process = modify::dynamics::diffusion_process<Space, Scalar>(space, std::move(diffusion_options));
	auto coordinate_rows = modify::dynamics::diffusion_potential_anchor_coordinates(process, spec.dimensions);

	DiffusionCoordinateTargets<Scalar> targets;
	targets.dimensions = spec.dimensions;
	targets.diffusion_steps = process.diffusion_steps;
	targets.record_count = process.record_count;
		targets.dense_distance_evaluations = process.dense_distance_evaluations;
		targets.max_dense_records = process.max_dense_records;
		targets.max_memory_bytes = process.max_memory_bytes;
		targets.max_distance_evaluations = process.max_distance_evaluations;
		targets.kernel_scale = process.kernel_scale;
	targets.pairwise_distances = process.pairwise_distances;
	targets.affinity_kernel = process.affinity_kernel;
	targets.diffusion_operator = process.diffusion_operator;
	const auto coordinate_vectors = numeric::matrix_to_row_vectors(coordinate_rows);
	for (std::size_t row = 0; row < process.record_count; ++row) {
		targets.coordinates.emplace(space.id(row), coordinate_vectors[row]);
	}
	return targets;
}

template <typename Scalar>
auto parametric_diffusion_coordinate_objective(const solve::parametric::CoordinateSolver<Scalar> &model,
										const DiffusionCoordinateTargets<Scalar> &targets,
										Scalar reconstruction_weight = Scalar(1), Scalar geometry_weight = Scalar(1))
	-> solve::parametric::CoordinateObjective<Scalar>
{
	return solve::parametric::coordinate_solver_objective(model, targets.coordinates, targets.dimensions,
														  reconstruction_weight, geometry_weight);
}

template <typename Scalar> class ParametricDiffusionCoordinateMap {
  public:
	using scalar_type = Scalar;

	ParametricDiffusionCoordinateMap(solve::parametric::CoordinateSolver<scalar_type> prototype,
								  DiffusionCoordinateSpec<scalar_type> geometry_spec,
								  CoordinateCalibrationSpec<scalar_type> calibration_spec = {},
								  scalar_type reconstruction_weight = scalar_type(1),
								  scalar_type geometry_weight = scalar_type(1),
								  std::string pairwise_distances = "exact_space_distances",
								  std::string affinity_kernel = "gaussian_affinity_kernel",
								  std::string diffusion_operator = "row_normalized_diffusion_operator")
		: prototype_(std::move(prototype)), geometry_spec_(std::move(geometry_spec)),
		  calibration_spec_(std::move(calibration_spec)), reconstruction_weight_(reconstruction_weight),
		  geometry_weight_(geometry_weight), pairwise_distances_(std::move(pairwise_distances)),
		  affinity_kernel_(std::move(affinity_kernel)), diffusion_operator_(std::move(diffusion_operator)),
		  pipeline_plan_(detail::parametric_diffusion_coordinate_pipeline_plan_for_execution(
			  reconstruction_weight_, geometry_weight_, pairwise_distances_, affinity_kernel_, diffusion_operator_))
	{
		validate_parameters();
	}

	ParametricDiffusionCoordinateMap(solve::parametric::CoordinateSolver<scalar_type> prototype,
								  DiffusionCoordinateSpec<scalar_type> geometry_spec,
								  CoordinateCalibrationSpec<scalar_type> calibration_spec, modify::compose::PipelinePlan pipeline_plan)
		: prototype_(std::move(prototype)), geometry_spec_(std::move(geometry_spec)),
		  calibration_spec_(std::move(calibration_spec)), pipeline_plan_(std::move(pipeline_plan))
	{
		const auto hooks = modify::compose::resolve_parametric_diffusion_coordinate_executable_hooks<scalar_type>(pipeline_plan_);
		hooks.validate();
		reconstruction_weight_ = hooks.reconstruction_weight;
		geometry_weight_ = hooks.geometry_weight;
		pairwise_distances_ = hooks.pairwise_distances;
		affinity_kernel_ = hooks.affinity_kernel;
		diffusion_operator_ = hooks.diffusion_operator;
		if (hooks.codec != "vector_record_codec") {
			throw std::invalid_argument("parametric diffusion coordinate map requires vector_record_codec");
		}
		validate_parameters();
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto derive_from(const Space &space) const -> ParametricCoordinateMapArtifact<typename Space::record_type, scalar_type>
	{
		if (space.empty()) {
			throw std::invalid_argument("cannot derive parametric diffusion coordinate map from an empty space");
		}

		using record_type = typename Space::record_type;
		using model_type = ParametricCoordinateMapArtifact<record_type, scalar_type>;
		using codec_type = typename model_type::codec_type;

		codec_type codec(space.records().front().size());
		auto coordinates = codec.encode_batch(space.records());
		auto ids = mtrc::record_ids(space);

		auto model = prototype_.clone();
		const auto targets = diffusion_coordinate_targets<Space, scalar_type>(space, geometry_spec_, pairwise_distances_,
																		affinity_kernel_, diffusion_operator_);
		if (targets.dimensions != model.latent_dimension()) {
			throw std::invalid_argument("diffusion coordinate target dimensions must match the coordinate dimension");
		}

		auto objective = parametric_diffusion_coordinate_objective(model, targets, reconstruction_weight_, geometry_weight_);

		solve::parametric::CoordinateDataset<scalar_type> dataset(std::move(ids), std::move(coordinates), space.version());
		solve::parametric::calibrate_coordinate_solver(
			model, dataset, objective,
			solve::parametric::coordinate_calibration_config(calibration_spec_.steps, calibration_spec_.batch_size,
															 calibration_spec_.shuffle, calibration_spec_.seed,
															 calibration_spec_.gradient_clip_norm));
		return model_type(std::move(model), std::move(codec), space.size(), "parametric_diffusion_coordinates",
						  "native_metric_diffusion_coordinate_solver", pipeline_plan_);
	}

  private:
	auto validate_parameters() const -> void
	{
		if (reconstruction_weight_ < scalar_type(0) || geometry_weight_ < scalar_type(0)) {
			throw std::invalid_argument("parametric diffusion coordinate loss weights must be non-negative");
		}
		if (reconstruction_weight_ == scalar_type(0) && geometry_weight_ == scalar_type(0)) {
			throw std::invalid_argument("parametric diffusion coordinate map requires at least one positive loss weight");
		}
		if (!detail::is_supported_diffusion_coordinate_pairwise_distances(pairwise_distances_)) {
			throw std::invalid_argument("unsupported parametric diffusion coordinate distance provider");
		}
		if (!detail::is_supported_diffusion_coordinate_affinity_kernel(affinity_kernel_)) {
			throw std::invalid_argument("unsupported parametric diffusion coordinate affinity kernel");
		}
		if (!detail::is_supported_diffusion_coordinate_diffusion_operator(diffusion_operator_)) {
			throw std::invalid_argument("unsupported parametric diffusion coordinate diffusion operator");
		}
	}

	solve::parametric::CoordinateSolver<scalar_type> prototype_;
	DiffusionCoordinateSpec<scalar_type> geometry_spec_;
	CoordinateCalibrationSpec<scalar_type> calibration_spec_;
	scalar_type reconstruction_weight_{1};
	scalar_type geometry_weight_{1};
	std::string pairwise_distances_{"exact_space_distances"};
	std::string affinity_kernel_{"gaussian_affinity_kernel"};
	std::string diffusion_operator_{"row_normalized_diffusion_operator"};
	modify::compose::PipelinePlan pipeline_plan_;
};

template <typename Scalar, typename Codec> class EncodedParametricDiffusionCoordinateMap {
  public:
	using scalar_type = Scalar;
	using codec_type = Codec;

	EncodedParametricDiffusionCoordinateMap(solve::parametric::CoordinateSolver<scalar_type> prototype,
										 DiffusionCoordinateSpec<scalar_type> geometry_spec, codec_type codec,
										 CoordinateCalibrationSpec<scalar_type> calibration_spec = {},
										 scalar_type reconstruction_weight = scalar_type(1),
										 scalar_type geometry_weight = scalar_type(1),
										 std::string pairwise_distances = "exact_space_distances",
										 std::string affinity_kernel = "gaussian_affinity_kernel",
										 std::string diffusion_operator = "row_normalized_diffusion_operator")
		: prototype_(std::move(prototype)), geometry_spec_(std::move(geometry_spec)),
		  calibration_spec_(std::move(calibration_spec)), codec_(std::move(codec)),
		  reconstruction_weight_(reconstruction_weight), geometry_weight_(geometry_weight),
		  pairwise_distances_(std::move(pairwise_distances)), affinity_kernel_(std::move(affinity_kernel)),
		  diffusion_operator_(std::move(diffusion_operator)),
		  pipeline_plan_(detail::parametric_diffusion_coordinate_pipeline_plan_for_execution(reconstruction_weight_, geometry_weight_,
																		  pairwise_distances_, affinity_kernel_,
																		  diffusion_operator_, "record_coordinate_codec"))
	{
		validate_parameters();
	}

	EncodedParametricDiffusionCoordinateMap(solve::parametric::CoordinateSolver<scalar_type> prototype,
										 DiffusionCoordinateSpec<scalar_type> geometry_spec,
										 CoordinateCalibrationSpec<scalar_type> calibration_spec, codec_type codec,
										 modify::compose::PipelinePlan pipeline_plan)
		: prototype_(std::move(prototype)), geometry_spec_(std::move(geometry_spec)),
		  calibration_spec_(std::move(calibration_spec)), codec_(std::move(codec)), pipeline_plan_(std::move(pipeline_plan))
	{
		const auto hooks = modify::compose::resolve_parametric_diffusion_coordinate_executable_hooks<scalar_type>(pipeline_plan_);
		hooks.validate();
		if (hooks.codec != "record_coordinate_codec") {
			throw std::invalid_argument("encoded parametric diffusion coordinate map requires record_coordinate_codec");
		}
		reconstruction_weight_ = hooks.reconstruction_weight;
		geometry_weight_ = hooks.geometry_weight;
		pairwise_distances_ = hooks.pairwise_distances;
		affinity_kernel_ = hooks.affinity_kernel;
		diffusion_operator_ = hooks.diffusion_operator;
		validate_parameters();
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto derive_from(const Space &space) const -> ParametricCoordinateMapArtifact<typename Space::record_type, scalar_type, codec_type>
	{
		if (space.empty()) {
			throw std::invalid_argument("cannot derive encoded parametric diffusion coordinate map from an empty space");
		}

		using record_type = typename Space::record_type;
		using model_type = ParametricCoordinateMapArtifact<record_type, scalar_type, codec_type>;

		auto coordinates = codec_.encode_batch(space.records());
		if (coordinates.rows() != space.size()) {
			throw std::invalid_argument("record coordinate codec row count does not match space");
		}
		if (coordinates.columns() != solve::parametric::coordinate_solver_input_dimension(prototype_)) {
			throw std::invalid_argument("record coordinate codec dimension does not match solver input dimension");
		}

		auto ids = mtrc::record_ids(space);

		auto model = prototype_.clone();
		const auto targets = diffusion_coordinate_targets<Space, scalar_type>(space, geometry_spec_, pairwise_distances_,
																		affinity_kernel_, diffusion_operator_);
		if (targets.dimensions != model.latent_dimension()) {
			throw std::invalid_argument("diffusion coordinate target dimensions must match the coordinate dimension");
		}

		auto objective = parametric_diffusion_coordinate_objective(model, targets, reconstruction_weight_, geometry_weight_);

		solve::parametric::CoordinateDataset<scalar_type> dataset(std::move(ids), std::move(coordinates), space.version());
		solve::parametric::calibrate_coordinate_solver(
			model, dataset, objective,
			solve::parametric::coordinate_calibration_config(calibration_spec_.steps, calibration_spec_.batch_size,
															 calibration_spec_.shuffle, calibration_spec_.seed,
															 calibration_spec_.gradient_clip_norm));
		return model_type(std::move(model), codec_, space.size(), "parametric_diffusion_coordinates", "native_metric_diffusion_coordinate_solver",
						  pipeline_plan_);
	}

  private:
	auto validate_parameters() const -> void
	{
		if (reconstruction_weight_ < scalar_type(0) || geometry_weight_ < scalar_type(0)) {
			throw std::invalid_argument("encoded parametric diffusion coordinate loss weights must be non-negative");
		}
		if (reconstruction_weight_ == scalar_type(0) && geometry_weight_ == scalar_type(0)) {
			throw std::invalid_argument(
				"encoded parametric diffusion coordinate map requires at least one positive loss weight");
		}
		if (!detail::is_supported_diffusion_coordinate_pairwise_distances(pairwise_distances_)) {
			throw std::invalid_argument("unsupported encoded parametric diffusion coordinate distance provider");
		}
		if (!detail::is_supported_diffusion_coordinate_affinity_kernel(affinity_kernel_)) {
			throw std::invalid_argument("unsupported encoded parametric diffusion coordinate affinity kernel");
		}
		if (!detail::is_supported_diffusion_coordinate_diffusion_operator(diffusion_operator_)) {
			throw std::invalid_argument("unsupported encoded parametric diffusion coordinate diffusion operator");
		}
	}

	solve::parametric::CoordinateSolver<scalar_type> prototype_;
	DiffusionCoordinateSpec<scalar_type> geometry_spec_;
	CoordinateCalibrationSpec<scalar_type> calibration_spec_;
	codec_type codec_;
	scalar_type reconstruction_weight_{1};
	scalar_type geometry_weight_{1};
	std::string pairwise_distances_{"exact_space_distances"};
	std::string affinity_kernel_{"gaussian_affinity_kernel"};
	std::string diffusion_operator_{"row_normalized_diffusion_operator"};
	modify::compose::PipelinePlan pipeline_plan_;
};

template <typename Scalar>
auto parametric_diffusion_coordinates(solve::parametric::CoordinateSolver<Scalar> model, DiffusionCoordinateSpec<Scalar> geometry_spec,
							  CoordinateCalibrationSpec<Scalar> calibration_spec = {}, Scalar reconstruction_weight = Scalar(1),
							  Scalar geometry_weight = Scalar(1),
							  std::string pairwise_distances = "exact_space_distances",
							  std::string affinity_kernel = "gaussian_affinity_kernel",
							  std::string diffusion_operator = "row_normalized_diffusion_operator")
	-> ParametricDiffusionCoordinateMap<Scalar>
{
	return ParametricDiffusionCoordinateMap<Scalar>(std::move(model), std::move(geometry_spec), std::move(calibration_spec),
												 reconstruction_weight, geometry_weight, std::move(pairwise_distances),
												 std::move(affinity_kernel), std::move(diffusion_operator));
}

template <typename Scalar>
auto parametric_diffusion_coordinates(solve::parametric::CoordinateSolver<Scalar> model, DiffusionCoordinateSpec<Scalar> geometry_spec,
							  CoordinateCalibrationSpec<Scalar> calibration_spec, modify::compose::PipelinePlan pipeline_plan)
	-> ParametricDiffusionCoordinateMap<Scalar>
{
	return ParametricDiffusionCoordinateMap<Scalar>(std::move(model), std::move(geometry_spec), std::move(calibration_spec),
												 std::move(pipeline_plan));
}

template <typename Scalar, typename Codec>
auto parametric_diffusion_coordinates_with_codec(solve::parametric::CoordinateSolver<Scalar> model, DiffusionCoordinateSpec<Scalar> geometry_spec,
										 CoordinateCalibrationSpec<Scalar> calibration_spec, Codec codec,
										 Scalar reconstruction_weight = Scalar(1), Scalar geometry_weight = Scalar(1),
										 std::string pairwise_distances = "exact_space_distances",
										 std::string affinity_kernel = "gaussian_affinity_kernel",
										 std::string diffusion_operator = "row_normalized_diffusion_operator")
	-> EncodedParametricDiffusionCoordinateMap<Scalar, Codec>
{
	return EncodedParametricDiffusionCoordinateMap<Scalar, Codec>(
		std::move(model), std::move(geometry_spec), std::move(codec), std::move(calibration_spec), reconstruction_weight,
		geometry_weight, std::move(pairwise_distances), std::move(affinity_kernel), std::move(diffusion_operator));
}

template <typename Scalar, typename Codec>
auto parametric_diffusion_coordinates_with_codec(solve::parametric::CoordinateSolver<Scalar> model, DiffusionCoordinateSpec<Scalar> geometry_spec,
										 CoordinateCalibrationSpec<Scalar> calibration_spec, Codec codec,
										 modify::compose::PipelinePlan pipeline_plan)
	-> EncodedParametricDiffusionCoordinateMap<Scalar, Codec>
{
	return EncodedParametricDiffusionCoordinateMap<Scalar, Codec>(std::move(model), std::move(geometry_spec),
															   std::move(calibration_spec), std::move(codec),
															   std::move(pipeline_plan));
}

template <typename Record, typename Scalar>
auto make_parametric_diffusion_coordinate_artifact(const ParametricCoordinateMapArtifact<Record, Scalar> &mapping_artifact,
											const solve::parametric::CoordinateObjective<Scalar> &objective,
											const CoordinateCalibrationSpec<Scalar> &calibration_spec,
											const DiffusionCoordinateSpec<Scalar> &geometry_spec,
											const DiffusionCoordinateTargets<Scalar> &geometry_targets,
											std::uint64_t source_space_version = 0)
	-> solve::parametric::CoordinateSolverArtifact<Scalar>
{
	const auto pipeline_json =
		mapping_artifact.has_pipeline_plan()
			? modify::compose::pipeline_plan_to_json(mapping_artifact.pipeline_plan())
			: modify::compose::pipeline_plan_to_json(detail::parametric_diffusion_coordinate_pipeline_plan_for_execution<Scalar>(
				  Scalar(1), Scalar(1), geometry_targets.pairwise_distances, geometry_targets.affinity_kernel,
				  geometry_targets.diffusion_operator));
	mtrc::core::Metadata metadata = {
		{"mapping",
		 {{"name", mapping_artifact.mapping_name()},
		  {"strategy", mapping_artifact.strategy_name()},
		  {"target", "metric_space"}}},
		{"pipeline", pipeline_json},
		{"diffusion_coordinates",
		 {{"spec", diffusion_coordinate_spec_to_json(geometry_spec)},
		  {"targets", diffusion_coordinate_targets_to_json(geometry_targets)}}}};

	return solve::parametric::make_coordinate_solver_artifact(
		mapping_artifact.coordinate_solver(), mapping_artifact.codec(), objective,
		solve::parametric::coordinate_calibration_config(calibration_spec.steps, calibration_spec.batch_size,
														 calibration_spec.shuffle, calibration_spec.seed,
														 calibration_spec.gradient_clip_norm),
		mapping_artifact.source_record_count(), source_space_version, "metric.parametric_diffusion_coordinate_artifact",
		std::move(metadata));
}

template <typename Record, typename Scalar>
auto load_parametric_diffusion_coordinate_artifact(const solve::parametric::CoordinateSolverArtifact<Scalar> &artifact)
	-> ParametricCoordinateMapArtifact<Record, Scalar>
{
	if (artifact.manifest.at("format").template get<std::string>() != "metric.parametric_diffusion_coordinate_artifact") {
		throw std::invalid_argument("unexpected parametric diffusion coordinate artifact format");
	}
	const auto mapping = artifact.manifest.at("mapping");
	const auto mapping_name = mapping.at("name").template get<std::string>();
	const auto strategy_name = mapping.at("strategy").template get<std::string>();
	if (mapping_name != "parametric_diffusion_coordinates" || strategy_name != "native_metric_diffusion_coordinate_solver") {
		throw std::invalid_argument("unexpected parametric diffusion coordinate provenance");
	}

	const auto codec_json = artifact.manifest.at("codec");
	if (codec_json.at("type").template get<std::string>() != "VectorRecordCodec") {
		throw std::invalid_argument("unsupported parametric diffusion coordinate codec");
	}

	using model_type = ParametricCoordinateMapArtifact<Record, Scalar>;
	typename model_type::codec_type codec(codec_json.at("coordinate_count").template get<std::size_t>());
	// The diffusion-coordinate spec/targets and calibration diagnostics are recorded as
	// provenance metadata only: they are intentionally not validated or
	// reconstructed here. The loaded backend solver reproduces transform/inverse_transform
	// from the serialized network, codec, and pipeline plan alone.
	auto model = solve::parametric::load_coordinate_solver_model(artifact);
	const auto source_record_count = artifact.manifest.at("source").at("record_count").template get<std::size_t>();
	modify::compose::PipelinePlan pipeline_plan;
	if (artifact.manifest.contains("pipeline")) {
		pipeline_plan = modify::compose::pipeline_plan_from_json(artifact.manifest.at("pipeline"));
	}
	return model_type(std::move(model), std::move(codec), source_record_count, mapping_name, strategy_name,
					  std::move(pipeline_plan));
}

} // namespace mtrc::modify::map

#endif
