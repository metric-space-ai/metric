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
	   	
template <typename Sample, typename D = double>
D KolmogorovSmirnov_func(const Sample& sample_1, const Sample& sample_2)
{
    return 0;
}

template <typename Sample, typename D = double>
auto KolmogorovSmirnov::operator()(const Sample& sample_1, const Sample& sample_2) -> distance_return_type
{

	Sample& cumulitive_distribution_1;
	Sample& cumulitive_distribution_2;

	Sample::value_type cumulitive_sum = 0;
	std::cout << sample_1.size() << std::endl;
	for (int i = 0; i < sample_1.size(); i++)
	{
		cumulitive_sum += sample_1[i];
		cumulitive_distribution_1.push_back(cumulitive_sum);
	}

	cumulitive_sum = 0;
	for (int i = 0; i < sample_2.size(); i++)
	{
		cumulitive_sum += sample_2[i];
		cumulitive_distribution_2.push_back(cumulitive_sum);
	}
	
	D max_difference = 0;
	for (int i = 0; i < cumulitive_distribution_1.size(); i++)
	{
		std::cout << (cumulitive_distribution_1[i] - cumulitive_distribution_2[i]) << std::endl;
		if (abs(cumulitive_distribution_1[i] - cumulitive_distribution_2[i]) > max_difference)
		{
			max_difference = abs(cumulitive_distribution_1[i] - cumulitive_distribution_2[i]);
		}
	}


    return max_difference;
}


}  // namespace metric
#endif
