/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "metric_EMD.hpp"

namespace bp = boost::python;

void export_metric_EMD() {

bp::class_<metric::EMD<int>>("EMD")
    .def(bp::init<std::vector<std::vector<int>>>())
    .def(bp::init<std::size_t, std::size_t, bp::optional<const int&,std::vector<std::vector<int>>*>>())
    .def(bp::init<std::vector<std::vector<int>>, bp::optional<const int&,std::vector<std::vector<int>>*>>())

    .def("__call__", +[](metric::EMD<int>& self, bp::object& A, bp::object& B) {
        return self.operator()(WrapStlVector<int>(A), WrapStlVector<int>(B));
    });
}

