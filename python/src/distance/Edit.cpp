/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/distance/k-structured/Edit.hpp"

#include <boost/python.hpp>
#include <string>
#include <vector>

namespace py = boost::python;

template<typename Value>
void register_wrapper_edit() {
    using Metric = metric::Edit<Value>;
    auto p1 = &Metric::template operator()<std::basic_string_view<Value>>;
    auto p2 = &Metric::template operator()<std::vector<double>>;
    int (Metric::*p3)(const Value*, const Value*) const = &Metric::operator();
    py::class_<Metric>("Edit")
        .def("__call__", p1)
        .def("__call__", p2)
        .def("__call__", p3);
}

void export_metric_Edit() {
    register_wrapper_edit<char>();
}
