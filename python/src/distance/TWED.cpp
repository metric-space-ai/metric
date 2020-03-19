#include "modules/distance/k-structured/TWED.hpp"

#include <boost/python.hpp>
#include <vector>

namespace py = boost::python;

template<typename ValueType>
void register_wrapper_TWED() {
    using Metric = metric::TWED<ValueType>;
    py::class_<Metric>("TWED")
        .def(py::init<ValueType, ValueType>((py::arg("penalty") = 0, py::arg("elastic") = 1)))
        .def("__call__", &Metric::template operator()<std::vector<double>>)
        .def("__call__", &Metric::template operator()<std::vector<int>>)
        .def("__call__", &Metric::template operator()<std::vector<unsigned long long>>)
        .def_readonly("penalty", &Metric::penalty)
        .def_readonly("elastic", &Metric::elastic)
        .def_readonly("is_zero_padded", &Metric::is_zero_padded);
}

void export_metric_TWED() {
    register_wrapper_TWED<double>();
}
