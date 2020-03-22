/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/
#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_metric_matrix(py::module& m);
void export_metric_Tree(py::module& m);

PYBIND11_MODULE(_space, m) {
    export_metric_matrix(m);
    export_metric_Tree(m);
}
