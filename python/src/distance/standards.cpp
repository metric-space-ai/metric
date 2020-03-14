#include "modules/distance/k-related/Standards.hpp"

#include <boost/python.hpp>
#include <vector>

namespace py = boost::python;

template<typename Value, typename Container>
void register_wrapper_euclidean() {
    using Metric = metric::Euclidian<Value>;
    auto p1 = &Metric::template operator()<Container>;
    Value (Metric::*p2)(const Value&, const Value&) const = &Metric::operator();
    py::class_<Metric>("Euclidean")
        .def("__call__", p1)
        .def("__call__", p2);
}

template<typename Value, typename Container>
void register_wrapper_manhatten() {
    using Metric = metric::Manhatten<Value>;
    py::class_<Metric>("Manhatten")
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_pnorm() {
    using Metric = metric::P_norm<Value>;
    py::class_<Metric>("P_norm", py::init<Value>(py::arg("p") = 1))
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_euclidean_thresholded() {
    using Metric = metric::Euclidian_thresholded<Value>;
    py::class_<Metric>("Euclidean_thresholded", py::init())
        .def(py::init<Value, Value>((py::arg("thres"), py::arg("factor"))))
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_cosine() {
    using Metric = metric::Cosine<Value>;
    py::class_<Metric>("Cosine")
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_chebyshev() {
    using Metric = metric::Chebyshev<Value>;
    py::class_<Metric>("Chebyshev")
        .def("__call__", &Metric::template operator()<Container>);
}

void export_metric_standards() {
    register_wrapper_euclidean<double, std::vector<double>>();
    register_wrapper_manhatten<double, std::vector<double>>();
    register_wrapper_pnorm<double, std::vector<double>>();
    register_wrapper_euclidean_thresholded<double, std::vector<double>>();
    register_wrapper_cosine<double, std::vector<double>>();
    register_wrapper_chebyshev<double, std::vector<double>>();
}