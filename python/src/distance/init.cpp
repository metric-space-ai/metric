/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/
#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_metric_EMD(py::module& m);
void export_metric_kohonen(py::module& m);
void export_metric_sorensen(py::module& m);
void export_metric_SSIM(py::module& m);
void export_metric_standards(py::module& m);
void export_metric_TWED(py::module& m);
void export_metric_Edit(py::module& m);

PYBIND11_MODULE(_distance, m) {
    export_metric_EMD(m);
    export_metric_kohonen(m);
    export_metric_sorensen(m);
    export_metric_SSIM(m);
    export_metric_standards(m);
    export_metric_TWED(m);
    export_metric_Edit(m);
}
