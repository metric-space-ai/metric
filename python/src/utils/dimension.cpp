#include "modules/utils/crossfilter.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <string>
#include <vector>

namespace py = pybind11;

template <typename V, typename T>
void register_wrapper_dimension(py::module& m) {
    using Dimension = cross::dimension<V, T>;
    using Callback = py::object;
    auto dim = py::class_<Dimension>(m, "Dimension");
    dim.def("dispose", &Dimension::dispose);
    dim.def("get_offset", &Dimension::get_offset);
    dim.def("get_bit_index", &Dimension::get_bit_index);
    dim.def("filter_range", &Dimension::filter_range);
    dim.def("filter_exact", &Dimension::filter_exact);
    dim.def("filter_all", &Dimension::filter_all);
    dim.def("filter_with_predicate", &Dimension::filter_with_predicate);
    dim.def("filter_function", &Dimension::filter_function);

    dim.def("bottom", &Dimension::bottom);
    dim.def("top", &Dimension::top);
    dim.def("feature_all_count", &Dimension::feature_all_count);

    dim.def("feature", +[](Dimension& self, Callback& key){
        return self.feature(key);
    });
    dim.def("feature", +[](Dimension& self, Callback& add_func, Callback& remove_func, Callback& init_func){
        return self.feature(add_func, remove_func, init_func);
    });
    dim.def("feature", +[](Dimension& self, Callback& add_func, Callback& remove_func, Callback& init_func, Callback& key_func){
        return self.feature(add_func, remove_func, init_func, key_func);
    });
    dim.def("feature_count", +[](Dimension& self){
        return self.feature_count();
    });
    dim.def("feature_count", +[](Dimension& self, Callback& key_func){
        return self.feature_count(key_func);
    });
    dim.def("feature_sum", +[](Dimension& self, Callback& val_func){
        return self.feature_sum(val_func);
    });
    dim.def("feature_sum", +[](Dimension& self, Callback& val_func, Callback& key_func){
        return self.feature_sum(val_func, key_func);
    });
    dim.def("feature_all", +[](Dimension& self, Callback& add_func, Callback& remove_func, Callback& init_func){
        return self.feature_all(add_func, remove_func, init_func);
    });
    dim.def("feature_all_sum", +[](Dimension& self, Callback& val_func){
        return self.feature_all_sum(val_func);
    });
}


void export_metric_dimension(py::module& m) {
    register_wrapper_dimension<double, py::object>(m);
    register_wrapper_dimension<int, py::object>(m);
    register_wrapper_dimension<std::string, py::object>(m);
    register_wrapper_dimension<py::object, py::object>(m);
}