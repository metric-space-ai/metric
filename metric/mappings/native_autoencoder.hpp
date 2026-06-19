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

#include "../core/concepts.hpp"
#include "../core/metric_space.hpp"
#include "../core/record_id.hpp"
#include "../core/result.hpp"
#include "../distance/k-related/Standards.hpp"
#include "../utils/dnn.hpp"

namespace metric::mappings {

template <typename Record, typename Scalar = typename Record::value_type> class NativeAutoencoderModel {
  public:
	using record_type = Record;
	using scalar_type = Scalar;
	using latent_record_type = std::vector<scalar_type>;
	using latent_metric_type = Euclidean<scalar_type>;
	using space_type = MetricSpace<latent_record_type, latent_metric_type>;
	using result_type = MappingResult<space_type>;
	using codec_type = dnn::VectorRecordCodec<record_type, scalar_type>;
	using matrix_type = blaze::DynamicMatrix<scalar_type>;

	NativeAutoencoderModel(dnn::AutoencoderModel<scalar_type> model, codec_type codec,
						   std::size_t source_record_count)
		: model_(std::move(model))
		, codec_(std::move(codec))
		, source_record_count_(source_record_count)
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
		auto latent_records = matrix_to_records(latent);
		space_type derived_space(std::move(latent_records), latent_metric_type{});
		auto lineage = singleton_lineage(space);

		result_type result{std::move(derived_space), lineage.first, lineage.second, space.size(),
						   codec_.inverse_supported(), "native_autoencoder", model_.backend_name(), "metric_space"};
		return result;
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
		const auto latent = records_to_matrix(latent_records);
		const auto reconstructed = model_.decode(latent);
		return codec_.decode_batch(reconstructed);
	}

	auto training_report() const -> const dnn::TrainingReport<scalar_type> & { return model_.training_report(); }
	auto source_record_count() const -> std::size_t { return source_record_count_; }
	auto latent_dimension() const -> std::size_t { return model_.latent_dimension(); }
	auto native_model() const -> const dnn::AutoencoderModel<scalar_type> & { return model_; }

  private:
	template <typename Space>
	static auto singleton_lineage(const Space &space)
		-> std::pair<std::vector<std::vector<RecordId>>, std::vector<RecordId>>
	{
		std::vector<std::vector<RecordId>> source_records;
		std::vector<RecordId> representative_records;
		source_records.reserve(space.size());
		representative_records.reserve(space.size());

		for (std::size_t index = 0; index < space.size(); ++index) {
			const auto id = space.id(index);
			source_records.push_back(std::vector<RecordId>{id});
			representative_records.push_back(id);
		}

		return {std::move(source_records), std::move(representative_records)};
	}

	static auto matrix_to_records(const matrix_type &matrix) -> std::vector<latent_record_type>
	{
		std::vector<latent_record_type> records;
		records.reserve(matrix.rows());
		for (std::size_t row = 0; row < matrix.rows(); ++row) {
			latent_record_type record;
			record.reserve(matrix.columns());
			for (std::size_t column = 0; column < matrix.columns(); ++column) {
				record.push_back(matrix(row, column));
			}
			records.push_back(std::move(record));
		}
		return records;
	}

	static auto records_to_matrix(const std::vector<latent_record_type> &records) -> matrix_type
	{
		if (records.empty()) {
			return matrix_type(0, 0);
		}

		const auto columns = records.front().size();
		matrix_type matrix(records.size(), columns);
		for (std::size_t row = 0; row < records.size(); ++row) {
			if (records[row].size() != columns) {
				throw std::invalid_argument("latent records must have a consistent feature count");
			}
			for (std::size_t column = 0; column < columns; ++column) {
				matrix(row, column) = records[row][column];
			}
		}
		return matrix;
	}

	mutable dnn::AutoencoderModel<scalar_type> model_;
	codec_type codec_;
	std::size_t source_record_count_{};
};

template <typename Scalar> class NativeAutoencoderMapping {
  public:
	using scalar_type = Scalar;

	NativeAutoencoderMapping(dnn::AutoencoderModel<scalar_type> prototype, dnn::CompositeLoss<scalar_type> objective,
							 dnn::TrainingSpec<scalar_type> spec = {})
		: prototype_(std::move(prototype))
		, objective_(std::move(objective))
		, spec_(std::move(spec))
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
		std::vector<RecordId> ids;
		ids.reserve(space.size());
		for (std::size_t index = 0; index < space.size(); ++index) {
			ids.push_back(space.id(index));
		}

		dnn::EncodedDataset<scalar_type> dataset(std::move(ids), std::move(features), space.version());
		auto model = prototype_.clone();
		const dnn::NativeDnnTrainer<scalar_type> trainer;
		trainer.fit(model, dataset, objective_, spec_);

		return model_type(std::move(model), std::move(codec), space.size());
	}

  private:
	dnn::AutoencoderModel<scalar_type> prototype_;
	dnn::CompositeLoss<scalar_type> objective_;
	dnn::TrainingSpec<scalar_type> spec_;
};

template <typename Scalar>
auto native_autoencoder(dnn::AutoencoderModel<Scalar> model, dnn::CompositeLoss<Scalar> objective,
						dnn::TrainingSpec<Scalar> spec = {}) -> NativeAutoencoderMapping<Scalar>
{
	return NativeAutoencoderMapping<Scalar>(std::move(model), std::move(objective), std::move(spec));
}

} // namespace metric::mappings

#endif
