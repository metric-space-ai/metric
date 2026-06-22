#ifndef UTILS_RANDOM_H_
#define UTILS_RANDOM_H_

#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

#include <metric/numeric/Math.h>

namespace mtrc::solve::parametric::dnn {

namespace internal {

template <typename DerivedX, typename DerivedY, typename XType, typename YType>
int create_shuffled_batches(const DerivedX &x, const DerivedY &y, int batchSize, std::mt19937 &rng,
							std::vector<XType> &xBatches, std::vector<YType> &yBatches)
{
	const int observationsNumber = x.rows();
	const int dimX = x.columns();
	const int dimY = y.columns();

	if (batchSize <= 0) {
		throw std::invalid_argument("DNN mini-batch size must be positive");
	}
	if (y.rows() != observationsNumber) {
		throw std::invalid_argument("Input X and Y have different number of observations");
	}
	if (observationsNumber == 0) {
		xBatches.clear();
		yBatches.clear();
		return 0;
	}

	// Randomly shuffle the IDs
	std::vector<int> id(observationsNumber);
	std::iota(id.begin(), id.end(), 0);
	std::shuffle(id.begin(), id.end(), rng);

	// Compute batch size
	if (batchSize > observationsNumber) {
		batchSize = observationsNumber;
	}

	const int batchesNumber = (observationsNumber - 1) / batchSize + 1;
	const int lastBatchSize = observationsNumber - (batchesNumber - 1) * batchSize;

	// Create shuffled data
	xBatches.clear();
	yBatches.clear();
	xBatches.reserve(batchesNumber);
	yBatches.reserve(batchesNumber);

	for (int i = 0; i < batchesNumber; i++) {
		const int currentBatchSize = (i == batchesNumber - 1) ? lastBatchSize : batchSize;
		const int offset = i * batchSize;

		xBatches.push_back(mtrc::numeric::select_rows_as<XType>(x, id, static_cast<std::size_t>(offset),
																  static_cast<std::size_t>(currentBatchSize)));
		yBatches.push_back(mtrc::numeric::select_rows_as<YType>(y, id, static_cast<std::size_t>(offset),
																  static_cast<std::size_t>(currentBatchSize)));
	}

	return batchesNumber;
}

// Fill array with N(mu, sigma^2) random numbers
template <typename Scalar>
inline void set_normal_random(Scalar *arr, const int n, std::mt19937 &rng, const Scalar &mu = Scalar(0),
							  const Scalar &sigma = Scalar(1))
{
	std::normal_distribution<Scalar> normalDistribution(mu, sigma);
	for (auto i = 0; i < n; ++i) {
		arr[i] = normalDistribution(rng);
	}
}

} // namespace internal

} // namespace mtrc::solve::parametric::dnn

#endif /* UTILS_RANDOM_H_ */
