#include "modules/distance/k-related/Standards.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

template<typename Value, typename Container>
void register_wrapper_euclidean(py::module& m) {
    using Metric = metric::Euclidian<Value>;
    auto p1 = &Metric::template operator()<Container>;
    Value (Metric::*p2)(const Value&, const Value&) const = &Metric::operator();
    py::class_<Metric>(m, "Euclidean")
        .def(py::init<>())
        .def("__call__", p1)
        .def("__call__", p2);
}

template<typename Value, typename Container>
void register_wrapper_manhatten(py::module& m) {
    using Metric = metric::Manhatten<Value>;
    py::class_<Metric>(m, "Manhatten")
        .def(py::init<>())
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_pnorm(py::module& m) {
    using Metric = metric::P_norm<Value>;
    py::class_<Metric>(m, "P_norm")
        .def(py::init<Value>(), py::arg("p") = 1)
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_euclidean_thresholded(py::module& m) {
    using Metric = metric::Euclidian_thresholded<Value>;
    py::class_<Metric>(m, "Euclidean_thresholded")
        .def(py::init<>())
        .def(py::init<Value, Value>(), py::arg("thres"), py::arg("factor"))
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_cosine(py::module& m) {
    using Metric = metric::Cosine<Value>;
    py::class_<Metric>(m, "Cosine")
        .def(py::init<>())
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_chebyshev(py::module& m) {
    using Metric = metric::Chebyshev<Value>;
    py::class_<Metric>(m, "Chebyshev")
        .def(py::init<>())
        .def("__call__", &Metric::template operator()<Container>);
}

void export_metric_standards(py::module& m) {
    register_wrapper_euclidean<double, std::vector<double>>(m);
    register_wrapper_manhatten<double, std::vector<double>>(m);
    register_wrapper_pnorm<double, std::vector<double>>(m);
    register_wrapper_euclidean_thresholded<double, std::vector<double>>(m);
    register_wrapper_cosine<double, std::vector<double>>(m);
    register_wrapper_chebyshev<double, std::vector<double>>(m);
}