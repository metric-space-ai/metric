#include "modules/distance/k-structured/SSIM.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

template<typename DistanceType, typename Value>
void register_wrapper_SSIM(py::module& m) {
    using Metric = metric::SSIM<DistanceType, Value>;
    using ValueType = typename Value::value_type;
    using Container = typename std::vector<Value>;
    py::class_<Metric>(m, "SSIM")
        .def(py::init<>())
        .def(py::init<ValueType, ValueType>(), py::arg("dynamic_range"), py::arg("masking"))
        .def("__call__", &Metric::template operator()<Container>)
        .def_readonly("dynamic_range", &Metric::dynamic_range)
        .def_readonly("masking", &Metric::masking);
}

void export_metric_SSIM(py::module& m) {
    register_wrapper_SSIM<double, std::vector<double>>(m);
}
