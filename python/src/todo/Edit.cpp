#include "metric_py.hpp"
#include "modules/distance.hpp"

#include <string>
#include <vector>

namespace bp = boost::python;

template<typename Value>
void register_wrapper_edit() {
    using Metric = metric::Edit<Value>;
    auto p1 = &Metric::template operator()<std::basic_string_view<Value>>;
    auto p2 = &Metric::template operator()<std::vector<double>>;
    int (Metric::*p3)(const Value*, const Value*) const = &Metric::operator();
    bp::class_<Metric>("Edit")
        .def("__call__", p1)
        .def("__call__", p2)
        .def("__call__", p3);
}

void export_metric_Edit() {
    register_wrapper_edit<char>();
}
