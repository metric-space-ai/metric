/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#pragma once
#include "metric_types.hpp"
#include "metric_py.hpp"
#include <boost/python/numpy.hpp>

namespace metric_Entropy {

//TODO: fix vector<> in matric::estimate function
template <typename Container, typename Metric = metric::Euclidian<double>, typename L = double>
double entropy_wrap(const Container& data, std::size_t k = 3, L logbase = 2, Metric metric = Metric()) {
    using ContainerIn = WrapStlMatrix<double>;
    std::vector<ContainerIn> data2;
    for (unsigned i = 0; i < data.size(); i++)
        data2.push_back(data[i]);
    return metric::entropy<std::vector<ContainerIn>, Metric, L>(data2, k, logbase, metric);
}

//BOOST_PYTHON_FUNCTION_OVERLOADS(entropy_overloads, entropy, 1, 4);
double entropy(const WrapStlMatrix<double>& obj, std::size_t k = 3, double logbase = 2.0, const char* metricName = "eucludean") {
    double ret = std::numeric_limits<double>::max();
    bool got = false;
        size_t i = 0;
        boost::mpl::for_each<MetricTypes, MPLHelpType<boost::mpl::_1> >([&](auto type_wrapper) {
        using Type1 = typename decltype(type_wrapper)::agrument_type;
            assert(MetricTypeNames.size() > i);
            auto name = MetricTypeNames[i++];
            if (!got && name == metricName) {
                got = true;
                ret = entropy_wrap<WrapStlMatrix<double>,Type1>(obj, k, logbase, Type1());
            }
        });
    if (!got) throw std::runtime_error("No such metric");
    return ret;
}

};
