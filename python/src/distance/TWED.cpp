#include "modules/distance/k-structured/TWED.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

template<typename ValueType>
void register_wrapper_TWED(py::module& m) {
    using Metric = metric::TWED<ValueType>;
    py::class_<Metric>(m, "TWED")
        .def(py::init<ValueType, ValueType>(), py::arg("penalty") = 0, py::arg("elastic") = 1)
        .def("__call__", &Metric::template operator()<std::vector<double>>)
        .def("__call__", &Metric::template operator()<std::vector<int>>)
        .def("__call__", &Metric::template operator()<std::vector<unsigned long long>>)
        .def_readonly("penalty", &Metric::penalty)
        .def_readonly("elastic", &Metric::elastic)
        .def_readonly("is_zero_padded", &Metric::is_zero_padded);
}

void export_metric_TWED(py::module& m) {
    register_wrapper_TWED<double>(m);
}
