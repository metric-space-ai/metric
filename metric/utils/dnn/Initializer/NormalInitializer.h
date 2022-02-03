#ifndef NORMALINITIALIZER_H
#define NORMALINITIALIZER_H

#include "../Initializer.h"

#include <random>

namespace metric::dnn {
template <typename Scalar> class NormalInitializer : public Initializer<Scalar> {
  protected:
	using typename Initializer<Scalar>::ColumnMatrix;
	using typename Initializer<Scalar>::RowVector;

	std::normal_distribution<Scalar> normalDistribution;
	std::mt19937 randomEngine;

  public:
	NormalInitializer(const Scalar mu, const Scalar sigma, const std::mt19937 &randomEngine)
		: randomEngine(randomEngine), normalDistribution(mu, sigma)
	{
	}

	void init(const size_t rows, const size_t columns, ColumnMatrix &matrix)
	{
		matrix = blaze::generate<blaze::columnMajor>(
			rows, columns, [this](size_t i, size_t j) { return normalDistribution(randomEngine); });
	}

	void init(const size_t size, RowVector &vector)
	{
		vector =
			blaze::generate<blaze::rowVector>(size, [this](size_t pos) { return normalDistribution(randomEngine); });
	}
};
} // namespace metric::dnn

#endif