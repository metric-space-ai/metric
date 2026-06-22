#include <algorithm>
#include <cassert>
#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

#include <metric/numeric/Math.h>

#include "metric/solve/parametric/dnn.hpp"

namespace {

using Matrix = mtrc::numeric::DynamicMatrix<double>;

struct SmallRecord {
	double base{};
};

struct SmallFeatureEncoder {
	std::size_t feature_count{0};

	auto operator()(const SmallRecord &record) const -> std::vector<float>
	{
		std::vector<float> values;
		values.reserve(feature_count);
		for (std::size_t index = 0; index < feature_count; ++index) {
			values.push_back(static_cast<float>(record.base + static_cast<double>(index)));
		}
		return values;
	}
};

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool { return std::abs(lhs - rhs) <= tolerance; }

auto assert_same_shape(const Matrix &matrix, std::size_t rows, std::size_t columns) -> void
{
	assert(matrix.rows() == rows);
	assert(matrix.columns() == columns);
}

} // namespace

int main()
{
	{
		const mtrc::solve::parametric::dnn::FlatVectorCodec<double, double> codec(2, 10.0);
		const std::vector<double> flat{0.0, 10.0, 20.0, 30.0};
		const auto encoded = codec.encode_flat(flat);
		assert_same_shape(encoded, 2, 2);
		assert(close(encoded(0, 0), 0.0));
		assert(close(encoded(0, 1), 1.0));
		assert(close(encoded(1, 0), 2.0));
		assert(close(encoded(1, 1), 3.0));

		const auto denormalized = codec.decode_flat(encoded);
		assert(denormalized == flat);

		const auto raw = codec.decode_flat(encoded, false);
		assert((raw == std::vector<double>{0.0, 1.0, 2.0, 3.0}));

		const std::vector<double> empty_flat;
		const auto empty_encoded = codec.encode_flat(empty_flat);
		assert_same_shape(empty_encoded, 0, 2);
		assert(codec.decode_flat(empty_encoded).empty());

		bool rejected_bad_shape = false;
		try {
			(void)codec.encode_flat({1.0, 2.0, 3.0});
		} catch (const std::invalid_argument &) {
			rejected_bad_shape = true;
		}
		assert(rejected_bad_shape);
	}

	{
		const Matrix features{{3.0, 4.0}, {7.0, 8.0}};
		const auto dataset = mtrc::solve::parametric::dnn::EncodedDataset<double>::from_features(features, 12);
		assert(dataset.size() == 2);
		assert(dataset.feature_count() == 2);
		assert(dataset.source_space_version() == 12);
		assert(dataset.ids()[0] == mtrc::solve::parametric::dnn::SampleId::from_index(0));
		assert(dataset.ids()[1] == mtrc::solve::parametric::dnn::SampleId::from_index(1));
	}

	{
		const Matrix empty_features(0, 3);
		const auto dataset = mtrc::solve::parametric::dnn::EncodedDataset<double>::from_features(empty_features, 5);
		assert(dataset.size() == 0);
		assert(dataset.feature_count() == 3);
		std::mt19937 rng(3);
		assert(dataset.batches(2, rng, false).empty());
	}

	{
		using record_type = std::vector<float>;
		const mtrc::solve::parametric::dnn::VectorRecordCodec<record_type, double> codec(2);
		const std::vector<record_type> records{{1.0F, 2.0F}, {3.0F, 4.0F}};
		const auto encoded = codec.encode_batch(records);
		assert_same_shape(encoded, 2, 2);
		assert(close(encoded(0, 0), 1.0));
		assert(close(encoded(1, 1), 4.0));

		const std::vector<record_type> empty_records;
		const auto empty_encoded = codec.encode_batch(empty_records);
		assert_same_shape(empty_encoded, 0, 2);

		const Matrix decoded_input{{1.25, 2.5}, {3.75, 4.5}};
		const auto decoded = codec.decode_batch(decoded_input);
		assert(decoded.size() == 2);
		assert(decoded[0].size() == 2);
		assert(close(static_cast<double>(decoded[0][0]), 1.25));
		assert(close(static_cast<double>(decoded[1][1]), 4.5));

		bool rejected_bad_record_width = false;
		try {
			(void)codec.encode_batch(std::vector<record_type>{{1.0F}});
		} catch (const std::invalid_argument &) {
			rejected_bad_record_width = true;
		}
		assert(rejected_bad_record_width);
	}

	{
		const auto codec =
			mtrc::solve::parametric::dnn::make_feature_record_codec<SmallRecord, double>(3, SmallFeatureEncoder{3}, "small_features");
		const std::vector<SmallRecord> records{{1.0}, {4.0}};
		const auto encoded = codec.encode_batch(records);
		assert_same_shape(encoded, 2, 3);
		assert(close(encoded(0, 0), 1.0));
		assert(close(encoded(0, 2), 3.0));
		assert(close(encoded(1, 0), 4.0));
		assert(close(encoded(1, 2), 6.0));

		const std::vector<SmallRecord> empty_records;
		const auto empty_encoded = codec.encode_batch(empty_records);
		assert_same_shape(empty_encoded, 0, 3);
		assert(!codec.inverse_supported());

		const auto bad_codec =
			mtrc::solve::parametric::dnn::make_feature_record_codec<SmallRecord, double>(2, SmallFeatureEncoder{3}, "bad_features");
		bool rejected_bad_feature_width = false;
		try {
			(void)bad_codec.encode_batch(records);
		} catch (const std::invalid_argument &) {
			rejected_bad_feature_width = true;
		}
		assert(rejected_bad_feature_width);
	}

	{
		const std::vector<mtrc::solve::parametric::dnn::SampleId> ids{
			mtrc::solve::parametric::dnn::SampleId::from_index(3), mtrc::solve::parametric::dnn::SampleId::from_index(7),
			mtrc::solve::parametric::dnn::SampleId::from_index(11), mtrc::solve::parametric::dnn::SampleId::from_index(13)};
		const Matrix features{{30.0, 31.0}, {70.0, 71.0}, {110.0, 111.0}, {130.0, 131.0}};
		const mtrc::solve::parametric::dnn::EncodedDataset<double> dataset(ids, features, 99);
		assert(dataset.size() == ids.size());
		assert(dataset.source_space_version() == 99);

		std::mt19937 rng(7);
		const auto ordered_batches = dataset.batches(3, rng, false);
		assert(ordered_batches.size() == 2);
		assert((ordered_batches[0].ids == std::vector<mtrc::solve::parametric::dnn::SampleId>{ids[0], ids[1], ids[2]}));
		assert((ordered_batches[1].ids == std::vector<mtrc::solve::parametric::dnn::SampleId>{ids[3]}));
		assert_same_shape(ordered_batches[0].x, 3, 2);
		assert(close(ordered_batches[0].x(2, 0), 110.0));
		assert(close(ordered_batches[1].x(0, 1), 131.0));

		std::mt19937 shuffled_rng(11);
		const auto shuffled_batches = dataset.batches(2, shuffled_rng, true);
		std::vector<std::size_t> seen;
		for (const auto &batch : shuffled_batches) {
			assert(batch.ids.size() == batch.x.rows());
			for (std::size_t row = 0; row < batch.ids.size(); ++row) {
				const auto id_index = batch.ids[row].index();
				seen.push_back(id_index);
				assert(close(batch.x(row, 0), static_cast<double>(id_index * 10)));
				assert(close(batch.x(row, 1), static_cast<double>(id_index * 10 + 1)));
			}
		}
		std::sort(seen.begin(), seen.end());
		assert((seen == std::vector<std::size_t>{3, 7, 11, 13}));
	}

	return 0;
}
