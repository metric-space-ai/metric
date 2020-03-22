/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/

#ifndef _METRIC_UTILS_POOR_MANS_QUANTUM_POOR_MANS_QUANTUM_CPP
#define _METRIC_UTILS_POOR_MANS_QUANTUM_POOR_MANS_QUANTUM_CPP
#include "../poor_mans_quantum.hpp"

#include <limits>
#include <random>

#include "../../../3rdparty/blaze/Math.h"

#ifdef USE_VECTOR_SORT
#include "3dparty/vector_sort.hpp"
#endif

#include "math_functions.hpp"
#include "distributions/Normal.hpp"
#include "distributions/Weibull.hpp"
//#include "distributions/Binomial.hpp"
#include "distributions/Discrete.hpp"

#include <random>

namespace metric {

template <typename Distribution, typename T = double>
class random_generator {
private:
    Distribution _dist;

public:
    random_generator(size_t seed = std::random_device {}(), Distribution d = Distribution());

    blaze::DynamicMatrix<T> operator()(const size_t n, const size_t k, const T par1, const T par2);
};

template <typename Distribution, typename T = double>
class pdf {
private:
    Distribution _dist;

public:
    pdf(Distribution d = Distribution());

    blaze::DynamicMatrix<T> operator()(const blaze::DynamicMatrix<T> mat, const T par1, const T par2);
};

template <typename Distribution, typename T = double>
class cdf {
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
blaze::DynamicMatrix<T> random_generator<Distribution, T>::operator()(
    const size_t n, const size_t k, const T par1, const T par2)
{
    blaze::DynamicMatrix<T> M = _dist.rnd(n, k, par1, par2);

    return M;
}

template <typename Distribution, typename T>
blaze::DynamicMatrix<T> pdf<Distribution, T>::operator()(const blaze::DynamicMatrix<T> mat, const T par1, const T par2)
{
    blaze::DynamicMatrix<T> M = _dist.pdf(mat, par1, par2);

    return M;
}

template <typename Distribution, typename T>
blaze::DynamicMatrix<T> cdf<Distribution, T>::operator()(const blaze::DynamicMatrix<T> mat, const T par1, const T par2)
{
    blaze::DynamicMatrix<T> M = _dist.cdf(mat, par1, par2);

    return M;
}

/*** constructor for univariate distribution ***/
template <typename Distribution, typename T>
PMQ<Distribution, T>::PMQ(T par1, T par2, size_t n, Distribution d)
    : _dist(d)
    , _generator(std::random_device {}())
{
    _dist._p1 = par1;
    _dist._p2 = par2;
    T min = quantil(T(1) / T(n));
    T max = quantil(T(1) - T(1) / T(n));
    _dist._prob = linspace(T(0.5) / T(n), T(1) - T(0.5) / T(n), n);
    _dist._data.resize(n);
    for (size_t i = 0; i < n; ++i) {
        _dist._data[i] = icdf(_dist._prob[i]);
    }
}

// Updated by Stepan Mamontov 26.02.2020 (updated to use generic types)
/*** constructor for discrete samples ***/
//template <typename Distribution, typename T>
//PMQ<Distribution, T>::PMQ(std::vector<T> data, Distribution d)
//    : _dist(d)
//    , _generator(std::random_device {}())
//{
//    _dist._data.resize(data.size());
//    for (size_t i = 0; i < data.size(); ++i) {
//        _dist._data[i] = T(data[i]);
//    }
//#if USE_VECTOR_SORT
//    vector_sort::sort(_dist._data);
//#else
//    std::sort(_dist._data.begin(), _dist._data.end());
//#endif
//	auto prob = linspace(T(0.5) / T(data.size()), T(1) - T(0.5) / T(data.size()), data.size());
//
//    _dist._prob.resize(prob.size());
//    for (size_t i = 0; i < prob.size(); ++i) {
//        _dist._prob[i] = T(prob[i]);
//    }
//}

// Updated by Stepan Mamontov 26.02.2020 (calculate histogram of the data and then calculate cumulative distribution)
/*** constructor for discrete samples ***/
template <typename Distribution, typename T>
PMQ<Distribution, T>::PMQ(std::vector<T> data, Distribution d)
    : _dist(d)
    , _generator(std::random_device {}())
{
#if USE_VECTOR_SORT
	vector_sort::sort(data);
#else
	std::sort(data.begin(), data.end());
#endif

	std::map<T, int> hist{};
	for (int i = 0; i < data.size(); i++)
	{
        ++hist[data[i]];
    }
	
	_dist._data.clear();
	_dist._prob.clear();
	T cumulative_sum = 0;
	for(typename std::map<T, int>::iterator it = hist.begin(); it != hist.end(); ++it)
	{
		_dist._data.push_back(it->first);
		cumulative_sum += (T) it->second / data.size();
		_dist._prob.push_back(cumulative_sum);
	}
}

template <typename Distribution, typename T>
size_t PMQ<Distribution, T>::size()
{
    return _dist._data.size();
}

template <typename Distribution, typename T>
template <typename mT>
mT PMQ<Distribution, T>::believe(mT rv)
{

    size_t n1 = _dist._data.size();
    size_t n2 = rv._dist._data.size();
    size_t n = n1 + n2;

    size_t it1 = std::round((n1) / float(n) * float(RV_SAMPLES));
    size_t it2 = std::round((n2) / float(n) * float(RV_SAMPLES));

    std::vector<float> y1(it1);
    std::vector<float> y2(it2);

    for (size_t i = 0; i < it1; ++i) {
        y1[i] = rnd();
    }

    for (size_t i = 0; i < it2; ++i) {
        auto value1 = rnd();
        auto value2 = rnd();
        auto value3 = rv.rnd();

        if (std::abs(value3 - value1) < std::abs(value3 - value2))
            y2[i] = value1;
        else
            y2[i] = value2;
    }

    y1.insert(y1.end(), y2.begin(), y2.end());  // concat vectors

    mT out(resize(y1, n / 2));
    return out;
}

template <typename Distribution, typename T>
T PMQ<Distribution, T>::rnd()
{
    return _dist.rnd();
}

template <typename Distribution, typename T>
T PMQ<Distribution, T>::mean()
{
    return _dist.mean();
}

template <typename Distribution, typename T>
T PMQ<Distribution, T>::variance()
{
    return _dist.variance();
}

template <typename Distribution, typename T>
T PMQ<Distribution, T>::median()
{
    return _dist.median();
}

template <typename Distribution, typename T>
T PMQ<Distribution, T>::quantil(T p)
{
    return _dist.quantil(p);
}

template <typename Distribution, typename T>
T PMQ<Distribution, T>::cdf(T x)
{
    return _dist.cdf(x);
}

template <typename Distribution, typename T>
T PMQ<Distribution, T>::icdf(T x)
{
    return _dist.icdf(x);
}

template <typename Distribution, typename T>
T PMQ<Distribution, T>::pdf(T x)
{
    return _dist.pdf(x);
}

/*** addition ***/
template <typename T1, typename T2>
PMQ<Discrete<float>> operator+(PMQ<T1> rv1, PMQ<T2> rv2)
{
    size_t n = (rv1._dist._data.size() + rv2._dist._data.size()) / 2;
    std::vector<float> y(RV_SAMPLES);
    for (size_t i = 0; i < y.size(); ++i) {
        y[i] = rv1.rnd() + rv2.rnd();
    }
    PMQ<Discrete<float>> out(resize(y, n));
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator+(PMQ<mT> rv, float x)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = rv._dist._data[i] + x;
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator+(float x, PMQ<mT> rv)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = rv._dist._data[i] + x;
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator+(PMQ<mT> rv, double x)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = rv._dist._data[i] + float(x);
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator+(double x, PMQ<mT> rv)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = rv._dist._data[i] + float(x);
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

/*** substraction ***/
template <typename mT1, typename mT2>
PMQ<Discrete<float>> operator-(PMQ<mT1> rv1, PMQ<mT2> rv2)
{
    size_t n = (rv1._dist._data.size() + rv2._dist._data.size()) / 2;
    std::vector<float> y(RV_SAMPLES);
    for (size_t i = 0; i < y.size(); ++i) {
        y[i] = rv1.rnd() - rv2.rnd();
    }
    PMQ<Discrete<float>> out(resize(y, n));
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator-(PMQ<mT> rv, float x)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = rv._dist._data[i] - x;
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator-(float x, PMQ<mT> rv)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = x - rv._dist._data[i];
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator-(PMQ<mT> rv, double x)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = rv._dist._data[i] - float(x);
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator-(double x, PMQ<mT> rv)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = float(x) - rv._dist._data[i];
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

/*** multiplication ***/
template <typename mT1, typename mT2>
PMQ<Discrete<float>> operator*(PMQ<mT1> rv1, PMQ<mT2> rv2)
{
    size_t n = (rv1._dist._data.size() + rv2._dist._data.size()) / 2;
    std::vector<float> y(RV_SAMPLES);
    for (size_t i = 0; i < y.size(); ++i) {
        y[i] = rv1.rnd() * rv2.rnd();
    }
    PMQ<Discrete<float>> out(resize(y, n));
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator*(PMQ<mT> rv, float x)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = rv._dist._data[i] * x;
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator*(float x, PMQ<mT> rv)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = x * rv._dist._data[i];
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator*(PMQ<mT> rv, double x)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = rv._dist._data[i] * float(x);
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator*(double x, PMQ<mT> rv)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = rv._dist._data[i] * float(x);
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

/*** division ***/
template <typename mT1, typename mT2>
PMQ<Discrete<float>> operator/(PMQ<mT1> rv1, PMQ<mT2> rv2)
{
    size_t n = (rv1._dist._data.size() + rv2._dist._data.size()) / 2;
    std::vector<float> y(RV_SAMPLES);
    for (size_t i = 0; i < y.size(); ++i) {
        y[i] = rv1.rnd() / rv2.rnd();
    }
    PMQ<Discrete<float>> out(resize(y, n));
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator/(PMQ<mT> rv, float x)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = rv._dist._data[i] / x;
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator/(float x, PMQ<mT> rv)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = x / rv._dist._data[i];
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator/(PMQ<mT> rv, double x)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = rv._dist._data[i] / float(x);
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

template <typename mT>
PMQ<Discrete<float>> operator/(double x, PMQ<mT> rv)
{
    size_t n = rv._dist._data.size();
    std::vector<float> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = float(x) / rv._dist._data[i];
    }
    PMQ<Discrete<float>> out(y);
    return out;
}

/*** equality ***/
// significance test
template <typename mT1, typename mT2>
float operator==(const PMQ<mT1>& rv0, const PMQ<mT2>& rv1)
{
    auto [rv0_l, rv0_r] = rv0.confidence(1 - RV_ERROR);
    auto [rv01_l, rv01_r] = rv1.merged_confidence(rv0_l, rv0_r, 1 - RV_ERROR);
    auto l = rv1.in_confidence(rv01_l, rv01_r);

    auto [rv1_l, rv1_r] = rv1.confidence(1 - RV_ERROR);
    auto [rv10_l, rv10_r] = rv0.merged_confidence(rv1_l, rv1_r, 1 - RV_ERROR);
    auto r = rv0.in_confidence(rv10_l, rv10_r);

    return (l + r) / 2;
}

/*** inequality ***/
// significance test
template <typename mT1, typename mT2>
float operator!=(const PMQ<mT1>& rv0, const PMQ<mT2>& rv1)
{
    auto [rv0_l, rv0_r] = rv0.confidence(1 - RV_ERROR);
    auto [rv01_l, rv01_r] = rv1.merged_confidence(rv0_l, rv0_r, 1 - RV_ERROR);
    auto l = rv1.out_confidence(rv01_l, rv01_r);

    auto [rv1_l, rv1_r] = rv1.confidence(1 - RV_ERROR);
    auto [rv10_l, rv10_r] = rv0.merged_confidence(rv1_l, rv1_r, 1 - RV_ERROR);
    auto r = rv0.out_confidence(rv10_l, rv10_r);

    return (l + r) / 2;
}

template <typename T1, typename T2>
PMQ<Discrete<float>> merge(PMQ<T1> rv1, PMQ<T2> rv2)
{
    size_t n = rv1._dist._data.size() + rv2._dist._data.size();
    std::vector<float> y(rv1._dist._data);
    y.insert(y.end(), rv2._dist._data.begin(), rv2._dist._data.end());
    PMQ<Discrete<float>> rv(y);
    return rv;
}

template <typename T1>
std::tuple<PMQ<Discrete<float>>, PMQ<Discrete<float>>> split(PMQ<T1> rv)
{
    size_t n = rv._dist._data.size() / 2;
    std::vector<float> y1(RV_SAMPLES);
    std::vector<float> y2(RV_SAMPLES);
    for (size_t i = 0; i < y1.size(); ++i) {
        auto value1 = rv.rnd();
        auto value2 = rv.rnd();

        if (value1 > value2) {
            std::swap(value1, value2);
        }
        y1[i] = value1;
        y2[i] = value2;
    }
    PMQ<Discrete<float>> out1(resize(y1, n));
    PMQ<Discrete<float>> out2(resize(y2, n));
    return { out1, out2 };
}

template <typename Distribution, typename T>
std::tuple<PMQ<Discrete<float>>, PMQ<Discrete<float>>> PMQ<Distribution, T>::confidence(const T& confidencelevel) const
{

    std::random_device rd;  //seed for the random number engine
    std::mt19937 gen(rd());  //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<T> dis(T(0), T(1));

    size_t samples = RV_SAMPLES;
    size_t windowSize = _dist._data.size();
    // compute probability matrix of set_0
    std::vector<std::vector<T>> m_0(samples, std::vector<T>(_dist._data.size()));

    for (size_t i = 0; i < samples; ++i) {
        for (size_t j = 0; j < _dist._data.size(); ++j) {
            m_0[i][j] = T(dis(gen));  // fill with random numbers
        }
#if USE_VECTOR_SORT
        vector_sort::sort(m_0[i]);
#else
        std::sort(m_0[i].begin(), m_0[i].end());  // sort the row
#endif
        m_0[i] = akimaInterp1(_dist._prob, _dist._data, m_0[i]);  // interpolate the random numbers
    }

    auto m_0t = transpose(m_0);

    m_0.clear();
    m_0.shrink_to_fit();

    // compute left and right confidence boundaries of set_0
    std::vector<T> set_0_left(_dist._data.size());
    std::vector<T> set_0_right(_dist._data.size());
    for (size_t i = 0; i < _dist._data.size(); ++i) {
        set_0_left[i] = quickQuantil(m_0t[i], (T(1) - confidencelevel) / T(2));
        set_0_right[i] = quickQuantil(m_0t[i], confidencelevel + (T(1) - confidencelevel) / T(2));
    }

    m_0t.clear();
    m_0t.shrink_to_fit();

    PMQ<Discrete<float>> left(set_0_left);
    PMQ<Discrete<float>> right(set_0_right);
    return { left, right };
}

template <typename Distribution, typename T>
template <typename mT1, typename mT2>
std::tuple<PMQ<Discrete<float>>, PMQ<Discrete<float>>> PMQ<Distribution, T>::merged_confidence(
    const mT1& set_0_left, const mT2& set_0_right, const T confidencelevel) const
{

    std::random_device rd;  //seed for the random number engine
    std::mt19937 gen(rd());  //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<T> dis(T(0), T(1));

    size_t samples = RV_SAMPLES;
    size_t windowSize = _dist._data.size();

    // compute probability matrix of left and right and medians of set_0
    std::vector<std::vector<T>> m_prop_1(samples, std::vector<T>(windowSize));

    for (size_t i = 0; i < samples; ++i) {
        for (size_t j = 0; j < windowSize; ++j) {
            m_prop_1[i][j] = T(dis(gen));  // fill with random numbers
        }
#if USE_VECTOR_SORT
        vector_sort::sort(m_prop_1[i]);
#else
        std::sort(m_prop_1[i].begin(), m_prop_1[i].end());  // sort the row

#endif
    }

    std::vector<std::vector<T>> quants(2, std::vector<T>(windowSize));

    // left
    std::vector<std::vector<T>> m(samples, std::vector<T>(windowSize));
    for (size_t i = 0; i < samples; ++i) {
        m[i] = akimaInterp1(
            set_0_left._dist._prob, set_0_left._dist._data, m_prop_1[i]);  // interpolate the random numbers
    }
    auto mt = transpose(m);

    for (size_t i = 0; i < windowSize; ++i) {
        quants[0][i] = quickQuantil(mt[i], (T(1.0) - confidencelevel) / T(2.0));
    }

    //right
    for (size_t i = 0; i < samples; ++i) {
        m[i] = akimaInterp1(set_0_right._dist._prob, set_0_right._dist._data, m_prop_1[i]);
    }

    mt = transpose(m);

    for (size_t i = 0; i < windowSize; ++i) {
        quants[1][i] = quickQuantil(mt[i], confidencelevel + (T(1.0) - confidencelevel) / T(2.0));
    }

    PMQ<Discrete<float>> left(quants[0]);
    PMQ<Discrete<float>> right(quants[1]);
    return { left, right };
}

template <typename Distribution, typename T>
float PMQ<Distribution, T>::is_normal_distributed()
{
    PMQ<Normal> control(mean(), variance(), size());
    PMQ<Normal> var(_dist._data);  // TODO: use self reference
    return (var == control);
}

template <typename Distribution, typename T>
float PMQ<Distribution, T>::is_weibull_distributed()
{
    auto [x0, x1] = weibull_fit(_dist._data);
    PMQ<Weibull> control(x0, x1, size());
    PMQ<Normal> var(_dist._data);  // TODO: use self reference
    return (var == control);
}

template <typename Distribution, typename T>
template <typename mT1, typename mT2>
float PMQ<Distribution, T>::in_confidence(const mT1& set_left, const mT2& set_right) const
{
    if (set_left._dist._data.size() != _dist._data.size() || set_right._dist._data.size() != _dist._data.size()) {
        return float(-999999999);
    }

    int num_out = 0;
    for (size_t i = 0; i < _dist._data.size(); ++i) {
        if (_dist._data[i] > set_left._dist._data[i] && _dist._data[i] < set_right._dist._data[i]) {
            num_out += 1;
        }
    }
    return float(num_out) / float(_dist._data.size());
}

template <typename Distribution, typename T>
template <typename mT1, typename mT2>
float PMQ<Distribution, T>::out_confidence(const mT1& set_left, const mT2& set_right) const
{
    if (set_left._dist._data.size() != _dist._data.size() || set_right._dist._data.size() != _dist._data.size()) {
        return float(-999999999);
    }

    int num_out = 0;
    for (size_t i = 0; i < _dist._data.size(); ++i) {
        if (_dist._data[i] < set_left._dist._data[i] || _dist._data[i] > set_right._dist._data[i]) {
            num_out += 1;
        }
    }
    return float(num_out) / float(_dist._data.size());
}

}  //end namespace
#endif