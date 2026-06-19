#ifndef DATASET_H_
#define DATASET_H_

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <nlohmann/json.hpp>

#include "metric/core/record_id.hpp"

namespace metric::dnn {

using SampleId = metric::RecordId;

template <class Scalar> struct DnnBatch {
	std::vector<SampleId> ids;
	blaze::DynamicMatrix<Scalar> x;
};

template <class Scalar> class EncodedDataset {
  public:
	using matrix_type = blaze::DynamicMatrix<Scalar>;

	EncodedDataset() = default;

	EncodedDataset(std::vector<SampleId> ids, matrix_type features, std::uint64_t source_space_version = 0)
		: ids_(std::move(ids))
		, features_(std::move(features))
		, source_space_version_(source_space_version)
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
			batch.x.resize(current_size, feature_count());

			for (std::size_t row = 0; row < current_size; ++row) {
				const auto source_row = order[offset + row];
				batch.ids.push_back(ids_[source_row]);
				blaze::row(batch.x, row) = blaze::row(features_, source_row);
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
	using matrix_type = blaze::DynamicMatrix<Scalar>;

	explicit FlatVectorCodec(std::size_t feature_count, InputDataType norm_value = InputDataType{})
		: feature_count_(feature_count)
		, norm_value_(norm_value)
	{
		if (feature_count_ == 0) {
			throw std::invalid_argument("FlatVectorCodec feature count must be positive");
		}
	}

	auto feature_count() const -> std::size_t { return feature_count_; }
	auto norm_value() const -> InputDataType { return norm_value_; }
	auto to_json() const -> nlohmann::json
	{
		return {{"type", "FlatVectorCodec"}, {"feature_count", feature_count_}, {"norm_value", norm_value_}};
	}

	auto encode_flat(const std::vector<InputDataType> &input) const -> matrix_type
	{
		if (input.size() % feature_count_ != 0) {
			throw std::invalid_argument("Flat vector size must be divisible by feature count");
		}

		matrix_type encoded(input.size() / feature_count_, feature_count_);
		for (std::size_t row = 0; row < encoded.rows(); ++row) {
			for (std::size_t column = 0; column < encoded.columns(); ++column) {
				encoded(row, column) = static_cast<Scalar>(input[row * feature_count_ + column]);
			}
		}

		if (norm_value_ != InputDataType{}) {
			encoded /= Scalar(norm_value_);
		}

		return encoded;
	}

	auto decode_flat(const matrix_type &matrix, bool denormalize = true) const -> std::vector<InputDataType>
	{
		if (matrix.columns() != feature_count_) {
			throw std::invalid_argument("Decoded matrix feature count does not match codec");
		}

		matrix_type decoded(matrix);
		if (denormalize && norm_value_ != InputDataType{}) {
			decoded *= Scalar(norm_value_);
		}

		std::vector<InputDataType> output;
		output.reserve(decoded.rows() * decoded.columns());
		for (std::size_t row = 0; row < decoded.rows(); ++row) {
			for (std::size_t column = 0; column < decoded.columns(); ++column) {
				output.push_back(static_cast<InputDataType>(decoded(row, column)));
			}
		}

		return output;
	}

  private:
	std::size_t feature_count_{0};
	InputDataType norm_value_{};
};

template <class Record, class Scalar> class VectorRecordCodec {
  public:
	using matrix_type = blaze::DynamicMatrix<Scalar>;

	explicit VectorRecordCodec(std::size_t feature_count)
		: feature_count_(feature_count)
	{
		if (feature_count_ == 0) {
			throw std::invalid_argument("VectorRecordCodec feature count must be positive");
		}
	}

	auto feature_count() const -> std::size_t { return feature_count_; }
	auto inverse_supported() const -> bool { return true; }
	auto to_json() const -> nlohmann::json
	{
		return {{"type", "VectorRecordCodec"}, {"feature_count", feature_count_}, {"inverse_supported", true}};
	}

	auto encode_batch(const std::vector<Record> &records) const -> matrix_type
	{
		matrix_type encoded(records.size(), feature_count_);
		for (std::size_t row = 0; row < records.size(); ++row) {
			if (records[row].size() != feature_count_) {
				throw std::invalid_argument("VectorRecordCodec record feature count does not match codec");
			}
			for (std::size_t column = 0; column < feature_count_; ++column) {
				encoded(row, column) = static_cast<Scalar>(records[row][column]);
			}
		}
		return encoded;
	}

	auto decode_batch(const matrix_type &matrix) const -> std::vector<Record>
	{
		if (matrix.columns() != feature_count_) {
			throw std::invalid_argument("VectorRecordCodec decoded feature count does not match codec");
		}

		std::vector<Record> records;
		records.reserve(matrix.rows());
		for (std::size_t row = 0; row < matrix.rows(); ++row) {
			Record record;
			record.reserve(feature_count_);
			for (std::size_t column = 0; column < feature_count_; ++column) {
				record.push_back(static_cast<typename Record::value_type>(matrix(row, column)));
			}
			records.push_back(std::move(record));
		}
		return records;
	}

  private:
	std::size_t feature_count_{0};
};

} // namespace metric::dnn

#endif /* DATASET_H_ */
