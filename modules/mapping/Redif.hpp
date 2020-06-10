#pragma once

#include "../../3rdparty/blaze/Blaze.h"
#include "../../modules/distance.hpp"

#include <vector>

namespace metric
{

template<typename Tv = double, class Metric = metric::Euclidean<Tv>>
class Redif
{
public:
    Redif(
        const std::vector<std::vector<Tv>>& trainData,
        size_t nNeighbors = 10,
        size_t nIter = 15,
        Metric metric = Metric()
    );
    ~Redif() = default;

    std::vector<std::vector<Tv>> encode(const std::vector<std::vector<Tv>>& x);

    blaze::DynamicMatrix<Tv> decode(
        const blaze::DynamicMatrix<Tv>& xEncoded,
        blaze::DynamicMatrix<Tv>& xTrainEncoded,
        const blaze::DynamicVector<size_t>& l_idx
    );

	std::vector<std::vector<Tv>> get_train_encoded()
	{
		std::vector<std::vector<Tv>> encoded_data_as_vectors(xTrainEncoded.rows(), std::vector<Tv>(xTrainEncoded.columns()));	
		for (size_t i = 0; i < xTrainEncoded.rows(); i++)
			for (size_t j = 0; j < xTrainEncoded.columns(); j++)
				encoded_data_as_vectors[i][j] = xTrainEncoded(i, j);

		return encoded_data_as_vectors;
	}
private:

    template<bool flag = blaze::rowVector>
    void Quicksort(blaze::DynamicVector<size_t>& idx, blaze::DynamicVector<Tv, flag>& data, size_t start, size_t end);

    template<bool flag = blaze::rowVector>
    size_t partition(
        blaze::DynamicVector<size_t> & idx,
        blaze::DynamicVector<Tv, flag> & data,
        size_t start,
        size_t end
    );

    blaze::DynamicMatrix<Tv> encode(const blaze::DynamicMatrix<Tv>& x);

    blaze::DynamicMatrix<Tv> getLocalDistMatrix(const blaze::DynamicMatrix<Tv>& dataSample);
    blaze::DynamicMatrix<Tv> calcWeightedGraphLaplacian(const blaze::DynamicMatrix<Tv>& localDist);

    blaze::DynamicMatrix<Tv> trainModel(size_t nIter);

    size_t nNeighbors;
    Metric metric;
    blaze::DynamicMatrix<Tv> xTrain;
    blaze::DynamicMatrix<Tv> xTrainEncoded;
    std::vector<blaze::DynamicMatrix<Tv>> LArray;
};

}

#include "Redif.cpp"
