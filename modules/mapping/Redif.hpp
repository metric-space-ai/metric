#pragma once

#include "3rdparty/blaze/Blaze.h"
#include "modules/distance.hpp"

#include <vector>

namespace metric
{
	template<typename Tv = double>
	class Redif
	{
	public:
	    template <class Metric = metric::Euclidian<Tv>>
		Redif(
		    const std::vector<std::vector<Tv>>& trainData,
		    size_t nNeighbors = 10,
		    size_t nIter = 15
		    Metric metric = Metric()
        );
		~Redif() = default;

		template <class Metric = metric::Euclidian<Tv>>
		blaze::DynamicMatrix<Tv> encode(
		    const blaze::DynamicMatrix<Tv>& x,
		    blaze::DynamicVector<Tv> & l_idx,
		    Metric metric = Metric()
        );
		blaze::DynamicMatrix<Tv> decode(
		    const blaze::DynamicMatrix<Tv>& xEncoded,
		    const blaze::DynamicMatrix<Tv>& xTrainEncoded,
		    const blaze::DynamicVector<size_t>& l_idx
        );
	private:

		template<bool flag = blaze::rowVector>
		void Quicksort(
		    blaze::DynamicVector<size_t> & idx,
		    blaze::DynamicVector<Tv, flag> & data,
		    size_t start,
		    size_t end
        );

		template<bool flag = blaze::rowVector>
		size_t partition(
		    blaze::DynamicVector<size_t> & idx,
		    blaze::DynamicVector<Tv, flag> & data,
		    size_t start,
		    size_t end
        );

		blaze::DynamicMatrix<Tv> getLocalDistMatrix(const blaze::DynamicMatrix<Tv>& dataSample);
		blaze::DynamicMatrix<Tv> calcWeightedGraphLaplacian(const blaze::DynamicMatrix<Tv>& localDist);
		blaze::DynamicMatrix<Tv> trainModel(size_t nIter);

		blaze::DynamicMatrix<Tv> xTrain;
		std::vector<blaze::DynamicMatrix<Tv>> LArray;
		size_t nNeighbors;
	};
}
