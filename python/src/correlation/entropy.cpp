/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/correlation/entropy.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <vector>
#include <string>
#include <functional>

namespace py = pybind11;


template <typename Container, typename Metric>
void wrap_metric_entropy(py::module& m) {
    using Value = double ;
    std::string name = "entropy";
    m.def(name.c_str(), &metric::entropy<Container, Metric>,
        "Continuous entropy estimator",
        py::arg("data"),
        py::arg("k") = 3,
        py::arg("logbase") = 2,
        py::arg("metric") = Metric(),
        py::arg("exp") = false
    );
}

void export_metric_entropy(py::module& m) {
    using Value = double;
    using RecType = std::vector<Value>;
    using Container = std::vector<RecType>;
    wrap_metric_entropy<Container, metric::Euclidian<Value>>(m);
    wrap_metric_entropy<Container, metric::Manhatten<Value>>(m);
    wrap_metric_entropy<Container, metric::Chebyshev<Value>>(m);
    wrap_metric_entropy<Container, metric::P_norm<Value>>(m);
    wrap_metric_entropy<Container, std::function<double(const RecType&, const RecType&)>>(m);
}


PYBIND11_MODULE(_entropy, m) {
    export_metric_entropy(m);
}
