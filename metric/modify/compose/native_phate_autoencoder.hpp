// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_PIPELINES_NATIVE_PHATE_AUTOENCODER_HPP
#define _METRIC_PIPELINES_NATIVE_PHATE_AUTOENCODER_HPP

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/modify/map/native_phate_autoencoder.hpp>
#include <metric/solve/parametric/dnn.hpp>
#include "native_phate_autoencoder_plan.hpp"

namespace mtrc::modify::compose {

template <typename Scalar> class NativePhateAutoencoderPipeline {
  public:
	using scalar_type = Scalar;
	using component_type = PipelineComponent;

	NativePhateAutoencoderPipeline(solve::parametric::dnn::AutoencoderModel<scalar_type> prototype,
								   modify::map::PhateGeometrySpec<scalar_type> geometry_spec,
								   solve::parametric::dnn::TrainingSpec<scalar_type> training_spec = {},
								   scalar_type reconstruction_weight = scalar_type(1),
								   scalar_type geometry_weight = scalar_type(1))
		: NativePhateAutoencoderPipeline(std::move(prototype), std::move(geometry_spec), std::move(training_spec),
										 native_phate_autoencoder_pipeline_plan(reconstruction_weight, geometry_weight))
	{
	}

	NativePhateAutoencoderPipeline(solve::parametric::dnn::AutoencoderModel<scalar_type> prototype,
								   modify::map::PhateGeometrySpec<scalar_type> geometry_spec,
								   solve::parametric::dnn::TrainingSpec<scalar_type> training_spec, PipelinePlan plan)
		: prototype_(std::move(prototype)), geometry_spec_(std::move(geometry_spec)),
		  training_spec_(std::move(training_spec)), plan_(std::move(plan))
	{
		const auto hooks = resolve_native_phate_autoencoder_executable_hooks<scalar_type>(plan_);
		hooks.validate();
		codec_ = hooks.codec;
		pairwise_distances_ = hooks.pairwise_distances;
		affinity_kernel_ = hooks.affinity_kernel;
		diffusion_operator_ = hooks.diffusion_operator;
		reconstruction_weight_ = hooks.reconstruction_weight;
		geometry_weight_ = hooks.geometry_weight;
		validate();
	}

	auto name() const -> const std::string & { return plan_.name(); }

	auto plan() const -> const PipelinePlan & { return plan_; }
	auto components() const -> const std::vector<component_type> & { return plan_.components(); }
	auto component_count() const -> std::size_t { return plan_.component_count(); }
	auto codec() const -> const std::string & { return codec_; }
	auto pairwise_distances() const -> const std::string & { return pairwise_distances_; }
	auto affinity_kernel() const -> const std::string & { return affinity_kernel_; }
	auto diffusion_operator() const -> const std::string & { return diffusion_operator_; }

	auto has_component(const std::string &role, const std::string &name) const -> bool
	{
		return plan_.has_component(role, name);
	}

	auto has_component_parameter(const std::string &role, const std::string &name, const std::string &key,
								 const std::string &value) const -> bool
	{
		return plan_.has_component_parameter(role, name, key, value);
	}

	auto validate() const -> void
	{
		(void)modify::map::NativePhateAutoencoderMapping<scalar_type>(prototype_.clone(), geometry_spec_, training_spec_,
																   plan_);
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto fit(const Space &space) const
	{
		auto mapping = modify::map::native_phate_autoencoder(prototype_.clone(), geometry_spec_, training_spec_, plan_);
		return mapping.fit(space);
	}

  private:
	solve::parametric::dnn::AutoencoderModel<scalar_type> prototype_;
	modify::map::PhateGeometrySpec<scalar_type> geometry_spec_;
	solve::parametric::dnn::TrainingSpec<scalar_type> training_spec_;
	std::string codec_{"vector_record_codec"};
	std::string pairwise_distances_{"exact_space_distances"};
	std::string affinity_kernel_{"gaussian_affinity_kernel"};
	std::string diffusion_operator_{"row_normalized_diffusion_operator"};
	scalar_type reconstruction_weight_{1};
	scalar_type geometry_weight_{1};
	PipelinePlan plan_;
};

template <typename Scalar>
auto native_phate_autoencoder(solve::parametric::dnn::AutoencoderModel<Scalar> model, modify::map::PhateGeometrySpec<Scalar> geometry_spec,
							  solve::parametric::dnn::TrainingSpec<Scalar> training_spec = {}, Scalar reconstruction_weight = Scalar(1),
							  Scalar geometry_weight = Scalar(1)) -> NativePhateAutoencoderPipeline<Scalar>
{
	return NativePhateAutoencoderPipeline<Scalar>(std::move(model), std::move(geometry_spec), std::move(training_spec),
												  reconstruction_weight, geometry_weight);
}

template <typename Scalar>
auto native_phate_autoencoder(const NativePhateAutoencoderPipelinePlanBuilder<Scalar> &builder,
							  solve::parametric::dnn::AutoencoderModel<Scalar> model, modify::map::PhateGeometrySpec<Scalar> geometry_spec,
							  solve::parametric::dnn::TrainingSpec<Scalar> training_spec = {}) -> NativePhateAutoencoderPipeline<Scalar>
{
	return NativePhateAutoencoderPipeline<Scalar>(std::move(model), std::move(geometry_spec), std::move(training_spec),
												  builder.plan());
}

template <typename Scalar>
auto native_phate_autoencoder(PipelinePlan plan, solve::parametric::dnn::AutoencoderModel<Scalar> model,
							  modify::map::PhateGeometrySpec<Scalar> geometry_spec,
							  solve::parametric::dnn::TrainingSpec<Scalar> training_spec = {}) -> NativePhateAutoencoderPipeline<Scalar>
{
	return NativePhateAutoencoderPipeline<Scalar>(std::move(model), std::move(geometry_spec), std::move(training_spec),
												  std::move(plan));
}

} // namespace mtrc::modify::compose

#endif
