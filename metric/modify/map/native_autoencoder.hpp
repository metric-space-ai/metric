// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_NATIVE_AUTOENCODER_HPP
#define _METRIC_MAPPINGS_NATIVE_AUTOENCODER_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/numeric/Math.h>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/record/id.hpp>
#include <metric/core/result.hpp>
#include <metric/metric/catalog/vector/Standards.hpp>
#include <metric/modify/compose/pipeline.hpp>
#include <metric/solve/parametric/dnn.hpp>

namespace mtrc::modify::map {

template <typename Record, typename Scalar, typename Codec = solve::parametric::dnn::VectorRecordCodec<Record, Scalar>>
class NativeAutoencoderModel {
  public:
	using record_type = Record;
	using scalar_type = Scalar;
	using latent_record_type = std::vector<scalar_type>;
	using latent_metric_type = Euclidean<scalar_type>;
	using space_type = MetricSpace<latent_record_type, latent_metric_type>;
	using result_type = MappingResult<space_type>;
	using codec_type = Codec;
	using matrix_type = mtrc::numeric::DynamicMatrix<scalar_type>;

	NativeAutoencoderModel(solve::parametric::dnn::AutoencoderModel<scalar_type> model, codec_type codec, std::size_t source_record_count,
						   std::string mapping_name = "native_autoencoder", std::string strategy_name = "native_dnn",
						   modify::compose::PipelinePlan pipeline_plan = modify::compose::PipelinePlan{})
		: model_(std::move(model)), codec_(std::move(codec)), source_record_count_(source_record_count),
		  mapping_name_(std::move(mapping_name)), strategy_name_(std::move(strategy_name)),
		  pipeline_plan_(std::move(pipeline_plan))
	{
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto transform(const Space &space) const -> result_type
	{
		static_assert(std::is_same<typename Space::record_type, record_type>::value,
					  "NativeAutoencoderModel can only transform spaces with the fitted record type");
		if (space.empty()) {
			throw std::invalid_argument("cannot transform an empty space with a native autoencoder");
		}

		const auto features = codec_.encode_batch(space.records());
		const auto latent = model_.encode(features);
		auto latent_records = numeric::matrix_to_row_vectors(latent);
		space_type derived_space(std::move(latent_records), latent_metric_type{});
		auto lineage = mtrc::one_to_one_lineage(space);

		// The latent space is a derived Euclidean coordinate space (a true metric
		// on the codes). The fitted network transforms out-of-sample records whose
		// codec shape matches the fitted input. The coordinates approximate, but do
		// not reproduce, the source geometry, so callers must not treat the latent
		// metric as isometric to the source metric.
		return core::make_mapping_result(
			std::move(derived_space), std::move(lineage.source_records),
			std::move(lineage.representative_records), space.size(), codec_.inverse_supported(), mapping_name_,
			strategy_name_, "metric_space", core::metric_traits<latent_metric_type>::law, true,
			"autoencoder latent coordinates; derived Euclidean coordinate space approximating source geometry; "
			"valid for records matching the fitted codec input shape");
	}

	auto inverse_transform(const space_type &latent_space) const -> std::vector<record_type>
	{
		return inverse_transform(latent_space.records());
	}

	auto inverse_transform(const result_type &result) const -> std::vector<record_type>
	{
		return inverse_transform(result.space);
	}

	auto inverse_transform(const std::vector<latent_record_type> &latent_records) const -> std::vector<record_type>
	{
		const auto latent = numeric::row_vectors_to_matrix<scalar_type>(latent_records);
		const auto reconstructed = model_.decode(latent);
		return codec_.decode_batch(reconstructed);
	}

	auto training_report() const -> const solve::parametric::dnn::TrainingReport<scalar_type> & { return model_.training_report(); }
	auto source_record_count() const -> std::size_t { return source_record_count_; }
	auto latent_dimension() const -> std::size_t { return model_.latent_dimension(); }
	auto native_model() const -> const solve::parametric::dnn::AutoencoderModel<scalar_type> & { return model_; }
	auto codec() const -> const codec_type & { return codec_; }
	auto mapping_name() const -> const std::string & { return mapping_name_; }
	auto strategy_name() const -> const std::string & { return strategy_name_; }
	auto has_pipeline_plan() const -> bool { return !pipeline_plan_.name().empty(); }
	auto pipeline_plan() const -> const modify::compose::PipelinePlan & { return pipeline_plan_; }

  private:
	mutable solve::parametric::dnn::AutoencoderModel<scalar_type> model_;
	codec_type codec_;
	std::size_t source_record_count_{};
	std::string mapping_name_;
	std::string strategy_name_;
	modify::compose::PipelinePlan pipeline_plan_;
};

template <typename Scalar> class NativeAutoencoderMapping {
  public:
	using scalar_type = Scalar;

	NativeAutoencoderMapping(solve::parametric::dnn::AutoencoderModel<scalar_type> prototype, solve::parametric::dnn::CompositeLoss<scalar_type> objective,
							 solve::parametric::dnn::TrainingSpec<scalar_type> spec = {})
		: prototype_(std::move(prototype)), objective_(std::move(objective)), spec_(std::move(spec))
	{
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto fit(const Space &space) const -> NativeAutoencoderModel<typename Space::record_type, scalar_type>
	{
		if (space.empty()) {
			throw std::invalid_argument("cannot fit native autoencoder mapping on an empty space");
		}

		using record_type = typename Space::record_type;
		using model_type = NativeAutoencoderModel<record_type, scalar_type>;
		using codec_type = typename model_type::codec_type;

		codec_type codec(space.records().front().size());
		auto features = codec.encode_batch(space.records());
		auto ids = mtrc::record_ids(space);

		solve::parametric::dnn::EncodedDataset<scalar_type> dataset(std::move(ids), std::move(features), space.version());
		auto model = prototype_.clone();
		const solve::parametric::dnn::NativeDnnTrainer<scalar_type> trainer;
		trainer.fit(model, dataset, objective_, spec_);

		return model_type(std::move(model), std::move(codec), space.size());
	}

  private:
	solve::parametric::dnn::AutoencoderModel<scalar_type> prototype_;
	solve::parametric::dnn::CompositeLoss<scalar_type> objective_;
	solve::parametric::dnn::TrainingSpec<scalar_type> spec_;
};

template <typename Scalar>
auto native_autoencoder(solve::parametric::dnn::AutoencoderModel<Scalar> model, solve::parametric::dnn::CompositeLoss<Scalar> objective,
						solve::parametric::dnn::TrainingSpec<Scalar> spec = {}) -> NativeAutoencoderMapping<Scalar>
{
	return NativeAutoencoderMapping<Scalar>(std::move(model), std::move(objective), std::move(spec));
}

} // namespace mtrc::modify::map

#endif
