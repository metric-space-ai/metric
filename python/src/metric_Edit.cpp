/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "metric_py.hpp"
#include "modules/distance.hpp"

namespace bp = boost::python;

class NotUsed {};
class metricsEdit {
    metric::Edit<NotUsed> metricEdit;

public:
    int call(const WrapStlMatrix<double>& A, const WrapStlMatrix<double>& B) {
        return metricEdit(A,B);
    }
};

void export_metric_Edit() {

    bp::class_<metricsEdit>("Edit")
        .def("__call__", +[](metricsEdit& self, bp::object& A, bp::object& B) {
            return self.call(A,B);
        },
    "return Edit distance between A and B");
}
