// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_NATIVE_PHATE_AUTOENCODER_HPP
#define _METRIC_MAPPINGS_NATIVE_PHATE_AUTOENCODER_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
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
#include <metric/modify/compose/native_phate_autoencoder_plan.hpp>
#include <metric/modify/dynamics/diffusion.hpp>
#include <metric/space/storage/distance_matrix.hpp>
#include <metric/solve/parametric/dnn.hpp>
#include "native_autoencoder.hpp"
#include <metric/numeric/Math.h>

namespace mtrc::modify::map {

template <typename Scalar> struct PhateGeometrySpec {
	std::size_t dimensions{1};
	std::size_t diffusion_steps{1};
	std::size_t max_dense_records{0};
	Scalar kernel_scale{0};
	Scalar epsilon{Scalar(1.0e-12)};
};

template <typename Scalar> struct PhateGeometryTargets {
	using target_table_type = typename solve::parametric::dnn::BottleneckCoordinateMSELoss<Scalar>::target_table_type;

	target_table_type coordinates;
	std::size_t dimensions{0};
	std::size_t diffusion_steps{0};
	std::size_t record_count{0};
	std::size_t dense_distance_evaluations{0};
	std::size_t max_dense_records{0};
	Scalar kernel_scale{0};
	std::string method{"diffusion_potential_anchor_coordinates"};
	std::string pairwise_distances{"exact_space_distances"};
	std::string affinity_kernel{"gaussian_affinity_kernel"};
	std::string diffusion_operator{"row_normalized_diffusion_operator"};
};

template <typename Scalar> auto phate_geometry_spec_to_json(const PhateGeometrySpec<Scalar> &spec) -> mtrc::core::Metadata
{
	return {{"dimensions", spec.dimensions},
			{"diffusion_steps", spec.diffusion_steps},
			{"max_dense_records", spec.max_dense_records},
			{"kernel_scale", spec.kernel_scale},
			{"epsilon", spec.epsilon}};
}

template <typename Scalar>
auto phate_geometry_targets_to_json(const PhateGeometryTargets<Scalar> &targets) -> mtrc::core::Metadata
{
	return {{"dimensions", targets.dimensions},
			{"diffusion_steps", targets.diffusion_steps},
			{"record_count", targets.record_count},
			{"dense_distance_evaluations", targets.dense_distance_evaluations},
			{"max_dense_records", targets.max_dense_records},
			{"kernel_scale", targets.kernel_scale},
			{"method", targets.method},
			{"pairwise_distances", targets.pairwise_distances},
			{"affinity_kernel", targets.affinity_kernel},
			{"diffusion_operator", targets.diffusion_operator},
			{"target_count", targets.coordinates.size()}};
}

inline auto native_phate_pipeline_components_to_json() -> mtrc::core::Metadata
{
	const auto plan = modify::compose::native_phate_autoencoder_pipeline_plan(1.0, 1.0);
	return modify::compose::pipeline_components_to_json(plan.components());
}

namespace detail {

inline auto is_supported_phate_pairwise_distances(const std::string &name) -> bool
{
	return modify::dynamics::is_supported_pairwise_distances(name);
}

inline auto is_supported_phate_affinity_kernel(const std::string &name) -> bool
{
	return modify::dynamics::is_supported_affinity_kernel(name);
}

inline auto is_supported_phate_diffusion_operator(const std::string &name) -> bool
{
	return modify::dynamics::is_supported_diffusion_operator(name);
}

template <typename Scalar>
auto native_phate_pipeline_plan_for_execution(
	Scalar reconstruction_weight, Scalar geometry_weight, const std::string &pairwise_distances,
	const std::string &affinity_kernel = "gaussian_affinity_kernel",
	const std::string &diffusion_operator = "row_normalized_diffusion_operator",
	const std::string &codec = "vector_record_codec") -> modify::compose::PipelinePlan
{
	if (codec != "vector_record_codec" && codec != "feature_record_codec") {
		throw std::invalid_argument("unsupported native PHATE autoencoder codec");
	}
	if (!is_supported_phate_pairwise_distances(pairwise_distances)) {
		throw std::invalid_argument("unsupported native PHATE autoencoder distance provider");
	}
	if (!is_supported_phate_affinity_kernel(affinity_kernel)) {
		throw std::invalid_argument("unsupported native PHATE autoencoder affinity kernel");
	}
	if (!is_supported_phate_diffusion_operator(diffusion_operator)) {
		throw std::invalid_argument("unsupported native PHATE autoencoder diffusion operator");
	}
	auto builder = modify::compose::native_phate_autoencoder_pipeline_builder(reconstruction_weight, geometry_weight);
	if (codec == "feature_record_codec") {
		builder.use_feature_record_codec();
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
auto phate_geometry_targets(const Space &space, PhateGeometrySpec<Scalar> spec = {},
							std::string pairwise_distances = "exact_space_distances",
							std::string affinity_kernel = "gaussian_affinity_kernel",
							std::string diffusion_operator = "row_normalized_diffusion_operator")
	-> PhateGeometryTargets<Scalar>
{
	if (space.empty()) {
		throw std::invalid_argument("PHATE geometry targets require a non-empty space");
	}
	if (!detail::is_supported_phate_pairwise_distances(pairwise_distances)) {
		throw std::invalid_argument("unsupported PHATE geometry distance provider");
	}
	if (!detail::is_supported_phate_affinity_kernel(affinity_kernel)) {
		throw std::invalid_argument("unsupported PHATE geometry affinity kernel");
	}
	if (!detail::is_supported_phate_diffusion_operator(diffusion_operator)) {
		throw std::invalid_argument("unsupported PHATE geometry diffusion operator");
	}
	if (spec.dimensions == 0) {
		throw std::invalid_argument("PHATE geometry target dimensions must be positive");
	}
	if (spec.diffusion_steps == 0) {
		throw std::invalid_argument("PHATE geometry diffusion steps must be positive");
	}
	if (spec.epsilon <= Scalar(0)) {
		throw std::invalid_argument("PHATE geometry epsilon must be positive");
	}

	modify::dynamics::DiffusionOptions<Scalar> diffusion_options;
	diffusion_options.diffusion_steps = spec.diffusion_steps;
	diffusion_options.max_dense_records = spec.max_dense_records;
	diffusion_options.kernel_scale = spec.kernel_scale;
	diffusion_options.epsilon = spec.epsilon;
	diffusion_options.pairwise_distances = std::move(pairwise_distances);
	diffusion_options.affinity_kernel = std::move(affinity_kernel);
	diffusion_options.diffusion_operator = std::move(diffusion_operator);

	const auto process = modify::dynamics::diffusion_process<Space, Scalar>(space, std::move(diffusion_options));
	auto coordinate_rows = modify::dynamics::diffusion_potential_anchor_coordinates(process, spec.dimensions);

	PhateGeometryTargets<Scalar> targets;
	targets.dimensions = spec.dimensions;
	targets.diffusion_steps = process.diffusion_steps;
	targets.record_count = process.record_count;
	targets.dense_distance_evaluations = process.dense_distance_evaluations;
	targets.max_dense_records = process.max_dense_records;
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
auto native_phate_autoencoder_objective(const solve::parametric::dnn::AutoencoderModel<Scalar> &model,
										const PhateGeometryTargets<Scalar> &targets,
										Scalar reconstruction_weight = Scalar(1), Scalar geometry_weight = Scalar(1))
	-> solve::parametric::dnn::CompositeLoss<Scalar>
{
	if (reconstruction_weight < Scalar(0) || geometry_weight < Scalar(0)) {
		throw std::invalid_argument("native PHATE autoencoder loss weights must be non-negative");
	}
	if (reconstruction_weight == Scalar(0) && geometry_weight == Scalar(0)) {
		throw std::invalid_argument("native PHATE autoencoder requires at least one positive loss weight");
	}
	if (targets.dimensions != model.latent_dimension()) {
		throw std::invalid_argument("PHATE target dimensions must match the autoencoder bottleneck dimension");
	}

	solve::parametric::dnn::CompositeLoss<Scalar> objective;
	if (reconstruction_weight > Scalar(0)) {
		objective.add(std::make_shared<solve::parametric::dnn::ReconstructionMSELoss<Scalar>>(), reconstruction_weight);
	}
	if (geometry_weight > Scalar(0)) {
		objective.add(std::make_shared<solve::parametric::dnn::BottleneckCoordinateMSELoss<Scalar>>(model.topology().bottleneck_layer,
																				 targets.coordinates),
					  geometry_weight);
	}
	return objective;
}

template <typename Scalar> class NativePhateAutoencoderMapping {
  public:
	using scalar_type = Scalar;

	NativePhateAutoencoderMapping(solve::parametric::dnn::AutoencoderModel<scalar_type> prototype,
								  PhateGeometrySpec<scalar_type> geometry_spec,
								  solve::parametric::dnn::TrainingSpec<scalar_type> training_spec = {},
								  scalar_type reconstruction_weight = scalar_type(1),
								  scalar_type geometry_weight = scalar_type(1),
								  std::string pairwise_distances = "exact_space_distances",
								  std::string affinity_kernel = "gaussian_affinity_kernel",
								  std::string diffusion_operator = "row_normalized_diffusion_operator")
		: prototype_(std::move(prototype)), geometry_spec_(std::move(geometry_spec)),
		  training_spec_(std::move(training_spec)), reconstruction_weight_(reconstruction_weight),
		  geometry_weight_(geometry_weight), pairwise_distances_(std::move(pairwise_distances)),
		  affinity_kernel_(std::move(affinity_kernel)), diffusion_operator_(std::move(diffusion_operator)),
		  pipeline_plan_(detail::native_phate_pipeline_plan_for_execution(
			  reconstruction_weight_, geometry_weight_, pairwise_distances_, affinity_kernel_, diffusion_operator_))
	{
		validate_parameters();
	}

	NativePhateAutoencoderMapping(solve::parametric::dnn::AutoencoderModel<scalar_type> prototype,
								  PhateGeometrySpec<scalar_type> geometry_spec,
								  solve::parametric::dnn::TrainingSpec<scalar_type> training_spec, modify::compose::PipelinePlan pipeline_plan)
		: prototype_(std::move(prototype)), geometry_spec_(std::move(geometry_spec)),
		  training_spec_(std::move(training_spec)), pipeline_plan_(std::move(pipeline_plan))
	{
		const auto hooks = modify::compose::resolve_native_phate_autoencoder_executable_hooks<scalar_type>(pipeline_plan_);
		hooks.validate();
		reconstruction_weight_ = hooks.reconstruction_weight;
		geometry_weight_ = hooks.geometry_weight;
		pairwise_distances_ = hooks.pairwise_distances;
		affinity_kernel_ = hooks.affinity_kernel;
		diffusion_operator_ = hooks.diffusion_operator;
		if (hooks.codec != "vector_record_codec") {
			throw std::invalid_argument("native PHATE autoencoder default mapping requires vector_record_codec");
		}
		validate_parameters();
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto fit(const Space &space) const -> NativeAutoencoderModel<typename Space::record_type, scalar_type>
	{
		if (space.empty()) {
			throw std::invalid_argument("cannot fit native PHATE autoencoder mapping on an empty space");
		}

		using record_type = typename Space::record_type;
		using model_type = NativeAutoencoderModel<record_type, scalar_type>;
		using codec_type = typename model_type::codec_type;

		codec_type codec(space.records().front().size());
		auto features = codec.encode_batch(space.records());
		auto ids = mtrc::record_ids(space);

		auto model = prototype_.clone();
		const auto targets = phate_geometry_targets<Space, scalar_type>(space, geometry_spec_, pairwise_distances_,
																		affinity_kernel_, diffusion_operator_);
		if (targets.dimensions != model.latent_dimension()) {
			throw std::invalid_argument("PHATE target dimensions must match the autoencoder bottleneck dimension");
		}

		auto objective = native_phate_autoencoder_objective(model, targets, reconstruction_weight_, geometry_weight_);

		solve::parametric::dnn::EncodedDataset<scalar_type> dataset(std::move(ids), std::move(features), space.version());
		const solve::parametric::dnn::NativeDnnTrainer<scalar_type> trainer;
		trainer.fit(model, dataset, objective, training_spec_);
		return model_type(std::move(model), std::move(codec), space.size(), "native_phate_autoencoder",
						  "native_dnn_phate_ae", pipeline_plan_);
	}

  private:
	auto validate_parameters() const -> void
	{
		if (reconstruction_weight_ < scalar_type(0) || geometry_weight_ < scalar_type(0)) {
			throw std::invalid_argument("native PHATE autoencoder loss weights must be non-negative");
		}
		if (reconstruction_weight_ == scalar_type(0) && geometry_weight_ == scalar_type(0)) {
			throw std::invalid_argument("native PHATE autoencoder requires at least one positive loss weight");
		}
		if (!detail::is_supported_phate_pairwise_distances(pairwise_distances_)) {
			throw std::invalid_argument("unsupported native PHATE autoencoder distance provider");
		}
		if (!detail::is_supported_phate_affinity_kernel(affinity_kernel_)) {
			throw std::invalid_argument("unsupported native PHATE autoencoder affinity kernel");
		}
		if (!detail::is_supported_phate_diffusion_operator(diffusion_operator_)) {
			throw std::invalid_argument("unsupported native PHATE autoencoder diffusion operator");
		}
	}

	solve::parametric::dnn::AutoencoderModel<scalar_type> prototype_;
	PhateGeometrySpec<scalar_type> geometry_spec_;
	solve::parametric::dnn::TrainingSpec<scalar_type> training_spec_;
	scalar_type reconstruction_weight_{1};
	scalar_type geometry_weight_{1};
	std::string pairwise_distances_{"exact_space_distances"};
	std::string affinity_kernel_{"gaussian_affinity_kernel"};
	std::string diffusion_operator_{"row_normalized_diffusion_operator"};
	modify::compose::PipelinePlan pipeline_plan_;
};

template <typename Scalar, typename Codec> class NativeFeaturePhateAutoencoderMapping {
  public:
	using scalar_type = Scalar;
	using codec_type = Codec;

	NativeFeaturePhateAutoencoderMapping(solve::parametric::dnn::AutoencoderModel<scalar_type> prototype,
										 PhateGeometrySpec<scalar_type> geometry_spec, codec_type codec,
										 solve::parametric::dnn::TrainingSpec<scalar_type> training_spec = {},
										 scalar_type reconstruction_weight = scalar_type(1),
										 scalar_type geometry_weight = scalar_type(1),
										 std::string pairwise_distances = "exact_space_distances",
										 std::string affinity_kernel = "gaussian_affinity_kernel",
										 std::string diffusion_operator = "row_normalized_diffusion_operator")
		: prototype_(std::move(prototype)), geometry_spec_(std::move(geometry_spec)),
		  training_spec_(std::move(training_spec)), codec_(std::move(codec)),
		  reconstruction_weight_(reconstruction_weight), geometry_weight_(geometry_weight),
		  pairwise_distances_(std::move(pairwise_distances)), affinity_kernel_(std::move(affinity_kernel)),
		  diffusion_operator_(std::move(diffusion_operator)),
		  pipeline_plan_(detail::native_phate_pipeline_plan_for_execution(reconstruction_weight_, geometry_weight_,
																		  pairwise_distances_, affinity_kernel_,
																		  diffusion_operator_, "feature_record_codec"))
	{
		validate_parameters();
	}

	NativeFeaturePhateAutoencoderMapping(solve::parametric::dnn::AutoencoderModel<scalar_type> prototype,
										 PhateGeometrySpec<scalar_type> geometry_spec,
										 solve::parametric::dnn::TrainingSpec<scalar_type> training_spec, codec_type codec,
										 modify::compose::PipelinePlan pipeline_plan)
		: prototype_(std::move(prototype)), geometry_spec_(std::move(geometry_spec)),
		  training_spec_(std::move(training_spec)), codec_(std::move(codec)), pipeline_plan_(std::move(pipeline_plan))
	{
		const auto hooks = modify::compose::resolve_native_phate_autoencoder_executable_hooks<scalar_type>(pipeline_plan_);
		hooks.validate();
		if (hooks.codec != "feature_record_codec") {
			throw std::invalid_argument("native feature PHATE autoencoder mapping requires feature_record_codec");
		}
		reconstruction_weight_ = hooks.reconstruction_weight;
		geometry_weight_ = hooks.geometry_weight;
		pairwise_distances_ = hooks.pairwise_distances;
		affinity_kernel_ = hooks.affinity_kernel;
		diffusion_operator_ = hooks.diffusion_operator;
		validate_parameters();
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto fit(const Space &space) const -> NativeAutoencoderModel<typename Space::record_type, scalar_type, codec_type>
	{
		if (space.empty()) {
			throw std::invalid_argument("cannot fit native feature PHATE autoencoder mapping on an empty space");
		}

		using record_type = typename Space::record_type;
		using model_type = NativeAutoencoderModel<record_type, scalar_type, codec_type>;

		auto features = codec_.encode_batch(space.records());
		if (features.rows() != space.size()) {
			throw std::invalid_argument("native feature PHATE autoencoder codec row count does not match space");
		}
		if (features.columns() != prototype_.network().layers.at(prototype_.topology().input_layer)->getInputSize()) {
			throw std::invalid_argument("native feature PHATE autoencoder codec feature count does not match network");
		}

		auto ids = mtrc::record_ids(space);

		auto model = prototype_.clone();
		const auto targets = phate_geometry_targets<Space, scalar_type>(space, geometry_spec_, pairwise_distances_,
																		affinity_kernel_, diffusion_operator_);
		if (targets.dimensions != model.latent_dimension()) {
			throw std::invalid_argument("PHATE target dimensions must match the autoencoder bottleneck dimension");
		}

		auto objective = native_phate_autoencoder_objective(model, targets, reconstruction_weight_, geometry_weight_);

		solve::parametric::dnn::EncodedDataset<scalar_type> dataset(std::move(ids), std::move(features), space.version());
		const solve::parametric::dnn::NativeDnnTrainer<scalar_type> trainer;
		trainer.fit(model, dataset, objective, training_spec_);
		return model_type(std::move(model), codec_, space.size(), "native_phate_autoencoder", "native_dnn_phate_ae",
						  pipeline_plan_);
	}

  private:
	auto validate_parameters() const -> void
	{
		if (reconstruction_weight_ < scalar_type(0) || geometry_weight_ < scalar_type(0)) {
			throw std::invalid_argument("native feature PHATE autoencoder loss weights must be non-negative");
		}
		if (reconstruction_weight_ == scalar_type(0) && geometry_weight_ == scalar_type(0)) {
			throw std::invalid_argument("native feature PHATE autoencoder requires at least one positive loss weight");
		}
		if (!detail::is_supported_phate_pairwise_distances(pairwise_distances_)) {
			throw std::invalid_argument("unsupported native feature PHATE autoencoder distance provider");
		}
		if (!detail::is_supported_phate_affinity_kernel(affinity_kernel_)) {
			throw std::invalid_argument("unsupported native feature PHATE autoencoder affinity kernel");
		}
		if (!detail::is_supported_phate_diffusion_operator(diffusion_operator_)) {
			throw std::invalid_argument("unsupported native feature PHATE autoencoder diffusion operator");
		}
	}

	solve::parametric::dnn::AutoencoderModel<scalar_type> prototype_;
	PhateGeometrySpec<scalar_type> geometry_spec_;
	solve::parametric::dnn::TrainingSpec<scalar_type> training_spec_;
	codec_type codec_;
	scalar_type reconstruction_weight_{1};
	scalar_type geometry_weight_{1};
	std::string pairwise_distances_{"exact_space_distances"};
	std::string affinity_kernel_{"gaussian_affinity_kernel"};
	std::string diffusion_operator_{"row_normalized_diffusion_operator"};
	modify::compose::PipelinePlan pipeline_plan_;
};

template <typename Scalar>
auto native_phate_autoencoder(solve::parametric::dnn::AutoencoderModel<Scalar> model, PhateGeometrySpec<Scalar> geometry_spec,
							  solve::parametric::dnn::TrainingSpec<Scalar> training_spec = {}, Scalar reconstruction_weight = Scalar(1),
							  Scalar geometry_weight = Scalar(1),
							  std::string pairwise_distances = "exact_space_distances",
							  std::string affinity_kernel = "gaussian_affinity_kernel",
							  std::string diffusion_operator = "row_normalized_diffusion_operator")
	-> NativePhateAutoencoderMapping<Scalar>
{
	return NativePhateAutoencoderMapping<Scalar>(std::move(model), std::move(geometry_spec), std::move(training_spec),
												 reconstruction_weight, geometry_weight, std::move(pairwise_distances),
												 std::move(affinity_kernel), std::move(diffusion_operator));
}

template <typename Scalar>
auto native_phate_autoencoder(solve::parametric::dnn::AutoencoderModel<Scalar> model, PhateGeometrySpec<Scalar> geometry_spec,
							  solve::parametric::dnn::TrainingSpec<Scalar> training_spec, modify::compose::PipelinePlan pipeline_plan)
	-> NativePhateAutoencoderMapping<Scalar>
{
	return NativePhateAutoencoderMapping<Scalar>(std::move(model), std::move(geometry_spec), std::move(training_spec),
												 std::move(pipeline_plan));
}

template <typename Scalar, typename Codec>
auto native_phate_autoencoder_with_codec(solve::parametric::dnn::AutoencoderModel<Scalar> model, PhateGeometrySpec<Scalar> geometry_spec,
										 solve::parametric::dnn::TrainingSpec<Scalar> training_spec, Codec codec,
										 Scalar reconstruction_weight = Scalar(1), Scalar geometry_weight = Scalar(1),
										 std::string pairwise_distances = "exact_space_distances",
										 std::string affinity_kernel = "gaussian_affinity_kernel",
										 std::string diffusion_operator = "row_normalized_diffusion_operator")
	-> NativeFeaturePhateAutoencoderMapping<Scalar, Codec>
{
	return NativeFeaturePhateAutoencoderMapping<Scalar, Codec>(
		std::move(model), std::move(geometry_spec), std::move(codec), std::move(training_spec), reconstruction_weight,
		geometry_weight, std::move(pairwise_distances), std::move(affinity_kernel), std::move(diffusion_operator));
}

template <typename Scalar, typename Codec>
auto native_phate_autoencoder_with_codec(solve::parametric::dnn::AutoencoderModel<Scalar> model, PhateGeometrySpec<Scalar> geometry_spec,
										 solve::parametric::dnn::TrainingSpec<Scalar> training_spec, Codec codec,
										 modify::compose::PipelinePlan pipeline_plan)
	-> NativeFeaturePhateAutoencoderMapping<Scalar, Codec>
{
	return NativeFeaturePhateAutoencoderMapping<Scalar, Codec>(std::move(model), std::move(geometry_spec),
															   std::move(training_spec), std::move(codec),
															   std::move(pipeline_plan));
}

template <typename Record, typename Scalar>
auto make_native_phate_autoencoder_artifact(const NativeAutoencoderModel<Record, Scalar> &model,
											const solve::parametric::dnn::CompositeLoss<Scalar> &objective,
											const solve::parametric::dnn::TrainingSpec<Scalar> &training_spec,
											const PhateGeometrySpec<Scalar> &geometry_spec,
											const PhateGeometryTargets<Scalar> &geometry_targets,
											std::uint64_t source_space_version = 0)
	-> solve::parametric::dnn::NativeAutoencoderArtifact<Scalar>
{
	const auto pipeline_json =
		model.has_pipeline_plan()
			? modify::compose::pipeline_plan_to_json(model.pipeline_plan())
			: modify::compose::pipeline_plan_to_json(detail::native_phate_pipeline_plan_for_execution<Scalar>(
				  Scalar(1), Scalar(1), geometry_targets.pairwise_distances, geometry_targets.affinity_kernel,
				  geometry_targets.diffusion_operator));
	mtrc::core::Metadata metadata = {
		{"mapping", {{"name", model.mapping_name()}, {"strategy", model.strategy_name()}, {"target", "metric_space"}}},
		{"pipeline", pipeline_json},
		{"phate_geometry",
		 {{"spec", phate_geometry_spec_to_json(geometry_spec)},
		  {"targets", phate_geometry_targets_to_json(geometry_targets)}}}};

	return solve::parametric::dnn::make_native_autoencoder_artifact(model.native_model(), model.codec(), objective, training_spec,
												 model.source_record_count(), source_space_version,
												 "metric.native_phate_autoencoder_artifact", std::move(metadata));
}

template <typename Record, typename Scalar>
auto load_native_phate_autoencoder_model(const solve::parametric::dnn::NativeAutoencoderArtifact<Scalar> &artifact)
	-> NativeAutoencoderModel<Record, Scalar>
{
	if (artifact.manifest.at("format").template get<std::string>() != "metric.native_phate_autoencoder_artifact") {
		throw std::invalid_argument("unexpected native PHATE autoencoder artifact format");
	}
	const auto mapping = artifact.manifest.at("mapping");
	const auto mapping_name = mapping.at("name").template get<std::string>();
	const auto strategy_name = mapping.at("strategy").template get<std::string>();
	if (mapping_name != "native_phate_autoencoder" || strategy_name != "native_dnn_phate_ae") {
		throw std::invalid_argument("unexpected native PHATE autoencoder mapping provenance");
	}

	const auto codec_json = artifact.manifest.at("codec");
	if (codec_json.at("type").template get<std::string>() != "VectorRecordCodec") {
		throw std::invalid_argument("unsupported native PHATE autoencoder codec");
	}

	using model_type = NativeAutoencoderModel<Record, Scalar>;
	typename model_type::codec_type codec(codec_json.at("feature_count").template get<std::size_t>());
	// The PHATE geometry spec/targets and the training diagnostics are recorded as
	// provenance metadata only: they are intentionally not validated or
	// reconstructed here. The loaded model reproduces transform/inverse_transform
	// from the serialized network, codec, and pipeline plan alone.
	auto model = solve::parametric::dnn::load_native_autoencoder_model(artifact);
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
