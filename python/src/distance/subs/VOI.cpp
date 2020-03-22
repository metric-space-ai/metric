/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/distance/k-random/VOI.hpp"
#include "../../metric_types.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <string>

namespace py = pybind11;

template<typename Value>
void wrap_metric_VOI_kl(py::module& m) {
    using Metric = metric::VOI_kl<Value>;
    using Container = std::vector<std::vector<Value>>;
    py::class_<Metric>(m, "VOI_kl")
        .def(py::init<int, Value>(), py::arg("k") = 3, py::arg("logbase") = 2)
        .def("__call__", +[](Metric& self, const Container& a, const Container& b) {
            return self(a, b);
        }, "Calculate variation of information based on Kozachenko-Leonenko entropy estimator");
}

template<typename Value>
void wrap_metric_VOI_normalized(py::module& m) {
    using Metric = metric::VOI_normalized<Value>;
    using Container = std::vector<std::vector<Value>>;
    py::class_<Metric>(m, "VOI_normalized")
        .def(py::init<int, Value>(), py::arg("k") = 3, py::arg("logbase") = 2)
        .def("__call__", +[](Metric& self, const Container& a, const Container& b) {
            return self(a, b);
        }, "Calculate Variation of Information");
}

void export_metric_VOI(py::module& m) {
    wrap_metric_VOI_normalized<double>(m);
    wrap_metric_VOI_kl<double>(m);
}

template <typename Container, typename Metric>
void wrap_metric_entropy(py::module& m) {
    using Value = double ;//typename Container::value_type::value_type;
    std::string name = "entropy_" + getMetricName<Metric>();
    m.def(name.c_str(), +[](const Container& data, std::size_t k = 3, Value logbase = 2) {
            return metric::entropy<Container, Metric, Value>(data, k, logbase);
        },
        "Continuous entropy estimator",
        py::arg("data"), py::arg("k") = 3, py::arg("logbase") = 2
    );
}

void export_metric_entropy(py::module& m) {
    using Value = double;
    using Container = std::vector<std::vector<Value>>;
    wrap_metric_entropy<Container, metric::Euclidian<Value>>(m);
    wrap_metric_entropy<Container, metric::Manhatten<Value>>(m);
    wrap_metric_entropy<Container, metric::Chebyshev<Value>>(m);
    wrap_metric_entropy<Container, metric::P_norm<Value>>(m);
}

PYBIND11_MODULE(_voi, m) {
    export_metric_VOI(m);
    export_metric_entropy(m);
}