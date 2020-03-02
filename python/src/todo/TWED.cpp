#include "modules/distance.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

template<typename ValueType>
void register_wrapper_TWED() {
    using Metric = metric::TWED<ValueType>;
    bp::class_<Metric>("TWED")
        .def(bp::init<ValueType, ValueType>((bp::arg("penalty_") = 0, bp::arg("elastic_") = 1)))
        .def("__call__", &Metric::template operator()<std::vector<double>>)
        .def("__call__", &Metric::template operator()<std::vector<int>>)
        .def("__call__", &Metric::template operator()<std::vector<unsigned long long>>);
}

void export_metric_TWED() {
    register_wrapper_TWED<double>();
}
