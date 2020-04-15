#include "modules/mapping/affprop.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

template <typename RecType, typename Metric>
void register_wrapper_affprop(py::module& m) {
    using Class = metric::AffProp<RecType, Metric>;
    using Value = typename Metric::distance_type;

    auto cls = py::class_<Class>(m, "AffProp", "Affinity Propagation");
    cls.def(py::init<>());
    cls.def(py::init<Value, int, Value, Value>(),
        py::arg("preference") = 0.5,
        py::arg("maxiter") = 200,
        py::arg("tol") = 1.0e-6,
        py::arg("damp") = 0.5
    );
    cls.def("__call__", &Class::operator(),
        py::arg("dm")
    );
}

void export_metric_affprop(py::module& m) {
    using Value = double;
    using Container = std::vector<Value>;
    register_wrapper_affprop<Container, metric::Euclidean<Value>>(m);
}

PYBIND11_MODULE(affprop, m) {
    export_metric_affprop(m);
}