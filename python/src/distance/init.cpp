/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/
#include <boost/python.hpp>

namespace py = boost::python;

void export_metric_EMD();
void export_metric_kohonen();
void export_metric_sorensen();
void export_metric_SSIM();
void export_metric_standards();
void export_metric_TWED();
void export_metric_Edit();

BOOST_PYTHON_MODULE(_distance) {
    export_metric_EMD();
    export_metric_kohonen();
    export_metric_sorensen();
    export_metric_SSIM();
    export_metric_standards();
    export_metric_TWED();
    export_metric_Edit();
}
