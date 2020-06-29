
//#include "../../../modules/distance/k-related/Standards.hpp"
#include "../../../modules/utils/type_traits.hpp"
#include "../../utils/wrappers/lapack.hpp"


namespace metric {


/*
namespace riemannian_details {


// TODO copy functions

// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
template <typename Container, typename Functor>
double estimate(
        const Container & data,
        const Functor& entropy,
        const size_t sampleSize,
        const double threshold,
        size_t maxIterations
){
    using T = type_traits::underlying_type_t<Container>;
    using V = type_traits::index_value_type_t<Container>;
    const size_t dataSize = data.size();

    // Update maxIterations
    if (maxIterations == 0) {
        maxIterations = dataSize / sampleSize;
    }

    if (maxIterations > dataSize / sampleSize) {
        maxIterations = dataSize / sampleSize;
    }

    if (maxIterations < 1) {
        return entropy(data);
    }

    // Create shuffle indexes
    std::vector<size_t> indexes(dataSize);
    std::iota(indexes.begin(), indexes.end(), 0);

    auto rng = std::default_random_engine();
    std::shuffle(indexes.begin(), indexes.end(), rng);

    // Create vector container for fast random access
    const std::vector<V> vectorA(data.begin(), data.end());

    // Create samples
    std::vector<V> sampleA;
    sampleA.reserve(sampleSize);

    std::vector<double> entropyValues;
    double mu = 0;
    for (auto i = 1; i <= maxIterations; ++i) {
        size_t start = (i - 1) * sampleSize;
        size_t end = std::min(i * sampleSize - 1, dataSize - 1);

        // Create samples
        sampleA.clear();

        for (auto j = start; j < end; ++j) {
            sampleA.push_back(vectorA[indexes[j]]);
        }

        // Get sample mgc value
        double sample_entopy = entropy(sampleA);
        entropyValues.push_back(sample_entopy);

        std::sort(entropyValues.begin(), entropyValues.end());

        const size_t n = entropyValues.size();
        const auto p0 = entropy_details::linspace(0.5 / n, 1 - 0.5 / n, n);

        mu = entropy_details::mean(entropyValues);
        double sigma = entropy_details::variance(entropyValues, mu);

        const std::vector<double> synth = entropy_details::icdf(p0, mu, sigma);
        std::vector<double> diff;
        diff.reserve(n);
        for (auto i = 0; i < n; ++i) {
            diff.push_back(entropyValues[i] - synth[i]);
        }

        auto convergence = entropy_details::peak2ems(diff) / n;
        std::cout << n << " " << convergence << " " << sample_entopy << " " << mu << std::endl;

        if (convergence < threshold) {
            return mu;
        }
    }

    return mu;
}


}
// */


template <typename RecType, typename Metric>
template <typename C>
double RiemannianDistance<RecType, Metric>::operator()(const C& Xc, const C& Yc) const {
    using V = metric::type_traits::underlying_type_t<C>;

    blaze::DynamicMatrix<V> distancesX (Xc.size(), Xc.size(), 0);
    blaze::DynamicMatrix<V> distancesY (Yc.size(), Yc.size(), 0);

    for (size_t i = 0; i<Xc.size(); ++i) {
        for (size_t j = 0; j<Xc.size(); ++j) {
            auto d = metric(Xc[i], Xc[j]);
            if (i < j) { // upper triangular area only
                distancesX(i, j) = -d; // Laplacian matrix
                distancesX(i, i) += d;
                distancesX(j, j) += d;
            }
        }
    }
    for (size_t i = 0; i<Yc.size(); ++i) {
        for (size_t j = 0; j<Yc.size(); ++j) {
            auto d = metric(Yc[i], Yc[j]);
            if (i < j) { // upper triangular area only
                distancesY(i, j) = d;
                distancesY(i, i) += d;
                distancesY(j, j) += d;
            }
        }
    }
    return matDistance(distancesX, distancesY); // applying Riemannian to these distance matrices
}


template <typename RecType, typename Metric>
template <typename T>
T RiemannianDistance<RecType, Metric>::matDistance(blaze::DynamicMatrix<T> A, blaze::DynamicMatrix<T> B) const {
    blaze::DynamicVector<T> eigenValues;
    sygv(A, B, eigenValues);

    for (T& e : eigenValues) {
        if (e <= std::numeric_limits<T>::epsilon()) {
            e = 1;
        }
    }
    return sqrt(blaze::sum(blaze::pow(blaze::log(eigenValues), 2)));
}


}
