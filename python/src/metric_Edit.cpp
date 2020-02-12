/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "metric_Edit.hpp"

namespace bp = boost::python;

void export_metric_Edit() {

bp::class_<metric_Edit::metricsEdit>("Edit")
      .def("__call__", +[](metric_Edit::metricsEdit& self, bp::object& A, bp::object& B) {
          return self.call(A,B);
      }, "return Edit distance between A and B")
      ;
}
