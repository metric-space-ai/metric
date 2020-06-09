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

    std::tuple<std::vector<std::vector<Tv>>, std::vector<size_t>> encode(
        const std::vector<std::vector<Tv>>& x
    );

    blaze::DynamicMatrix<Tv> decode(
        const blaze::DynamicMatrix<Tv>& xEncoded,
        blaze::DynamicMatrix<Tv>& xTrainEncoded,
        const blaze::DynamicVector<size_t>& l_idx
    );
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

    std::tuple<blaze::DynamicMatrix<Tv>, blaze::DynamicVector<size_t>> encode(
        const blaze::DynamicMatrix<Tv>& x
    );

    blaze::DynamicMatrix<Tv> getLocalDistMatrix(const blaze::DynamicMatrix<Tv>& dataSample, Metric metric = Metric());
    blaze::DynamicMatrix<Tv> calcWeightedGraphLaplacian(const blaze::DynamicMatrix<Tv>& localDist);

    blaze::DynamicMatrix<Tv> trainModel(size_t nIter, Metric metric = Metric());

    size_t nNeighbors;
    Metric metric_;
    blaze::DynamicMatrix<Tv> xTrain;
    std::vector<blaze::DynamicMatrix<Tv>> LArray;
};

}

#include "Redif.cpp"
