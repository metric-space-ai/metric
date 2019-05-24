#include "poor_mans_quantum.hpp"

#include <limits>
#include <random>
#include "3dparty/blaze/Math.h"

#ifdef USE_VECTOR_SORT
#include "3dparty/vector_sort.hpp"
#endif

#include "poor_mans_quantum_impl/PMQ.hpp"


template <typename Distribution, typename T = double>
class random_generator
{
  private:
    Distribution _dist;

  public:
    random_generator(size_t seed = std::random_device{}(), Distribution d = Distribution());

    blaze::DynamicMatrix<T> operator() (const size_t n, const size_t k, const T par1, const T par2);
};

template <typename Distribution, typename T = double>
class pdf
{
  private:
    Distribution _dist;

  public:
    pdf(Distribution d = Distribution());

    blaze::DynamicMatrix<T> operator() (const blaze::DynamicMatrix<T> mat, const T par1, const T par2);
};

template <typename Distribution, typename T = double>
class cdf
{
  private:
    Distribution _dist;

  public:
    cdf(Distribution d = Distribution());

    blaze::DynamicMatrix<T> operator()(const blaze::DynamicMatrix<T> mat, const T par1, const T par2);
};

template <typename Distribution, typename T>
random_generator<Distribution, T>::random_generator(size_t seed, Distribution d)
    : _dist(d)
    {
      _dist.seed(seed);
}

template <typename Distribution, typename T>
pdf<Distribution, T>::pdf(Distribution d)
    : _dist(d)
{
}

template <typename Distribution, typename T>
cdf<Distribution, T>::cdf(Distribution d)
    : _dist(d)
{
}

template <typename Distribution, typename T>
blaze::DynamicMatrix<T>
random_generator<Distribution, T>::operator()(const size_t n, const size_t k, const T par1, const T par2)
{
    blaze::DynamicMatrix<T> M = _dist.rnd(n, k, par1, par2);

    return M;
}

template <typename Distribution, typename T>
blaze::DynamicMatrix<T>
pdf<Distribution, T>::operator()(const blaze::DynamicMatrix<T> mat, const T par1, const T par2)
{
    blaze::DynamicMatrix<T> M = _dist.pdf(mat, par1, par2);

    return M;
}

template <typename Distribution, typename T>
blaze::DynamicMatrix<T>
cdf<Distribution, T>::operator()(const blaze::DynamicMatrix<T> mat, const T par1, const T par2)
{
    blaze::DynamicMatrix<T> M = _dist.cdf(mat, par1, par2);

    return M;
}
