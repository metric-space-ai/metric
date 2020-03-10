/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/distance/k-random/VOI.hpp"
#include "../../metric_types.hpp"
#include "../../stl_wrappers.hpp"
#include <boost/python.hpp>
#include <vector>
#include <string>

namespace bp = boost::python;

template<typename Value>
void wrap_metric_VOI_kl() {
    using Metric = metric::VOI_kl<Value>;
    using Container = WrapStlVector<WrapStlVector<Value>>;
    bp::class_<Metric>("VOI_kl", bp::init<int, Value>((bp::arg("k") = 3, bp::arg("logbase") = 2)))
        .def("__call__", +[](Metric& self, const Container& a, const Container& b) {
            return self.operator()(a, b);
        }, "Calculate variation of information based on Kozachenko-Leonenko entropy estimator");
}

template<typename Value>
void wrap_metric_VOI_normalized() {
    using Metric = metric::VOI_normalized<Value>;
    using Container = WrapStlVector<WrapStlVector<Value>>;
    bp::class_<Metric>("VOI_normalized", bp::init<int, Value>(
            (
                bp::arg("k") = 3,
                bp::arg("logbase") = 2
            )
        ))
        .def("__call__", +[](Metric& self, const Container& a, const Container& b) {
            return self.operator()(a, b);
        }, "Calculate Variation of Information");
}

void export_metric_VOI() {
    wrap_metric_VOI_normalized<double>();
    wrap_metric_VOI_kl<double>();
}

template <typename Container, typename Metric>
void wrap_metric_entropy() {
    using Value = typename Container::value_type::value_type;
    std::string name = "entropy_" + getMetricName<Metric>();
    bp::def(name.c_str(), +[](const WrapStlVector<WrapStlVector<Value>>& data, std::size_t k = 3, Value logbase = 2) {
        return metric::entropy(data, k, logbase);
    }, (bp::arg("data"), bp::arg("k") = 3, bp::arg("logbase") = 2), "Continuous entropy estimator");
}


template <typename Container, typename Metric = metric::Euclidian<typename Container::value_type::value_type>, typename L = double>
double entropy(
    Container data, std::size_t k = 3, L logbase = 2);

void export_metric_entropy() {
    using Value = double;
    using Container = std::vector<std::vector<Value>>;
    wrap_metric_entropy<Container, metric::Euclidian<Value>>();
    wrap_metric_entropy<Container, metric::Manhatten<Value>>();
    wrap_metric_entropy<Container, metric::Chebyshev<Value>>();
    wrap_metric_entropy<Container, metric::P_norm<Value>>();
}

BOOST_PYTHON_MODULE(_VOI) {
    export_metric_VOI();
    export_metric_entropy();
}