/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

def("entropy", +[](bp::object& A, std::size_t k = 3, double logbase = 2.0, const char* metric = "eucludean") {
        return metric_Entropy::entropy(WrapStlMatrix<double>(A), k, logbase, metric);
    }, "Continuous entropy estimator");

