/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Michael Welsch
*/

#ifndef _METRIC_DISTANCE_POTENTIAL_MINIMIZATION_CPP
#define _METRIC_DISTANCE_POTENTIAL_MINIMIZATION_CPP

#include <tuple>
#include "../../3rdparty/blaze/Blaze.h"

namespace metric {


std::vector<double> fit_hysteresis(blaze::DynamicVector<double> x, blaze::DynamicVector<double> y,
size_t grid_row, 
size_t grid_column,
size_t steps = 100, std::vector<double> sigma = {50,30,15,5});

}  // end metric namespace

#include "distance_potential_minimization.cpp"
#endif
