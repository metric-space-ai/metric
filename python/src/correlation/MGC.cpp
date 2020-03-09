/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/correlation/mgc.hpp"
#include "../stl_wrappers.hpp"

#include <boost/python.hpp>

class NotUsed {};

namespace bp = boost::python;

template <class RecType, class Metric1, class Metric2>
void wrap_metric_MGC() {
    using Metric = metric::MGC<NotUsed, Metric1, NotUsed, Metric2>;
    using Container = std::vector<RecType>;
    auto mgc = bp::class_<Metric>("MGC", bp::no_init);
    mgc.def("__call__", +[](Metric& self,
        const WrapStlMatrix<double>& a,
        const WrapStlMatrix<double>& b) {
            return self(a, b);
        }
    );
    mgc.def("estimate", +[](Metric& self,
        const WrapStlMatrix<double>& a,
        const WrapStlMatrix<double>& b,
        size_t b_sample_size = 250,
        double threshold = 0.05,
        size_t max_iterations = 1000) {
            return self.estimate(a, b, b_sample_size, threshold, max_iterations);
        }
    );
    std::vector<double> (Metric::*xcorr)(const Container&, const Container&) = &Metric::xcorr;
    mgc.def("xcorr", xcorr);

    mgc.def("compute_distance_matrix", +[](Metric& self,
        const WrapStlMatrix<double>& c) {
            return self.computeDistanceMatrix(c);
        }
    );
    mgc.def("mean", &Metric::mean);
    mgc.def("variance", &Metric::variance);
    mgc.def("icdf", &Metric::icdf);
    mgc.def("erfcinv", &Metric::erfcinv);
    mgc.def("erfinv_imp", &Metric::erfinv_imp);
    mgc.def("polyeval", &Metric::polyeval);
    mgc.def("peak2ems", &Metric::peak2ems);
    mgc.def("linspace", &Metric::linspace);
}

void export_metric_MGC()
{
    wrap_metric_MGC<std::vector<double>, metric::Euclidian<double>, metric::Euclidian<double>>();
}
