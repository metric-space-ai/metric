/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_TRANSFORM_DISCRETE_COSINE_HPP
#define _METRIC_TRANSFORM_DISCRETE_COSINE_HPP

namespace metric {

/**
 * @brief apply forward or invese DCT depending on bool flag
 * 
 * @param Slices 
 * @param inverse 
 * @return true 
 * @return false 
 */

template <class BlazeMatrix>
bool apply_DCT(BlazeMatrix& Slices, bool inverse = false);


}
#include "discrete_cosine.cpp"
#endif

