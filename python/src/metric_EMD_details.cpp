/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "metric_py.hpp"
#include "metric_EMD.hpp"

namespace bp = boost::python;

void export_metric_EMD_details() {

bp::scope EMD_details = bp::class_<metric_EMD::EMD_detailsScope>("EMD_details")
    .def("ground_distance_matrix_of_2dgrid", metric_EMD::ground_distance_matrix_of_2dgrid_1<int>
            , bp::return_value_policy<bp::return_by_value>())
    .def("max_in_distance_matrix", metric::EMD_details::max_in_distance_matrix<std::vector<std::vector<int>>>
            , bp::return_value_policy<bp::return_by_value>())
    ;

}
