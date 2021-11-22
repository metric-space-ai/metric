/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/distance/k-random/RandomEMD.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <string>
#include <typeinfo>

namespace py = pybind11;


template<typename Sample, typename D = double>
void wrap_metric_RandomEMD(py::module& m) {

	using Class = metric::RandomEMD<Sample, D>;
    auto emd = py::class_<Class>(m, "RandomEMD", "Random Earth mover's distance");
    emd.def(py::init<>(), "Default constructor");
	emd.def(py::init<double>(),
			py::arg("precision")
	);
	D (Class::*call)(const Sample&, const Sample&) const = &Class::template operator()<Sample>;
    emd.def("__call__", call);
}

void export_metric_RandomEMD(py::module& m) {
    wrap_metric_RandomEMD<std::vector<double>, double>(m);
}



