#ifndef UTILS_RANDOM_H_
#define UTILS_RANDOM_H_

#include <random>

#include "../../../../3rdparty/blaze/Math.h"

namespace metric
{
namespace dnn
{

namespace internal {

    template <typename DerivedX, typename DerivedY, typename XType, typename YType>
int create_shuffled_batches(
    const DerivedX& x, const DerivedY& y,
    int batch_size, std::mt19937& rng,
    std::vector<XType>& x_batches, std::vector<YType>& y_batches
)
{
    const int nobs = x.columns();
    const int dimx = x.rows();
    const int dimy = y.rows();

    if (y.columns() != nobs)
    {
        throw std::invalid_argument("Input X and Y have different number of observations");
    }

    // Randomly shuffle the IDs
    std::vector<int> id(nobs);
    std::iota(id.begin(), id.end(), 0);
    std::shuffle(id.begin(), id.end(), rng);

    // Compute batch size
    if (batch_size > nobs)
    {
        batch_size = nobs;
    }

    const int nbatch = (nobs - 1) / batch_size + 1;
    const int last_batch_size = nobs - (nbatch - 1) * batch_size;
    // Create shuffled data
    x_batches.clear();
    y_batches.clear();
    x_batches.reserve(nbatch);
    y_batches.reserve(nbatch);

    for (int i = 0; i < nbatch; i++)
    {
        const int bsize = (i == nbatch - 1) ? last_batch_size : batch_size;
        x_batches.push_back(XType(dimx, bsize));
        y_batches.push_back(YType(dimy, bsize));
        // Copy data
        const int offset = i * batch_size;

        for (int j = 0; j < bsize; j++)
        {
            blaze::column(x_batches[i], j) = blaze::column(x, id[offset + j]);
            blaze::column(y_batches[i], j) = blaze::column(y, id[offset + j]);
        }
    }

    return nbatch;
}

// Fill array with N(mu, sigma^2) random numbers
template<typename Scalar>
inline void set_normal_random(Scalar* arr, const int n, std::mt19937& rng,
                              const Scalar& mu = Scalar(0),
                              const Scalar& sigma = Scalar(1))
{
    std::normal_distribution<Scalar> normalDistribution(mu, sigma);
    for (auto i = 0; i < n; ++i) {
        arr[i] = normalDistribution(rng);
    }
}


} // namespace internal

} // namespace dnn
} // namespace metric


#endif /* UTILS_RANDOM_H_ */
