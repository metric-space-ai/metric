/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include "RandomEMD.hpp"
#include <metric/utils/poor_mans_quantum.hpp>

namespace mtrc {

template <typename Sample, typename D>
auto RandomEMD<Sample, D>::operator()(const Sample &sample_1, const Sample &sample_2) const -> distance_type
{
	PMQ pmq_1(sample_1);
	PMQ pmq_2(sample_2);

	// find the area between distributions

	Sample concat_data;
	for (int i = 0; i < sample_1.size(); i++) {
		concat_data.push_back(sample_1[i]);
	}
	for (int i = 0; i < sample_2.size(); i++) {
		concat_data.push_back(sample_2[i]);
	}
	std::sort(concat_data.begin(), concat_data.end());

	//

	D area = 0;

	// Degenerate-input guards (this is the documented quarantine footgun):
	//  - both samples empty -> concat_data is empty -> concat_data[size()-1] would
	//    read out of bounds (crash). There is no mass to move, so the distance is 0.
	//  - all values identical (zero range) -> step == 0 -> the integration loop below
	//    never advances (infinite hang). The integral over a zero-width support is 0.
	if (concat_data.empty()) {
		return area;
	}
	double step = (concat_data[concat_data.size() - 1] - concat_data[0]) * precision;
	if (step <= 0) {
		return area;
	}

	for (double value = concat_data[0]; value <= concat_data[concat_data.size() - 1]; value += step) {
		area += abs(pmq_1.cdf(value) - pmq_2.cdf(value)) * step;
	}

	return area;
}

} // namespace mtrc
