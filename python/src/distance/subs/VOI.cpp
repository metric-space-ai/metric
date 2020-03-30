/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/distance/k-random/VOI.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;
//

template<typename Value>
void wrap_metric_VOI_normalized(py::module& m) {
    using Metric = metric::VOI_normalized<Value>;
    using Container = std::vector<std::vector<Value>>;
    py::class_<Metric>(m, "VOI_normalized", "Normalized Variation of Information")
        .def(py::init<int, Value>(),
            "Construct",
            py::arg("k") = 3,
            py::arg("logbase") = 2
        )
        .def("__call__", &Metric::template operator()<Container>,
            "Calculate Variation of Information",
            py::arg("a"),
            py::arg("b")
        );
}

void export_metric_VOI(py::module& m) {
    wrap_metric_VOI_normalized<double>(m);
}

PYBIND11_MODULE(voi, m) {
    export_metric_VOI(m);
}
