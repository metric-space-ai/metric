#include "modules/utils/datasets.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <tuple>

namespace py = pybind11;

void register_wrapper_datasets(py::module& m) {
    using Class = Datasets;
    auto dim = py::class_<Class>(m, "Datasets");
    dim.def("get_mnist", +[](Class& self, const std::string& filename){
        auto result = self.getMnist(filename);
        return py::make_tuple(std::get<0>(result), std::get<1>(result), std::get<2>(result));
    });
}

void export_metric_datasets(py::module& m) {
    register_wrapper_datasets(m);
}