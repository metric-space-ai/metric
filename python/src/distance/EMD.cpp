/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "../stl_wrappers.hpp"
#include "modules/distance/k-structured/EMD.hpp"

namespace bp = boost::python;

class EMD_detailsScope {};

template <class T>
std::vector<std::vector<T>>(*ground_distance_matrix_of_2dgrid_1)(size_t,size_t) = &metric::EMD_details::ground_distance_matrix_of_2dgrid<T>;

template <class T>
std::vector<std::vector<T>>(*ground_distance_matrix_of_2dgrid_2)(std::vector<std::vector<T>>) = &metric::EMD_details::ground_distance_matrix_of_2dgrid<T>;


void export_metric_EMD() {

    bp::class_<metric::EMD<int>>("EMD")
        .def(bp::init<std::vector<std::vector<int>>>())
        .def(bp::init<std::size_t, std::size_t, bp::optional<const int&,std::vector<std::vector<int>>*>>())
        .def(bp::init<std::vector<std::vector<int>>, bp::optional<const int&,std::vector<std::vector<int>>*>>())

        .def("__call__", +[](metric::EMD<int>& self, bp::object& A, bp::object& B) {
            return self.operator()(WrapStlVector<int>(A), WrapStlVector<int>(B));
        });

    bp::scope EMD_details = bp::class_<EMD_detailsScope>("EMD_details")
        .def("ground_distance_matrix_of_2dgrid",
            ground_distance_matrix_of_2dgrid_1<int>,
            bp::return_value_policy<bp::return_by_value>())
        .def("max_in_distance_matrix",
            metric::EMD_details::max_in_distance_matrix<std::vector<std::vector<int>>>,
            bp::return_value_policy<bp::return_by_value>());
}



