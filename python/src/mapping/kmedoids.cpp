#include "modules/mapping/kmedoids.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

template <typename RecType, typename Metric>
void register_wrapper_kmedoids(py::module& m) {
    m.def("kmedoids", &metric::kmedoids<RecType, Metric>,
        py::arg("dm"),
        py::arg("k")
    );
}

void export_metric_kmedoids(py::module& m) {
    register_wrapper_kmedoids<std::vector<double>, metric::Euclidean<double>>(m);
}
