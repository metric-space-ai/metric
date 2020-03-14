#include "modules/distance/k-structured/TWED.hpp"

#include <boost/python.hpp>
#include <vector>

namespace py = boost::python;

template<typename ValueType>
void register_wrapper_TWED() {
    using Metric = metric::TWED<ValueType>;
    py::class_<Metric>("TWED")
        .def(py::init<ValueType, ValueType>((py::arg("penalty_") = 0, py::arg("elastic_") = 1)))
        .def("__call__", &Metric::template operator()<std::vector<double>>)
        .def("__call__", &Metric::template operator()<std::vector<int>>)
        .def("__call__", &Metric::template operator()<std::vector<unsigned long long>>);
}

void export_metric_TWED() {
    register_wrapper_TWED<double>();
}
