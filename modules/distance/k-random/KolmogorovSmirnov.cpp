/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/
#ifndef _METRIC_DISTANCE_K_RANDOM_KOLMOGOROV_SMIRNOV_CPP
#define _METRIC_DISTANCE_K_RANDOM_KOLMOGOROV_SMIRNOV_CPP

#include "KolmogorovSmirnov.hpp"

namespace metric {
	

template <typename Sample, typename D>
auto KolmogorovSmirnov<typename Sample, typename D>::operator()(const Sample& sample_1, const Sample& sample_2) -> distance_return_type
{
	Sample normalized_distribution_1(sample_1.size());
	Sample normalized_distribution_2(sample_2.size());
	Sample cumulitive_distribution_1;
	Sample cumulitive_distribution_2;

	// make cumulative distribution functions for the first sample

	Sample::value_type min_value = INFINITY;
	Sample::value_type max_value = -INFINITY; 
	Sample::value_type cumulitive_sum = 0;
	for (int i = 0; i < sample_1.size(); i++)
	{
		if (sample_1[i] > max_value)
		{
			max_value = sample_1[i];
		}
		if (sample_1[i] < min_value)
		{
			min_value = sample_1[i];
		}
	}

	for (int i = 0; i < sample_1.size(); i++)
	{
		normalized_distribution_1[i] = (sample_1[i] - min_value) / (max_value - min_value);
	}
	for (int i = 0; i < normalized_distribution_1.size(); i++)
	{
		cumulitive_sum += normalized_distribution_1[i];
		cumulitive_distribution_1.push_back(cumulitive_sum);
	}
	
	// make cumulative distribution functions for the second sample
	
	min_value = INFINITY;
	max_value = -INFINITY;
	cumulitive_sum = 0;
	for (int i = 0; i < sample_2.size(); i++)
	{
		if (sample_2[i] > max_value)
		{
			max_value = sample_2[i];
		}
		if (sample_2[i] < min_value)
		{
			min_value = sample_2[i];
		}
	}

	for (int i = 0; i < sample_2.size(); i++)
	{
		normalized_distribution_2[i] = (sample_2[i] - min_value) / (max_value - min_value);
	}
	for (int i = 0; i < normalized_distribution_2.size(); i++)
	{
		cumulitive_sum += normalized_distribution_2[i];
		cumulitive_distribution_2.push_back(cumulitive_sum);
	}
	
	// find the most difference between distributions
	
	D max_difference = 0;
	for (int i = 0; i < cumulitive_distribution_1.size(); i++)
	{
		if (abs(cumulitive_distribution_1[i] - cumulitive_distribution_2[i]) > max_difference)
		{
			max_difference = abs(cumulitive_distribution_1[i] - cumulitive_distribution_2[i]);
		}
	}


    return max_difference;
}


}  // namespace metric
#endif
