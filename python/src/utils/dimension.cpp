#include "modules/utils/crossfilter.hpp"
#include <boost/python.hpp>
#include <string>
#include <vector>

namespace py = boost::python;

template <typename V, typename T>
void register_wrapper_dimension() {
    using Dimension = cross::dimension<V, T>;
    using Callback = py::object;
    auto dim = py::class_<Dimension>("Dimension");
    dim.def("dispose", &Dimension::dispose);
    dim.def("get_offset", &Dimension::dispose);
    dim.def("get_bit_index", &Dimension::get_bit_index);
    dim.def("filter_range", &Dimension::filter_range);
    dim.def("filter_exact", &Dimension::filter_exact);
    dim.def("filter_all", &Dimension::filter_all);
    dim.def("filter_with_predicate", &Dimension::filter_with_predicate);
    dim.def("filter_function", &Dimension::filter_function);

    dim.def("bottom", &Dimension::bottom);
    dim.def("top", &Dimension::top);
    dim.def("feature_all_count", &Dimension::feature_all_count);

    auto (Dimension::*feature1)(Callback) = &Dimension::feature;
    auto (Dimension::*feature2)(Callback, Callback, Callback) = &Dimension::feature;
    auto (Dimension::*feature3)(Callback, Callback, Callback, Callback) = &Dimension::feature;
    auto (Dimension::*feature_count1)() = &Dimension::feature_count;
    auto (Dimension::*feature_count2)(Callback) = &Dimension::feature_count;
    auto (Dimension::*feature_sum1)(Callback) = &Dimension::feature_sum;
    auto (Dimension::*feature_sum2)(Callback, Callback) = &Dimension::feature_sum;
    auto (Dimension::*feature_all)(Callback, Callback, Callback) = &Dimension::feature_all;
    auto (Dimension::*feature_all_sum)(Callback) = &Dimension::feature_sum;

    dim.def("feature", feature1);
    dim.def("feature", feature2);
    dim.def("feature", feature3);
    dim.def("feature_count", feature_count1);
    dim.def("feature_count", feature_count2);
    dim.def("feature_sum", feature_sum1);
    dim.def("feature_sum", feature_sum2);
    dim.def("feature_all", feature_all);
    dim.def("feature_all_sum", feature_all_sum);
}

void export_metric_dimension() {
    register_wrapper_dimension<std::string, std::vector<double>>();
    register_wrapper_dimension<int, std::vector<double>>();
}