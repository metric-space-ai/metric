/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_UTILS_VISUALIZER_HPP
#define _METRIC_UTILS_VISUALIZER_HPP

#include <string>
#include "blaze/Blaze.h"

namespace mat2bmp
{

/**
 * @brief 
 * 
 * @param m 
 * @param filename 
 */
template <class BlazeMatrix>
void blaze2bmp(BlazeMatrix m, std::string filename);

/**
 * @brief 
 * 
 * @param m 
 * @param filename 
 */
template <class BlazeMatrix>
void blaze2bmp_norm(BlazeMatrix m, std::string filename);

}


#include "visualizer.cpp"
#endif 
