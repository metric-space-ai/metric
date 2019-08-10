#ifndef _PMQ_DISCRETE_HPP
#define _PMQ_DISCRETE_HPP
/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#include "../math_functions.hpp"

namespace metric {

template <typename T = float>
class Discrete
{
  private:
  public:
    std::mt19937_64 _generator;
    T _p1;
    T _p2;
    std::vector<T> _data;
    std::vector<T> _prob;

    void seed(size_t seed);

    Discrete();

    /*** sample a random value ***/
    T rnd();
    T median();
    T quantil(T p);
    T mean();
    T variance();

    /*** cdf ***/
    T cdf(const T x);
};

template <typename T>
Discrete<T>::Discrete()
    : _generator(std::random_device{}())
{
}

template <typename T>
T Discrete<T>::rnd()
{
    std::uniform_real_distribution<T> Discrete_dist(T(0), T(1));
    std::vector<T> values;
    values.push_back(Discrete_dist(_generator));
    return akimaInterp1(_prob, _data, values)[0];
}

template <typename T>
T Discrete<T>::median()
{
    std::vector<T> values;
    values.push_back(0.5);
    return akimaInterp1(_prob, _data, values)[0];
}

template <typename T>
T Discrete<T>::quantil(T p)
{
    std::vector<T> values;
    values.push_back(p);
    return akimaInterp1(_prob, _data, values)[0];
}

template <typename T>
T Discrete<T>::mean()
{
    T sum = 0;
    for (size_t i = 1; i < _data.size(); ++i)
    {
        sum += ( _data[i] - _data[i-1] ) * (_prob[i] - _prob[i-1]);
    }
    T value = sum * (_data[_data.size()-1] -_data[0]);// / T(_data.size());

    return value;
}

template <typename T>
T Discrete<T>::variance()
{
    T mean_value = mean();
    T sum = 0;
    for (size_t i = 0; i < _data.size(); ++i)
    {
        sum += ( _data[i] - mean_value ) * ( _data[i] - mean_value );
    }
    return sum;
}

/*** cdf ***/
template <typename T>
T Discrete<T>::cdf(const T x)
{
    return (T(0.5) * (T(1) + (x - _p1) / (_p2 * 1.41421356237309504880)));
}

} // end namespace metric
#endif // header guard