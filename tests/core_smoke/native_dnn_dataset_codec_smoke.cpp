#include <algorithm>
#include <cassert>
#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

#include <blaze/Math.h>

#include "metric/utils/dnn.hpp"

namespace {

using Matrix = blaze::DynamicMatrix<double>;

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool
{
	return std::abs(lhs - rhs) <= tolerance;
}

auto assert_same_shape(const Matrix &matrix, std::size_t rows, std::size_t columns) -> void
{
	assert(matrix.rows() == rows);
	assert(matrix.columns() == columns);
}

} // namespace

int main()
{
	{
		const metric::dnn::FlatVectorCodec<double, double> codec(2, 10.0);
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
		const auto dataset = metric::dnn::EncodedDataset<double>::from_features(features, 12);
		assert(dataset.size() == 2);
		assert(dataset.feature_count() == 2);
		assert(dataset.source_space_version() == 12);
		assert(dataset.ids()[0] == metric::dnn::SampleId::from_index(0));
		assert(dataset.ids()[1] == metric::dnn::SampleId::from_index(1));
	}

	{
		const std::vector<metric::dnn::SampleId> ids{metric::dnn::SampleId::from_index(3),
													 metric::dnn::SampleId::from_index(7),
													 metric::dnn::SampleId::from_index(11),
													 metric::dnn::SampleId::from_index(13)};
		const Matrix features{{30.0, 31.0}, {70.0, 71.0}, {110.0, 111.0}, {130.0, 131.0}};
		const metric::dnn::EncodedDataset<double> dataset(ids, features, 99);
		assert(dataset.size() == ids.size());
		assert(dataset.source_space_version() == 99);

		std::mt19937 rng(7);
		const auto ordered_batches = dataset.batches(3, rng, false);
		assert(ordered_batches.size() == 2);
		assert((ordered_batches[0].ids == std::vector<metric::dnn::SampleId>{ids[0], ids[1], ids[2]}));
		assert((ordered_batches[1].ids == std::vector<metric::dnn::SampleId>{ids[3]}));
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
