/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "../stl_wrappers.hpp"
#include "modules/distance/k-structured/EMD.hpp"
#include <boost/python.hpp>
#include <vector>

namespace py = boost::python;


void export_metric_EMD() {
    using V = int;
    using Container = std::vector<std::vector<V>>;
    using Vector = std::vector<V>;
    using Class = metric::EMD<V>;

    Container (*func1)(size_t,size_t) = &metric::EMD_details::ground_distance_matrix_of_2dgrid<V>;
    //Container (*func2)(Container) = &metric::EMD_details::ground_distance_matrix_of_2dgrid<V>;

    auto emd = py::class_<Class>("EMD", "Earth mover's distance");
    emd.def(py::init<const Container&>(
            (
                py::arg("C")
            )
        ));
    emd.def(py::init<const Container&, const V&>(
            (
                py::arg("C"),
                py::arg("extra_mass_penalty")
            )
        ));
    emd.def(py::init<std::size_t, std::size_t>(
            (
                py::arg("rows"),
                py::arg("cols")
            )
        ));
    emd.def(py::init<std::size_t, std::size_t, const V&>(
            (
                py::arg("rows"),
                py::arg("cols"),
                py::arg("extra_mass_penalty")
            )
        ));
    V (Class::*call)(const Vector&, const Vector&) const = &Class::operator()<Vector>;
    emd.def("__call__", call);
    emd.def("ground_distance_matrix_of_2dgrid", func1, (py::arg("cols"), py::arg("rows")));
    //emd.def("ground_distance_matrix_of_2dgrid", func2, (py::arg("grid"));
    emd.def("max_in_distance_matrix", metric::EMD_details::max_in_distance_matrix<Container>);
}



