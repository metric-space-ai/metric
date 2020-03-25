#include "modules/mapping/Redif.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

template <typename Value, typename Metric>
void register_wrapper_Redif(py::module& m) {
    using Class = metric::Redif<Value>;
    using Container = std::vector<std::vector<Value>>;

    auto cls = py::class_<Class>(m, "Redif");
    cls.def(py::init<const Container&, size_t, size_t, Metric>(),
        py::arg("train_data"),
        py::arg("n_neighbors") = 10,
        py::arg("n_iter") = 15,
        py::arg("metric") = Metric()
    );
    Matrix (Class::*encode) (const std::vector<recType>&) = &Class::encode;
    cls.def("encode", encode);
    cls.def("decode", &Class::decode);
}

void export_metric_Redif(py::module& m) {
    using Value = double;
    register_wrapper_Redif<Value, metric::Euclidean<Value>>(m);
}
