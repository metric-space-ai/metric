/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/distance/k-random/RandomEMD.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <string>
#include <typeinfo>

namespace py = pybind11;


template<typename Value>
void wrap_metric_RandomEMD(py::module& m) {
    using Container = std::vector<std::vector<Value>>;
    using Vector = std::vector<Value>;
    using Class = metric::RandomEMD<Value>;

    auto emd = py::class_<Class>(m, "RandomEMD", "Random Earth mover's distance");
    emd.def(py::init<>(), "Default constructor");
    /*emd.def(py::init<Container&&>(), "Move constructor",
        py::arg("cost_matrix")
    );
    emd.def(py::init<const Container&, const Value&>(),
        py::arg("cost_matrix"),
        py::arg("extra_mass_penalty") = -1
    );
    emd.def(py::init<std::size_t, std::size_t, const Value&>(),
        py::arg("rows"),
        py::arg("cols"),
        py::arg("extra_mass_penalty") = -1
    );
    Value (Class::*call)(const Vector&, const Vector&) const = &Class::template operator()<Vector>;
    emd.def("__call__", call);
    Container (*func1)(size_t,size_t) = &metric::EMD_details::ground_distance_matrix_of_2dgrid<Value>;
    //Container (*func2)(Container) = &metric::EMD_details::ground_distance_matrix_of_2dgrid<Value>;
    emd.def_static("ground_distance_matrix_of_2dgrid", func1, py::arg("cols"), py::arg("rows"));
    //emd.def("ground_distance_matrix_of_2dgrid", func2, (py::arg("grid"));
    emd.def_static("max_in_distance_matrix", metric::EMD_details::max_in_distance_matrix<Container>);
*/}

void export_metric_RandomEMD(py::module& m) {
    wrap_metric_RandomEMD<double>(m);
}



