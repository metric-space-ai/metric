#include "modules/distance.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

template<typename Value, typename Container>
void register_wrapper_euclidian() {
    using Metric = metric::Euclidian<Value>;
    auto p1 = &Metric::template operator()<Container>;
    Value (Metric::*p2)(const Value&, const Value&) const = &Metric::operator();
    bp::class_<Metric>("Euclidian")
        .def("__call__", p1)
        .def("__call__", p2);
}

template<typename Value, typename Container>
void register_wrapper_manhatten() {
    using Metric = metric::Manhatten<Value>;
    bp::class_<Metric>("Manhatten")
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_pnorm() {
    using Metric = metric::P_norm<Value>;
    bp::class_<Metric>("P_norm", bp::init<Value>(bp::arg("p_") = 1))
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_euclidian_thresholded() {
    using Metric = metric::Euclidian_thresholded<Value>;
    bp::class_<Metric>("Euclidian_thresholded", bp::init<Value, Value>((bp::arg("thres_"), bp::arg("factor_"))))
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_cosine() {
    using Metric = metric::Cosine<Value>;
    bp::class_<Metric>("Cosine")
        .def("__call__", &Metric::template operator()<Container>);
}

template<typename Value, typename Container>
void register_wrapper_chebyshev() {
    using Metric = metric::Chebyshev<Value>;
    bp::class_<Metric>("Chebyshev")
        .def("__call__", &Metric::template operator()<Container>);
}

void export_metric_standards() {
    register_wrapper_euclidian<double, std::vector<double>>();
    register_wrapper_manhatten<double, std::vector<double>>();
    register_wrapper_pnorm<double, std::vector<double>>();
    register_wrapper_euclidian_thresholded<double, std::vector<double>>();
    register_wrapper_cosine<double, std::vector<double>>();
    register_wrapper_chebyshev<double, std::vector<double>>();
}