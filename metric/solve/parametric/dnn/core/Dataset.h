#ifndef DATASET_H_
#define DATASET_H_

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/numeric/Math.h>
#include <metric/core/metadata.hpp>

#include "metric/record/id.hpp"

namespace mtrc::solve::parametric::dnn {

using SampleId = mtrc::RecordId;

template <class Scalar> struct DnnBatch {
	std::vector<SampleId> ids;
	mtrc::numeric::DynamicMatrix<Scalar> x;
};

template <class Scalar> class EncodedDataset {
  public:
	using matrix_type = mtrc::numeric::DynamicMatrix<Scalar>;

	EncodedDataset() = default;

	EncodedDataset(std::vector<SampleId> ids, matrix_type features, std::uint64_t source_space_version = 0)
		: ids_(std::move(ids)), features_(std::move(features)), source_space_version_(source_space_version)
	{
		if (ids_.size() != features_.rows()) {
			throw std::invalid_argument("EncodedDataset IDs must match feature rows");
		}
	}

	static auto from_features(matrix_type features, std::uint64_t source_space_version = 0) -> EncodedDataset
	{
		std::vector<SampleId> ids;
		ids.reserve(features.rows());
		for (std::size_t index = 0; index < features.rows(); ++index) {
			ids.push_back(SampleId::from_index(index));
		}
		return EncodedDataset(std::move(ids), std::move(features), source_space_version);
	}

	auto size() const -> std::size_t { return ids_.size(); }
	auto feature_count() const -> std::size_t { return features_.columns(); }
	auto ids() const -> const std::vector<SampleId> & { return ids_; }
	auto features() const -> const matrix_type & { return features_; }
	auto source_space_version() const -> std::uint64_t { return source_space_version_; }

	auto batches(std::size_t batch_size, std::mt19937 &rng, bool shuffle = true) const -> std::vector<DnnBatch<Scalar>>
	{
		if (batch_size == 0) {
			throw std::invalid_argument("DNN batch size must be positive");
		}
		if (size() == 0) {
			return {};
		}

		std::vector<std::size_t> order(size());
		std::iota(order.begin(), order.end(), std::size_t{0});
		if (shuffle) {
			std::shuffle(order.begin(), order.end(), rng);
		}

		const auto batches_count = (size() + batch_size - 1) / batch_size;
		std::vector<DnnBatch<Scalar>> result;
		result.reserve(batches_count);

		for (std::size_t batch_index = 0; batch_index < batches_count; ++batch_index) {
			const auto offset = batch_index * batch_size;
			const auto current_size = std::min(batch_size, size() - offset);
			DnnBatch<Scalar> batch;
			batch.ids.reserve(current_size);
			batch.x = mtrc::numeric::select_rows(features_, order, offset, current_size);

			for (std::size_t row = 0; row < current_size; ++row) {
				const auto source_row = order[offset + row];
				batch.ids.push_back(ids_[source_row]);
			}

			result.push_back(std::move(batch));
		}

		return result;
	}

  private:
	std::vector<SampleId> ids_;
	matrix_type features_;
	std::uint64_t source_space_version_{0};
};

template <class InputDataType, class Scalar> class FlatVectorCodec {
  public:
	using matrix_type = mtrc::numeric::DynamicMatrix<Scalar>;

	explicit FlatVectorCodec(std::size_t coordinate_count, InputDataType norm_value = InputDataType{})
		: coordinate_count_(coordinate_count), norm_value_(norm_value)
	{
		if (coordinate_count_ == 0) {
			throw std::invalid_argument("FlatVectorCodec coordinate count must be positive");
		}
	}

	auto coordinate_count() const -> std::size_t { return coordinate_count_; }
	auto norm_value() const -> InputDataType { return norm_value_; }
	auto to_json() const -> mtrc::core::Metadata
	{
		return {{"type", "FlatVectorCodec"}, {"coordinate_count", coordinate_count_}, {"norm_value", norm_value_}};
	}

	auto encode_flat(const std::vector<InputDataType> &input) const -> matrix_type
	{
		auto encoded = mtrc::numeric::flat_values_to_matrix<Scalar>(input, coordinate_count_);
		if (norm_value_ != InputDataType{}) {
			encoded /= Scalar(norm_value_);
		}

		return encoded;
	}

	auto decode_flat(const matrix_type &matrix, bool denormalize = true) const -> std::vector<InputDataType>
	{
		if (matrix.columns() != coordinate_count_) {
			throw std::invalid_argument("Decoded matrix coordinate count does not match codec");
		}

		matrix_type decoded(matrix);
		if (denormalize && norm_value_ != InputDataType{}) {
			decoded *= Scalar(norm_value_);
		}

		return mtrc::numeric::matrix_to_flat_values_as<InputDataType>(decoded);
	}

  private:
	std::size_t coordinate_count_{0};
	InputDataType norm_value_{};
};

template <class Record, class Scalar> class VectorRecordCodec {
  public:
	using matrix_type = mtrc::numeric::DynamicMatrix<Scalar>;

	explicit VectorRecordCodec(std::size_t coordinate_count) : coordinate_count_(coordinate_count)
	{
		if (coordinate_count_ == 0) {
			throw std::invalid_argument("VectorRecordCodec coordinate count must be positive");
		}
	}

	auto coordinate_count() const -> std::size_t { return coordinate_count_; }
	auto inverse_supported() const -> bool { return true; }
	auto to_json() const -> mtrc::core::Metadata
	{
		return {{"type", "VectorRecordCodec"}, {"coordinate_count", coordinate_count_}, {"inverse_supported", true}};
	}

	auto encode_batch(const std::vector<Record> &records) const -> matrix_type
	{
		return mtrc::numeric::row_vectors_to_matrix<Scalar>(records, coordinate_count_);
	}

	auto decode_batch(const matrix_type &matrix) const -> std::vector<Record>
	{
		if (matrix.columns() != coordinate_count_) {
			throw std::invalid_argument("VectorRecordCodec decoded coordinate count does not match codec");
		}

		return mtrc::numeric::matrix_to_row_vectors_as<Record>(matrix);
	}

  private:
	std::size_t coordinate_count_{0};
};

template <class Record, class Scalar, class Encoder> class RecordCoordinateCodec {
  public:
	using matrix_type = mtrc::numeric::DynamicMatrix<Scalar>;

	RecordCoordinateCodec(std::size_t coordinate_count, Encoder encoder, std::string codec_name = "record_coordinate_codec")
		: coordinate_count_(coordinate_count), encoder_(std::move(encoder)), codec_name_(std::move(codec_name))
	{
		if (coordinate_count_ == 0) {
			throw std::invalid_argument("RecordCoordinateCodec coordinate count must be positive");
		}
	}

	auto coordinate_count() const -> std::size_t { return coordinate_count_; }
	auto inverse_supported() const -> bool { return false; }
	auto to_json() const -> mtrc::core::Metadata
	{
		return {{"type", "RecordCoordinateCodec"},
				{"name", codec_name_},
				{"coordinate_count", coordinate_count_},
				{"inverse_supported", false}};
	}

	auto encode_batch(const std::vector<Record> &records) const -> matrix_type
	{
		using encoded_row_type = decltype(std::declval<Encoder>()(std::declval<Record>()));

		std::vector<encoded_row_type> rows;
		rows.reserve(records.size());
		for (const auto &record : records) {
			auto coordinates = encoder_(record);
			if (coordinates.size() != coordinate_count_) {
				throw std::invalid_argument("RecordCoordinateCodec encoded coordinate count does not match codec");
			}
			rows.push_back(std::move(coordinates));
		}
		return mtrc::numeric::row_vectors_to_matrix<Scalar>(rows, coordinate_count_);
	}

	auto decode_batch(const matrix_type &) const -> std::vector<Record>
	{
		throw std::invalid_argument("RecordCoordinateCodec does not support inverse decode");
	}

  private:
	std::size_t coordinate_count_{0};
	Encoder encoder_;
	std::string codec_name_;
};

template <class Record, class Scalar, class Encoder>
auto make_record_coordinate_codec(std::size_t coordinate_count, Encoder encoder,
							   std::string codec_name = "record_coordinate_codec")
	-> RecordCoordinateCodec<Record, Scalar, Encoder>
{
	return RecordCoordinateCodec<Record, Scalar, Encoder>(coordinate_count, std::move(encoder), std::move(codec_name));
}

} // namespace mtrc::solve::parametric::dnn

#endif /* DATASET_H_ */
