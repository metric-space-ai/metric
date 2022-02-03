#include "metric/utils/datasets.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

void register_wrapper_datasets(py::module& m) {
    using Class = Datasets;
    auto dim = py::class_<Class>(m, "Datasets");
    dim.def(py::init<>());
    dim.def("get_mnist", &Class::getMnist,
        py::arg("filename")
    );
}

void export_metric_datasets(py::module& m) {
    register_wrapper_datasets(m);
}