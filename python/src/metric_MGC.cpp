/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

bp::class_<metric_MGC::metrics>("MGC", bp::init<std::string,std::string>())
    .def("estimate", +[](metric_MGC::metrics& self, bp::object& A, bp::object& B, const size_t BsampleSize = 250, const double threshold = 0.05, size_t maxIterations = 1000) {
        return self.mgc->estimate(python_wrap_stl(A), python_wrap_stl(B), BsampleSize, threshold, maxIterations);
    }, "return estimate of the correlation betweeen A and B")
    .def("__call__", +[](metric_MGC::metrics& self, bp::object& A, bp::object& B) {
        return self.mgc->operator()(python_wrap_stl(A), python_wrap_stl(B));
    }, "return correlation betweeen A and B")
    ;

