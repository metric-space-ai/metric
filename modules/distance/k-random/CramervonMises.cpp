/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include "CramervonMises.hpp"
#include "../../utils/poor_mans_quantum.hpp"

namespace metric {

template <typename Sample, typename D>
auto CramervonMises<Sample, D>::operator()(const Sample& sample_1, const Sample& sample_2) const -> distance_type
{
	PMQ pmq_1(sample_1);
	PMQ pmq_2(sample_2);
		
	// find the sum of squared differences between the two cdfs

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

	//
	
	D area = 0;
	double step = (concat_data[concat_data.size() - 1] - concat_data[0]) * precision;

	for (double value = concat_data[0]; value <= concat_data[concat_data.size() - 1]; value += step)
	{
		area += (pmq_1.cdf(value) - pmq_2.cdf(value)) * (pmq_1.cdf(value) - pmq_2.cdf(value)) * step;
	}

    return sqrt(area);
}


}  // namespace metric
