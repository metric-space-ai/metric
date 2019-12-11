#ifndef UTILS_RANDOM_H_
#define UTILS_RANDOM_H_

#include "../../../../3rdparty/blaze/Math.h"
#include "../RNG.h"

namespace MiniDNN
{

namespace internal {


// Shuffle the integer array
inline void shuffle(int *arr, const int n, RNG &rng)
{
	for (int i = n - 1; i > 0; i--) {
		// A random non-negative integer <= i
		const int j = int(rng.rand() * (i + 1));
		// Swap arr[i] and arr[j]
		const int tmp = arr[i];
		arr[i] = arr[j];
		arr[j] = tmp;
	}
}

blaze::DynamicVector<int> getVectorIntLinspaced(const size_t size, const int low, const int high)
{
	blaze::DynamicVector<int> v(size);

	for (size_t i = 0UL; i < size; i++) {
		v[i] = blaze::rand<int>(low, high);
	}

	return v;
}


template <typename DerivedX, typename DerivedY, typename XType, typename YType>
int create_shuffled_batches(const DerivedX &x, const DerivedY &y,
								int batch_size, RNG &rng,
								std::vector<XType> &x_batches, std::vector<YType> &y_batches)
{
	const int nobs = x.rows();
	const int dimx = x.columns();
	const int dimy = y.columns();

	if (y.rows() != nobs) {
		throw std::invalid_argument("Input X and Y have different number of observations");
	}

    // Randomly shuffle the IDs
    blaze::DynamicVector<int> id = getVectorIntLinspaced(nobs, 0, nobs - 1);
    shuffle(id.data(), id.size(), rng);

    // Compute batch size
	if (batch_size > nobs) {
		batch_size = nobs;
	}

    const int nbatch = (nobs - 1) / batch_size + 1;
    const int last_batch_size = nobs - (nbatch - 1) * batch_size;

    // Create shuffled data
    x_batches.clear();
    y_batches.clear();
    x_batches.reserve(nbatch);
    y_batches.reserve(nbatch);

    for (int i = 0; i < nbatch; i++) {
        const int bsize = (i == nbatch - 1) ? last_batch_size : batch_size;
        x_batches.push_back(XType(bsize, dimx));
        y_batches.push_back(YType(bsize, dimx));

        // Copy data
        const int offset = i * batch_size;

        for (int j = 0; j < bsize; j++) {
            blaze::rows(x_batches[i], j) = blaze::rows(x, id[offset + j]);
            blaze::rows(y_batches[i], j) = blaze::rows(y, id[offset + j]);
        }
    }

    return nbatch;
}

// Fill array with N(mu, sigma^2) random numbers
template<typename Scalar>
inline void set_normal_random(Scalar* arr, const int n, RNG& rng,
                              const Scalar& mu = Scalar(0),
                              const Scalar& sigma = Scalar(1))
{
    // For simplicity we use Box-Muller transform to generate normal random variates
    const double two_pi = 6.283185307179586476925286766559;

    for (int i = 0; i < n - 1; i += 2)
    {
        const double t1 = sigma * std::sqrt(-2 * std::log(rng.rand()));
        const double t2 = two_pi * rng.rand();
        arr[i]     = t1 * std::cos(t2) + mu;
        arr[i + 1] = t1 * std::sin(t2) + mu;
    }

    if (n % 2 == 1)
    {
        const double t1 = sigma * std::sqrt(-2 * std::log(rng.rand()));
        const double t2 = two_pi * rng.rand();
        arr[n - 1] = t1 * std::cos(t2) + mu;
    }
}


} // namespace internal

} // namespace MiniDNN


#endif /* UTILS_RANDOM_H_ */
