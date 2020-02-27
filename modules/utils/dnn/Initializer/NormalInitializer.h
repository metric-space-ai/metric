#ifndef NORMALINITIALIZER_H
#define NORMALINITIALIZER_H

#include "../Initializer.h"

#include <random>


namespace metric::dnn {
	template<typename Scalar>
	class NormalInitializer : public Initializer<Scalar>
	{
	protected:
		using typename Initializer<Scalar>::ColumnMatrix;
		using typename Initializer<Scalar>::RowVector;

		std::normal_distribution<Scalar> normalDistribution;
		std::mt19937 randomEngine;

	public:
		NormalInitializer(const Scalar mu, const Scalar sigma, const std::mt19937& randomEngine) :
																							randomEngine(randomEngine),
																							normalDistribution(mu, sigma)
		{}

		void init(const size_t rows, const size_t columns, ColumnMatrix &matrix) const
		{
			matrix = blaze::generate<Scalar>(rows, columns, [this](size_t i, size_t j){return normalDistribution(randomEngine);});
		}

		void init(const size_t size, RowVector &vector) const
		{
			vector = blaze::generate<Scalar, blaze::rowVector>(size, [this](size_t pos){return normalDistribution(randomEngine);});
		}

	};
}

#endif