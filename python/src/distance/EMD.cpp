/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/distance/k-structured/EMD.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <string>
#include <typeinfo>

namespace py = pybind11;

/**
So there are two types of classes:
- statically initialized templates (when algorithm known at compile time)
- generic implementation that calls function pointer, guaranteed: argument numbers and types, return type

In the future we can provide:
- generic scalar type
- automatic selection of scalar based on type of inputs (we don't need to create Euclidean(type=float), just Euclidean())

unfortunately we can't support multiple types at the same time because this will require manual work for each individual
class, with diving into implementation detail of that class (i.e. can it be copyable?, etc)


EMD(type=double) -> Any[EMD_int, EMD_double]

EMD_int -> EMD<int>
EMD_double -> EMD<double>

KOC(type=double) -> KOC_Euclidean_Grid4_Normal_double
KOC(metric=Metric(lambda x, y: sqrt(x - y)), type=int, distribution=MyDist()) -> KOC_Custom_Grid4_Custom_double

the problem when there is 2+ constructors, we need to have 2+ functions to create using each constructor.
we can use second approach with dynamic routing:
def KOC(type=int, *arg, **kwargs):
    impl[build_name(type)](*arg, **kwargs)


the problem with intermediate function is that docstring and autocomplete of arguments difficult to do with overloaded
constructors (something that happens on pybind11 side).

another problem with static methods (utils for classes)
we need to get proper static method of proper class, but we don't know proper class without call -> i.e constructing
is there a way to get class object without calling its constructor?
*/

template<typename Value>
void wrap_metric_EMD(py::module& m) {
    using Container = std::vector<std::vector<Value>>;
    using Vector = std::vector<Value>;
    using Class = metric::EMD<Value>;

    auto emd = py::class_<Class>(m, "EMD", "Earth mover's distance");
    emd.def(py::init<>(), "Default constructor");
    emd.def(py::init<Container&&>(), "Move constructor",
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
}

void export_metric_EMD(py::module& m) {
    wrap_metric_EMD<double>(m);
}



