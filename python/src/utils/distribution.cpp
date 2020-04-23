#include <pybind11/pybind11.h>

#include <random>

namespace py = pybind11;

template <typename Value>
void wrap_metric_distribution(py::module& m) {
    auto module = m.def_submodule("distribution");
    py::class_<std::uniform_real_distribution<Value>>(module, "Uniform", "Uniform distribution")
        .def(py::init<Value, Value>(),
            py::arg("a"),
            py::arg("b") = 1.0
        );

    py::class_<std::normal_distribution<Value>>(module, "Normal", "Gaussian distribution")
        .def(py::init<Value, Value>(),
            py::arg("mean"),
            py::arg("stddev") = 1.0
        );
}

void export_metric_distribution(py::module& m) {
    wrap_metric_distribution<double>(m);
}