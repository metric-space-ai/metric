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
    bp::class_<Metric>("VOI_kl", bp::init<int, Value>())
        .def("__call__", +[](Metric& self, bp::object& A, bp::object& B) {
            auto vecA = WrapStlVector<Value>(A);
            auto vecB = WrapStlVector<Value>(B);
            auto matA = WrapStlVector<WrapStlVector<Value>>(vecA);
            auto matB = WrapStlVector<WrapStlVector<Value>>(vecB);
            return self.operator()(matA, matB);
        }, "Calculate variation of information based on Kozachenko-Leonenko entropy estimator");
}

template<typename Value>
void wrap_metric_VOI_normalized() {
    using Metric = metric::VOI_normalized<Value>;
    bp::class_<Metric>("VOI_normalized", bp::init<int, Value>())
        .def("__call__", +[](Metric& self, bp::object& A, bp::object& B) {
            auto vecA = WrapStlVector<Value>(A);
            auto vecB = WrapStlVector<Value>(B);
            auto matA = WrapStlVector<WrapStlVector<Value>>(vecA);
            auto matB = WrapStlVector<WrapStlVector<Value>>(vecB);
            return self.operator()(matA, matB);
        }, "Calculate Variation of Information");
}

void export_metric_VOI() {
    wrap_metric_VOI_normalized<double>();
    wrap_metric_VOI_kl<double>();
}

template <typename Container, typename Metric>
void wrap_metric_entropy() {
    std::string name = "entropy_" + getMetricName<Metric>();
    auto entropy = &metric::entropy<Container, Metric, typename Container::value_type::value_type>;
    bp::def(name.c_str(), entropy, "Continuous entropy estimator");
}

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