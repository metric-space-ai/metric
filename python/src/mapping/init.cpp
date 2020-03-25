/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/
#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_metric_autoencoder(py::module& m);
void export_metric_DSPCC(py::module& m);
void export_metric_kmeans(py::module& m);
void export_metric_kmedoids(py::module& m);

PYBIND11_MODULE(_mapping, m) {
    export_metric_autoencoder(m);
    export_metric_DSPCC(m);
    export_metric_kmeans(m);
    export_metric_kmedoids(m);
}
