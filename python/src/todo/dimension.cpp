#include "modules/utils/crossfilter.hpp"
#include <boost/python.hpp>
#include <string>

namespace bp = boost::python;

template <typename V, typename T>
void register_wrapper_crossfilter() {
    using Dimension = cross::dimension<V, T>;
    auto dim = bp::class_<Dimension>("Dimension");
    dim.def("dispose", &Dimension::dispose);
    dim.def("get_offset", &Dimension::dispose);
    dim.def("get_bit_index", &Dimension::get_bit_index);
    dim.def("filter_range", &Dimension::filter_range);
    dim.def("filter_exact", &Dimension::filter_exact);
    dim.def("filter_all", &Dimension::filter_all);
    dim.def("filter_with_predicate", &Dimension::filter_with_predicate);
    dim.def("filter_function", &Dimension::filter_function);

    dim.def("filter", &Dimension::filter);
    dim.def("filter", &Dimension::filter);
    dim.def("filter", &Dimension::filter);
    dim.def("filter", &Dimension::filter);

    dim.def("bottom", &Dimension::bottom);
    dim.def("top", &Dimension::top);
    dim.def("feature_count", &Dimension::feature_count);
    dim.def("feature_all_count", &Dimension::feature_all_count);


}

void export_metric_crossfilter() {
    register_wrapper_crossfilter<std::string, Record>();
    register_wrapper_crossfilter<int, Record>();
}
/*
    void filter() { filter_all(); }
    void filter(const value_type_t& left, const value_type_t& right) { filter_range(left, right); }
    void filter(const value_type_t& value) { filter_exact(value); }
    void filter(std::function<bool(const value_type_t&)> filterFunction) { filter_with_predicate(filterFunction); }

    template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
    auto feature(AddFunc add_func_, RemoveFunc remove_func_, InitialFunc initial_func_) noexcept -> cross::feature<value_type_t, decltype(initial_func_()), this_type_t, false>;

    template <typename AddFunc, typename RemoveFunc, typename InitialFunc, typename KeyFunc>
    auto feature(AddFunc add_func_, RemoveFunc remove_func_, InitialFunc initial_func_, KeyFunc key) noexcept -> cross::feature<decltype(key(std::declval<value_type_t>())), decltype(initial_func_()), this_type_t, false>;

    template <typename G>
    auto feature(G key_) noexcept -> cross::feature<decltype(key_(std::declval<value_type_t>())), std::size_t, this_type_t, false>;

    template <typename K>
    auto feature_count(K key) noexcept -> cross::feature<decltype(key(std::declval<value_type_t>())), std::size_t, this_type_t, false>;

    template <typename ValueFunc>
    auto feature_sum(ValueFunc value) noexcept -> cross::feature<value_type_t, decltype(value(record_type_t())), this_type_t, false>;

    template <typename ValueFunc, typename KeyFunc>
    auto feature_sum(ValueFunc value, KeyFunc key) noexcept -> cross::feature<decltype(key(std::declval<value_type_t>())), decltype(value(std::declval<record_type_t>())), this_type_t, false>;

    template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
    auto feature_all(AddFunc add_func_, RemoveFunc remove_func_, InitialFunc initial_func_) noexcept -> cross::feature<std::size_t, decltype(initial_func_()), this_type_t, true>;

    template <typename G>
    auto feature_all_sum(G value) noexcept -> cross::feature<std::size_t, decltype(value(record_type_t())), this_type_t, true>;
*/