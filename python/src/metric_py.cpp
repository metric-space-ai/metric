/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/correlation.hpp"
#include "modules/distance.hpp"
#include "metric.hpp"

#include "metric_MGC.hpp"
#include "metric_Edit.hpp"
#include "metric_Entropy.hpp"
#include "metric_EMD.hpp"

BOOST_PYTHON_MODULE(metric)
{

#include "metric_MGC.cpp"
#include "metric_Edit.cpp"
#include "metric_Entropy.cpp"
#include "metric_EMD.cpp"

}

