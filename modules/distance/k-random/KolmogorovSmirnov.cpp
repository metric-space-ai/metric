/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/
#ifndef _METRIC_DISTANCE_K_RANDOM_KOLMOGOROV_SMIRNOV_CPP
#define _METRIC_DISTANCE_K_RANDOM_KOLMOGOROV_SMIRNOV_CPP

#include "KolmogorovSmirnov.hpp"
#include "../../utils/poor_mans_quantum.hpp"

namespace metric {

template <typename Sample, typename D>
auto KolmogorovSmirnov<typename Sample, typename D>::operator()(const Sample& sample_1, const Sample& sample_2) -> distance_return_type
{
	PMQ pmq_1(sample_1);
	PMQ pmq_2(sample_2);
		
	// find the most difference between distributions

	Sample concat_data;
	for (int i = 0; i < sample_1.size(); i++)
	{
		concat_data.push_back(sample_1[i]);
	}
	for (int i = 0; i < sample_2.size(); i++)
	{
		concat_data.push_back(sample_2[i]);
	}
	std::sort(concat_data.begin(), concat_data.end());
	
	D max_difference = 0;
	for (int i = 0; i < concat_data.size(); i++)
	{
		if (abs(pmq_1.cdf(concat_data[i]) - pmq_2.cdf(concat_data[i])) > max_difference)
		{
			max_difference = abs(pmq_1.cdf(concat_data[i]) - pmq_2.cdf(concat_data[i]));
		}
	}


    return max_difference;
}


}  // namespace metric
#endif
