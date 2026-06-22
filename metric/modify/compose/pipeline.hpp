// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_PIPELINES_PIPELINE_HPP
#define _METRIC_PIPELINES_PIPELINE_HPP

#include <algorithm>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/core/metadata.hpp>

#include <metric/core/errors.hpp>

namespace mtrc::modify::compose {

struct PipelineComponent {
	std::string role;
	std::string name;
	std::map<std::string, std::string> parameters;
	bool contributes_to_artifact{false};

	PipelineComponent() = default;

	PipelineComponent(std::string component_role, std::string component_name,
					  std::map<std::string, std::string> component_parameters = {}, bool artifact_contribution = false)
		: role(std::move(component_role)), name(std::move(component_name)), parameters(std::move(component_parameters)),
		  contributes_to_artifact(artifact_contribution)
	{
	}

	auto has_parameter(const std::string &key, const std::string &value) const -> bool
	{
		const auto found = parameters.find(key);
		return found != parameters.end() && found->second == value;
	}
};

struct PipelineReplacementPoint {
	std::string role;
	std::string default_name;
	std::string description;
	bool controls_execution{false};

	PipelineReplacementPoint() = default;

	PipelineReplacementPoint(std::string component_role, std::string component_name, std::string component_description,
							 bool execution_bound = false)
		: role(std::move(component_role)), default_name(std::move(component_name)),
		  description(std::move(component_description)), controls_execution(execution_bound)
	{
	}
};

class PipelinePlan {
  public:
	using component_type = PipelineComponent;

	PipelinePlan() = default;

	PipelinePlan(std::string plan_name, std::vector<component_type> plan_components = {})
		: name_(std::move(plan_name)), components_(std::move(plan_components))
	{
	}

	auto name() const -> const std::string & { return name_; }
	auto components() const -> const std::vector<component_type> & { return components_; }
	auto component_count() const -> std::size_t { return components_.size(); }
	auto empty() const -> bool { return components_.empty(); }

	auto add_component(component_type component) -> void { components_.push_back(std::move(component)); }

	auto has_component(const std::string &role, const std::string &name) const -> bool
	{
		return find_component(role, name) != components_.end();
	}

	auto has_component_parameter(const std::string &role, const std::string &name, const std::string &key,
								 const std::string &value) const -> bool
	{
		const auto found = find_component(role, name);
		return found != components_.end() && found->has_parameter(key, value);
	}

	auto replaced_component(const std::string &role, const std::string &name, component_type replacement) const
		-> PipelinePlan
	{
		auto copy = *this;
		const auto found = copy.find_component(role, name);
		if (found == copy.components_.end()) {
			throw PipelineValidationError("pipeline component to replace was not found");
		}
		*found = std::move(replacement);
		return copy;
	}

	auto validate_required_components(const std::vector<std::pair<std::string, std::string>> &required) const -> void
	{
		for (const auto &component : required) {
			if (!has_component(component.first, component.second)) {
				throw PipelineValidationError("pipeline plan is missing a required component");
			}
		}
	}

  private:
	auto find_component(const std::string &role, const std::string &name) -> std::vector<component_type>::iterator
	{
		return std::find_if(components_.begin(), components_.end(),
							[&](const auto &component) { return component.role == role && component.name == name; });
	}

	auto find_component(const std::string &role, const std::string &name) const
		-> std::vector<component_type>::const_iterator
	{
		return std::find_if(components_.begin(), components_.end(),
							[&](const auto &component) { return component.role == role && component.name == name; });
	}

	std::string name_;
	std::vector<component_type> components_;
};

inline auto pipeline_component_to_json(const PipelineComponent &component) -> mtrc::core::Metadata
{
	mtrc::core::Metadata result = {{"role", component.role}, {"name", component.name}};
	if (!component.parameters.empty()) {
		result["parameters"] = component.parameters;
	}
	if (component.contributes_to_artifact) {
		result["artifact"] = {{"contributes", true}};
	}
	return result;
}

inline auto pipeline_components_to_json(const std::vector<PipelineComponent> &components) -> mtrc::core::Metadata
{
	auto result = mtrc::core::Metadata::array();
	for (const auto &component : components) {
		result.push_back(pipeline_component_to_json(component));
	}
	return result;
}

inline auto pipeline_plan_to_json(const PipelinePlan &plan) -> mtrc::core::Metadata
{
	return {{"name", plan.name()}, {"components", pipeline_components_to_json(plan.components())}};
}

inline auto pipeline_component_from_json(const mtrc::core::Metadata &value) -> PipelineComponent
{
	PipelineComponent component;
	component.role = value.at("role").template get<std::string>();
	component.name = value.at("name").template get<std::string>();
	if (value.contains("parameters") && !value.at("parameters").is_null()) {
		component.parameters = value.at("parameters").template get<std::map<std::string, std::string>>();
	}
	if (value.contains("artifact") && value.at("artifact").is_object()) {
		component.contributes_to_artifact = value.at("artifact").value("contributes", false);
	}
	return component;
}

inline auto pipeline_components_from_json(const mtrc::core::Metadata &values) -> std::vector<PipelineComponent>
{
	std::vector<PipelineComponent> components;
	components.reserve(values.size());
	for (const auto &value : values) {
		components.push_back(pipeline_component_from_json(value));
	}
	return components;
}

inline auto pipeline_plan_from_json(const mtrc::core::Metadata &value) -> PipelinePlan
{
	return PipelinePlan(value.at("name").template get<std::string>(),
						pipeline_components_from_json(value.at("components")));
}

} // namespace mtrc::modify::compose

#endif
