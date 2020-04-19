/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/
#include "metric_types.hpp"

#include "modules/correlation/entropy.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <vector>
#include <string>

namespace py = pybind11;

template <typename Metric = metric::Euclidean<double>()>
metric::Entropy<void, Metric> createEntropy(
    const Metric& metric,
    size_t k = 7,
    size_t p = 70,
    bool exp = false
){
    return metric::Entropy<void, Metric>(metric, k, p, exp);
}

template <typename Container, typename Metric>
void wrap_metric_entropy(py::module& m) {
    using Class = metric::Entropy<void, Metric>;

    m.def("Entropy", &createEntropy<Metric>,
        "Factory of Entropy instances",
        py::arg("metric"),
        py::arg("k") = 7,
        py::arg("p") = 70,
        py::arg("exp") = false
    );

    const std::string name = std::string("Entropy_") + metric::getTypeName<Metric>();
    auto cls = py::class_<Class>(m, name.c_str());
    cls.def("__call__", &Class::template operator()<Container>,
        "Calculate entropy",
        py::arg("data")
    );

    cls.def("estimate", &Class::template estimate<Container>,
        "Estimate",
        py::arg("data"),
        py::arg("sample_size") = 250,
        py::arg("threshold") = 0.05,
        py::arg("max_iterations") = 100
    );
}


void export_metric_entropy(py::module& m) {
    using Value = double;
    using RecType = std::vector<Value>;
    using Container = std::vector<RecType>;
    using Functor = std::function<Value(const Container&, const Container&)>;

    boost::mpl::for_each<metric::MetricTypes, boost::mpl::make_identity<boost::mpl::_1>>([&](auto metr) {
        using MetricType = typename decltype(metr)::type;
        wrap_metric_entropy<Container, MetricType>(m);
    });

//    wrap_metric_entropy<Container, Functor>(m);
}


PYBIND11_MODULE(entropy, m) {
    export_metric_entropy(m);
}
