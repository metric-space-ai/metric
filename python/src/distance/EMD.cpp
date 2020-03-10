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

namespace bp = boost::python;


void export_metric_EMD() {
    using V = double;
    using Container = std::vector<std::vector<V>>;
    using Class = metric::EMD<V>;

    Container (*func1)(size_t,size_t) = &metric::EMD_details::ground_distance_matrix_of_2dgrid<V>;
    Container (*func2)(Container) = &metric::EMD_details::ground_distance_matrix_of_2dgrid<V>;

    bp::class_<Class>("EMD")
        .def(bp::init<const Container&>())
        .def(bp::init<std::size_t, std::size_t, bp::optional<const V&, Container*>>())
        .def(bp::init<const Container&, bp::optional<const V&, Container*>>())

        .def("__call__", +[](Class& self, bp::object& A, bp::object& B) {
            return self.operator()(WrapStlVector<V>(A), WrapStlVector<V>(B));
        })
        .def("ground_distance_matrix_of_2dgrid", func1)
        .def("ground_distance_matrix_of_2dgrid", func2)
        .def("max_in_distance_matrix", metric::EMD_details::max_in_distance_matrix<Container>)
        ;
}



