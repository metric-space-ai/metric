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

template<typename Container, typename Metric>
double entropy(const Container& data, Metric metric, size_t k, size_t p, bool exp)
{
    auto instance = metric::entropy<double, Metric>(metric, k, p, exp);
    return instance(data);
}


template <typename Container, typename Metric>
void wrap_metric_entropy(py::module& m) {
    m.def("entropy", &entropy<Container, Metric>,
        "internal function to create instantiate template Entropy classes",
        py::arg("data"),
        py::arg("metric"),
        py::arg("k"),
        py::arg("p"),
        py::arg("exp")
    );
    m.def("estimate", &metric::entropy_details::estimate<Container>);
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


PYBIND11_MODULE(entropy, m) {
    export_metric_entropy(m);
}
