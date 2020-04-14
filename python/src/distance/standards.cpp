#include "modules/distance/k-related/Standards.hpp"
#include "../stl_wrappers.hpp"
#include "custom.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>

namespace py = pybind11;

template<typename Value, typename Container>
void register_wrapper_euclidean(py::module& m) {
    using Metric = metric::Euclidean<Value>;
    auto p1 = &Metric::template operator()<Container>;
    Value (Metric::*p2)(const Value&, const Value&) const = &Metric::operator();
    py::class_<Metric>(m, "Euclidean")
        .def(py::init<>())
        .def("__call__", p1)
        .def("__call__", p2)
        .def("__call__", &Metric::template operator()<std::vector<Value>>)
        .def("__repr__", [](const Metric &a) { return "<Euclidean>"; });
}

template<typename Value, typename Container>
void register_wrapper_manhatten(py::module& m) {
    using Metric = metric::Manhatten<Value>;
    py::class_<Metric>(m, "Manhatten")
        .def(py::init<>())
        .def("__call__", &Metric::template operator()<Container>)
        .def("__call__", &Metric::template operator()<std::vector<Value>>)
        .def("__repr__", [](const Metric &a) { return "<Manhatten>"; });
}

template<typename Value, typename Container>
void register_wrapper_pnorm(py::module& m) {
    using Metric = metric::P_norm<Value>;
    py::class_<Metric>(m, "P_norm")
        .def(py::init<Value>(), py::arg("p") = 1)
        .def("__call__", &Metric::template operator()<Container>)
        .def("__call__", &Metric::template operator()<std::vector<Value>>)
        .def("__repr__", [](const Metric &a) { return "<P_norm>"; });
}

template<typename Value, typename Container>
void register_wrapper_euclidean_thresholded(py::module& m) {
    using Metric = metric::Euclidean_thresholded<Value>;
    py::class_<Metric>(m, "Euclidean_thresholded")
        .def(py::init<>())
        .def(py::init<Value, Value>(), py::arg("thres"), py::arg("factor"))
        .def("__call__", &Metric::template operator()<Container>)
        .def("__call__", &Metric::template operator()<std::vector<Value>>)
        .def("__repr__", [](const Metric &a) { return "<Euclidean_thresholded>"; });
}

template<typename Value, typename Container>
void register_wrapper_cosine(py::module& m) {
    using Metric = metric::Cosine<Value>;
    py::class_<Metric>(m, "Cosine")
        .def(py::init<>())
        .def("__call__", &Metric::template operator()<Container>)
        .def("__call__", &Metric::template operator()<std::vector<Value>>)
        .def("__repr__", [](const Metric &a) { return "<Cosine>"; });
}

template<typename Value, typename Container>
void register_wrapper_chebyshev(py::module& m) {
    using Metric = metric::Chebyshev<Value>;
    py::class_<Metric>(m, "Chebyshev")
        .def(py::init<>())
        .def("__call__", &Metric::template operator()<Container>)
        .def("__call__", &Metric::template operator()<std::vector<Value>>)
        .def("__repr__", [](const Metric &a) { return "<Chebyshev>"; });
}

//template<typename Value, typename Container>
//void register_wrapper_custom(py::module& m) {
//    using Metric = metric::PythonMetric<Container, Value>;
//    using Callable = typename Metric::Callable;
//    py::class_<Metric>(m, "Metric")
//        .def(py::init<>());
//        .def(py::init<const Callable&>());
//        .def("__call__", &Metric::template operator()<Container>);
//        .def("__repr__", [](const Metric &a) { return "<Metric>"; });
//}

void export_metric_standards(py::module& m) {
    register_wrapper_euclidean<double, NumpyToVectorAdapter<double>>(m);
    register_wrapper_manhatten<double, NumpyToVectorAdapter<double>>(m);
    register_wrapper_pnorm<double, NumpyToVectorAdapter<double>>(m);
    register_wrapper_euclidean_thresholded<double, NumpyToVectorAdapter<double>>(m);
    register_wrapper_cosine<double, NumpyToVectorAdapter<double>>(m);
    register_wrapper_chebyshev<double, NumpyToVectorAdapter<double>>(m);
//    register_wrapper_custom<double, NumpyToVectorAdapter<double>>(m);
}