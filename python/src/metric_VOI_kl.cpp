/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "metric_py.hpp"
#include "metric_VOI.hpp"

namespace bp = boost::python;

void export_metric_VOI_kl() {

bp::class_<metric::VOI_kl<double>>("VOI_kl", bp::init<int,double>()) //TODO defaults for arguments
    .def("__call__", +[](metric::VOI_kl<double>& self, bp::object& A, bp::object& B) {
        auto vecA = WrapStlVector<double>(A);
        auto vecB = WrapStlVector<double>(B);
        auto matA = WrapStlVector<WrapStlVector<double>>(vecA);
        auto matB = WrapStlVector<WrapStlVector<double>>(vecB);
        return self.operator()(matA, matB);
    }, "Calculate variation of information based on Kozachenko-Leonenko entropy estimator")
    ;
}

