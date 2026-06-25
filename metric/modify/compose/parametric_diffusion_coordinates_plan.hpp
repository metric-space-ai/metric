// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_PIPELINES_PARAMETRIC_DIFFUSION_COORDINATES_PLAN_HPP
#define _METRIC_PIPELINES_PARAMETRIC_DIFFUSION_COORDINATES_PLAN_HPP

#include <cstddef>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/core/errors.hpp>
#include "pipeline.hpp"

namespace mtrc::modify::compose {

namespace detail {

template <typename Scalar> auto scalar_to_string(Scalar value) -> std::string
{
	std::ostringstream stream;
	stream << value;
	return stream.str();
}

template <typename Scalar> auto scalar_from_string(const std::string &text, const std::string &context) -> Scalar
{
	std::istringstream stream(text);
	Scalar value{};
	stream >> value;
	if (!stream || !stream.eof()) {
		throw PipelineValidationError("invalid scalar pipeline parameter for " + context);
	}
	return value;
}

} // namespace detail

template <typename Scalar> struct ParametricDiffusionCoordinateExecutableHooks {
	PipelinePlan plan;
	std::string codec{"vector_record_codec"};
	std::string pairwise_distances{"exact_space_distances"};
	std::string affinity_kernel{"gaussian_affinity_kernel"};
	std::string diffusion_operator{"row_normalized_diffusion_operator"};
	Scalar reconstruction_weight{0};
	Scalar geometry_weight{0};
	std::vector<std::string> unsupported_reasons;

	auto executable() const -> bool { return unsupported_reasons.empty(); }

	auto validate() const -> void
	{
		if (executable()) {
			return;
		}
		throw PipelineValidationError("parametric diffusion coordinate pipeline plan is not executable: " +
									  unsupported_reasons.front());
	}
};

template <typename Scalar>
auto resolve_parametric_diffusion_coordinate_executable_hooks(const PipelinePlan &plan)
	-> ParametricDiffusionCoordinateExecutableHooks<Scalar>
{
	ParametricDiffusionCoordinateExecutableHooks<Scalar> hooks;
	hooks.plan = plan;
	if (plan.name() != "parametric_diffusion_coordinate_pipeline") {
		hooks.unsupported_reasons.push_back("unexpected pipeline plan name '" + plan.name() + "'");
	}

	const std::vector<std::pair<std::string, std::vector<std::string>>> required_components{
		{"space", {"metric_space"}},
		{"codec", {"vector_record_codec", "record_coordinate_codec"}},
		{"pairwise_distances", {"exact_space_distances", "distance_table_pairwise_distances"}},
		{"affinity_kernel", {"gaussian_affinity_kernel", "exponential_affinity_kernel"}},
		{"diffusion_operator", {"row_normalized_diffusion_operator", "lazy_row_normalized_diffusion_operator"}},
		{"target_generator", {"diffusion_potential_anchor_coordinates"}},
		{"coordinate_calibration", {"native_coordinate_calibration"}},
		{"coordinate_artifact", {"native_coordinate_mapping_artifact"}},
		{"artifact", {"native_coordinate_artifact"}},
	};

	const auto is_required_role = [&](const std::string &role) {
		for (const auto &component : required_components) {
			if (component.first == role) {
				return true;
			}
		}
		return false;
	};
	const auto expected_names_for_role = [&](const std::string &role) -> std::vector<std::string> {
		for (const auto &component : required_components) {
			if (component.first == role) {
				return component.second;
			}
		}
		return {};
	};
	const auto is_supported_component = [&](const std::string &role, const std::string &name) {
		const auto expected_names = expected_names_for_role(role);
		for (const auto &expected_name : expected_names) {
			if (name == expected_name) {
				return true;
			}
		}
		return false;
	};
	const auto expected_names_message = [&](const std::string &role) {
		const auto expected_names = expected_names_for_role(role);
		std::string message;
		for (std::size_t index = 0; index < expected_names.size(); ++index) {
			if (index > 0) {
				message += index + 1 == expected_names.size() ? "' or '" : "', '";
			}
			message += expected_names[index];
		}
		return message;
	};

	for (const auto &required : required_components) {
		bool found_role = false;
		bool found_supported = false;
		for (const auto &component : plan.components()) {
			if (component.role != required.first) {
				continue;
			}
			found_role = true;
			if (is_supported_component(component.role, component.name)) {
				found_supported = true;
				if (!component.parameters.empty()) {
					hooks.unsupported_reasons.push_back("component '" + component.role + "/" + component.name +
														"' has metadata parameters that are not executable hooks yet");
				}
				if (component.role == "codec") {
					hooks.codec = component.name;
				} else if (component.role == "pairwise_distances") {
					hooks.pairwise_distances = component.name;
				} else if (component.role == "affinity_kernel") {
					hooks.affinity_kernel = component.name;
				} else if (component.role == "diffusion_operator") {
					hooks.diffusion_operator = component.name;
				}
			} else {
				hooks.unsupported_reasons.push_back("component '" + component.role + "/" + component.name +
													"' is metadata-only; executable hook requires '" +
													expected_names_message(component.role) + "'");
			}
		}
		if (!found_role) {
			hooks.unsupported_reasons.push_back("missing executable component role '" + required.first + "'");
		} else if (!found_supported) {
			hooks.unsupported_reasons.push_back("missing executable component '" + required.first + "/" +
												expected_names_message(required.first) + "'");
		}
	}

	for (const auto &component : plan.components()) {
		if (component.role == "loss") {
			const auto weight = component.parameters.find("weight");
			if (weight == component.parameters.end()) {
				hooks.unsupported_reasons.push_back("loss component '" + component.name + "' is missing weight");
				continue;
			}
			if (component.parameters.size() != 1) {
				hooks.unsupported_reasons.push_back("loss component '" + component.name +
													"' has metadata parameters that are not executable hooks yet");
			}
			const auto parsed_weight = detail::scalar_from_string<Scalar>(weight->second, component.name + ".weight");
			if (parsed_weight < Scalar(0)) {
				hooks.unsupported_reasons.push_back("loss component '" + component.name + "' has negative weight");
			}
			if (component.name == "reconstruction_mse_loss") {
				hooks.reconstruction_weight = parsed_weight;
			} else if (component.name == "bottleneck_coordinate_mse_loss") {
				hooks.geometry_weight = parsed_weight;
			} else {
				hooks.unsupported_reasons.push_back("loss component '" + component.name +
													"' is metadata-only and has no executable hook");
			}
			continue;
		}
		if (!is_required_role(component.role)) {
			hooks.unsupported_reasons.push_back("unknown pipeline component role '" + component.role + "'");
			continue;
		}
		if (!is_supported_component(component.role, component.name)) {
			continue;
		}
	}

	if (hooks.reconstruction_weight == Scalar(0) && hooks.geometry_weight == Scalar(0)) {
		hooks.unsupported_reasons.push_back(
			"parametric diffusion coordinate executable plan requires at least one positive loss");
	}
	return hooks;
}

template <typename Scalar> class ParametricDiffusionCoordinatePipelinePlanBuilder {
  public:
	using scalar_type = Scalar;
	using parameter_map = std::map<std::string, std::string>;

	explicit ParametricDiffusionCoordinatePipelinePlanBuilder(scalar_type reconstruction_weight = scalar_type(1),
															 scalar_type geometry_weight = scalar_type(1))
		: reconstruction_weight_(reconstruction_weight), geometry_weight_(geometry_weight)
	{
	}

	auto replace_pairwise_distances(std::string name, parameter_map parameters = {})
		-> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		pairwise_distances_ = {"pairwise_distances", std::move(name), std::move(parameters)};
		return *this;
	}

	auto use_distance_table_pairwise_distances() -> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		return replace_pairwise_distances("distance_table_pairwise_distances");
	}

	auto replace_codec(std::string name, parameter_map parameters = {})
		-> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		codec_ = {"codec", std::move(name), std::move(parameters), true};
		return *this;
	}

	auto use_record_coordinate_codec() -> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		return replace_codec("record_coordinate_codec");
	}

	auto replace_affinity_kernel(std::string name, parameter_map parameters = {})
		-> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		affinity_kernel_ = {"affinity_kernel", std::move(name), std::move(parameters)};
		return *this;
	}

	auto use_exponential_affinity_kernel() -> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		return replace_affinity_kernel("exponential_affinity_kernel");
	}

	auto replace_diffusion_operator(std::string name, parameter_map parameters = {})
		-> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		diffusion_operator_ = {"diffusion_operator", std::move(name), std::move(parameters)};
		return *this;
	}

	auto use_lazy_row_normalized_diffusion_operator() -> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		return replace_diffusion_operator("lazy_row_normalized_diffusion_operator");
	}

	auto replace_target_generator(std::string name, parameter_map parameters = {}, bool contributes_to_artifact = true)
		-> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		target_generator_ = {"target_generator", std::move(name), std::move(parameters), contributes_to_artifact};
		return *this;
	}

	auto replace_coordinate_calibration(std::string name, parameter_map parameters = {})
		-> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		coordinate_calibration_ = {"coordinate_calibration", std::move(name), std::move(parameters)};
		return *this;
	}

	auto replace_coordinate_artifact(std::string name, parameter_map parameters = {}, bool contributes_to_artifact = true)
		-> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		coordinate_artifact_ = {"coordinate_artifact", std::move(name), std::move(parameters), contributes_to_artifact};
		return *this;
	}

	auto replace_artifact(std::string name, parameter_map parameters = {}, bool contributes_to_artifact = true)
		-> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		artifact_ = {"artifact", std::move(name), std::move(parameters), contributes_to_artifact};
		return *this;
	}

	auto replace_reconstruction_loss(std::string name, scalar_type weight, parameter_map parameters = {})
		-> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		reconstruction_loss_name_ = std::move(name);
		reconstruction_weight_ = weight;
		reconstruction_loss_parameters_ = std::move(parameters);
		return *this;
	}

	auto replace_geometry_loss(std::string name, scalar_type weight, parameter_map parameters = {})
		-> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		geometry_loss_name_ = std::move(name);
		geometry_weight_ = weight;
		geometry_loss_parameters_ = std::move(parameters);
		return *this;
	}

	auto without_reconstruction_loss() -> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		reconstruction_weight_ = scalar_type(0);
		return *this;
	}

	auto without_geometry_loss() -> ParametricDiffusionCoordinatePipelinePlanBuilder &
	{
		geometry_weight_ = scalar_type(0);
		return *this;
	}

	auto plan() const -> PipelinePlan
	{
		std::vector<PipelineComponent> components{
			space_,	  codec_,		  pairwise_distances_, affinity_kernel_, diffusion_operator_, target_generator_,
			coordinate_calibration_, coordinate_artifact_, artifact_};
		if (reconstruction_weight_ > scalar_type(0)) {
			auto parameters = reconstruction_loss_parameters_;
			parameters["weight"] = detail::scalar_to_string(reconstruction_weight_);
			components.push_back({"loss", reconstruction_loss_name_, std::move(parameters)});
		}
		if (geometry_weight_ > scalar_type(0)) {
			auto parameters = geometry_loss_parameters_;
			parameters["weight"] = detail::scalar_to_string(geometry_weight_);
			components.push_back({"loss", geometry_loss_name_, std::move(parameters)});
		}
		return PipelinePlan("parametric_diffusion_coordinate_pipeline", std::move(components));
	}

  private:
	PipelineComponent space_{"space", "metric_space"};
	PipelineComponent codec_{"codec", "vector_record_codec", {}, true};
	PipelineComponent pairwise_distances_{"pairwise_distances", "exact_space_distances"};
	PipelineComponent affinity_kernel_{"affinity_kernel", "gaussian_affinity_kernel"};
	PipelineComponent diffusion_operator_{"diffusion_operator", "row_normalized_diffusion_operator"};
	PipelineComponent target_generator_{"target_generator", "diffusion_potential_anchor_coordinates", {}, true};
	PipelineComponent coordinate_calibration_{"coordinate_calibration", "native_coordinate_calibration"};
	PipelineComponent coordinate_artifact_{"coordinate_artifact", "native_coordinate_mapping_artifact", {}, true};
	PipelineComponent artifact_{"artifact", "native_coordinate_artifact", {}, true};
	std::string reconstruction_loss_name_{"reconstruction_mse_loss"};
	std::string geometry_loss_name_{"bottleneck_coordinate_mse_loss"};
	parameter_map reconstruction_loss_parameters_;
	parameter_map geometry_loss_parameters_;
	scalar_type reconstruction_weight_{1};
	scalar_type geometry_weight_{1};
};

template <typename Scalar>
auto parametric_diffusion_coordinate_pipeline_plan(Scalar reconstruction_weight, Scalar geometry_weight) -> PipelinePlan
{
	return ParametricDiffusionCoordinatePipelinePlanBuilder<Scalar>(reconstruction_weight, geometry_weight).plan();
}

template <typename Scalar>
auto parametric_diffusion_coordinate_pipeline_builder(Scalar reconstruction_weight, Scalar geometry_weight)
	-> ParametricDiffusionCoordinatePipelinePlanBuilder<Scalar>
{
	return ParametricDiffusionCoordinatePipelinePlanBuilder<Scalar>(reconstruction_weight, geometry_weight);
}

inline auto parametric_diffusion_coordinate_replacement_points() -> std::vector<PipelineReplacementPoint>
{
	return {{"pairwise_distances", "exact_space_distances",
			 "Distance-provider metadata for exact, cached, graph-backed, or future approximate distance access.",
			 true},
			{"codec", "vector_record_codec", "Codec metadata for turning source records into solver coordinates.",
			 true},
			{"affinity_kernel", "gaussian_affinity_kernel",
			 "Affinity-kernel metadata for turning metric distances into neighborhood weights.", true},
			{"diffusion_operator", "row_normalized_diffusion_operator",
			 "Diffusion-operator metadata for normalizing and propagating affinity structure.", true},
			{"target_generator", "diffusion_potential_anchor_coordinates",
			 "Target-generator metadata for producing diffusion-potential anchor coordinates.", true},
			{"coordinate_calibration", "native_coordinate_calibration",
			 "Native calibration metadata for the parametric coordinate component.", true},
			{"loss", "reconstruction_mse_loss", "Optional reconstruction-loss metadata.", true},
			{"loss", "bottleneck_coordinate_mse_loss", "Optional bottleneck-geometry-loss metadata.", true},
			{"coordinate_artifact", "native_coordinate_mapping_artifact",
			 "Coordinate-artifact metadata for transform-capable native maps.", true},
			{"artifact", "native_coordinate_artifact",
			 "Artifact-boundary metadata for native coordinate-map persistence.", true}};
}

} // namespace mtrc::modify::compose

#endif
