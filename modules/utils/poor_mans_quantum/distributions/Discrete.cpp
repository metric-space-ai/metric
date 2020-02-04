/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_DISCRETE_CPP
#define _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_DISCRETE_CPP

#include <random>
#include <vector>

#include "../math_functions.hpp"

namespace metric {

template <typename T>
Discrete<T>::Discrete()
    : _generator(std::random_device {}())
{
}

// Updated by Stepan Mamontov 04.02.2020
template <typename T>
Discrete<T>::Discrete(std::vector<T> samples)
    : _generator(std::random_device {}())
{
	std::sort(samples.begin(), samples.end());
	for (int i = 0; i < samples.size(); i++)
	{
		std::cout << samples[i] << " ";
	}
	std::cout << std::endl;
	
	std::map<T, int> hist{};
	for (int i = 0; i < samples.size(); i++)
	{
        ++hist[samples[i]];
    }
	
	_data.clear();
	_prob.clear();
	double cumulative_sum = 0;
	for(std::map<T, int>::iterator it = hist.begin(); it != hist.end(); ++it) 
	{
		_data.push_back(it->first);
		cumulative_sum += (double) it->second / samples.size();
		_prob.push_back(cumulative_sum);
	}
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
	// Updated mean calculation by Stepan Mamontov 04.02.2020
	// Updated according to formula: mean = sum (x * P(x))  ->  https://en.wikipedia.org/wiki/Mean#Mean_of_a_probability_distribution
    T sum = _data[0] * _prob[0];
    for (size_t i = 1; i < _data.size(); ++i) {
        sum += _data[i] * (_prob[i] - _prob[i - 1]);
    }

    return sum;
}

template <typename T>
T Discrete<T>::variance()
{
	// Updated variance calculation by Stepan Mamontov 04.02.2020
	// Updated accroding to formula: variance = sum ((x - mean) * (x - mean) * P(x))  ->  https://en.wikipedia.org/wiki/Variance#Discrete_random_variable
    T mean_value = mean();
    T sum = (_data[0] - mean_value) * (_data[0] - mean_value) * _prob[0];
    for (size_t i = 1; i < _data.size(); ++i) {
        sum += (_data[i] - mean_value) * (_data[i] - mean_value) * (_prob[i] - _prob[i-1]);
    }
    return sum;
}

/*** cdf ***/
template <typename T>
T Discrete<T>::cdf(const T x)
{
	// Updated cdf calculation by Stepan Mamontov 04.02.2020
	// Updated by return interpolated func of cumulative propability from distributed value
    std::vector<T> values;
    values.push_back(x);
	T result = akimaInterp1(_data, _prob, values)[0];
	
	if (result < 0)
	{
		result = 0;
	}
	
	if (result > 1)
	{
		result = 1;
	}

	return result;
}

}  // end namespace metric
#endif  // header guard
