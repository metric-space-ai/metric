#pragma once

#include <vector>

#include "3rdparty/blaze/Blaze.h"
#include "modules/distance.hpp"

namespace metric
{
	template<typename recType, class Metric = metric::Euclidian<recType>, typename Tv = double>
	class Redif
	{
	public:
		Redif(std::vector<std::vector<Tv>> trainData, size_t nNeighbors = 10, size_t nIter = 15);
		~Redif() = default;
		
		blaze::DynamicMatrix<Tv> encode(blaze::DynamicMatrix<Tv> x, blaze::DynamicVector<Tv> & l_idx);
		blaze::DynamicMatrix<Tv> decode(blaze::DynamicMatrix<Tv> xEncoded, blaze::DynamicMatrix<Tv> xTrainEncoded, blaze::DynamicVector<size_t> l_idx);
	private:

		template<bool flag = blaze::rowVector>
		void Quicksort(blaze::DynamicVector<size_t> & idx, blaze::DynamicVector<Tv, flag> & data, size_t start, size_t end);

		template<bool flag = blaze::rowVector>
		size_t partition(blaze::DynamicVector<size_t> & idx, blaze::DynamicVector<Tv, flag> & data, size_t start, size_t end);

		blaze::DynamicMatrix<Tv> getLocalDistMatrix(blaze::DynamicMatrix<Tv> dataSample);
		blaze::DynamicMatrix<Tv> calcWeightedGraphLaplacian(blaze::DynamicMatrix<Tv> localDist);
		void trainModel(blaze::DynamicMatrix<Tv> & xTrainDe, size_t nIter);

		blaze::DynamicMatrix<Tv> xTrain;
		std::vector<blaze::DynamicMatrix<Tv>> LArray;
		size_t nNeighbors;
	};
}
