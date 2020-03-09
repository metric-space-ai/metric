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
    mgc.def("xcorr", +[](Metric& self,
        const WrapStlMatrix<double>& a,
        const WrapStlMatrix<double>& b,
        int n) {
            return self.xcorr(a, b, n);
        }
    );

//    mgc.def("compute_distance_matrix", +[](Metric& self,  FIXME: Metric parameter undefined
//        const WrapStlMatrix<double>& c) {
//            return self.computeDistanceMatrix(c);
//        }
//    );
    mgc.def("mean", &Metric::mean);
    mgc.def("variance", &Metric::variance);
    mgc.def("icdf", &Metric::icdf);
    mgc.def("erfcinv", &Metric::erfcinv);
    mgc.def("erfinv_imp", &Metric::erfinv_imp);
    mgc.def("polyeval", &Metric::polyeval);
    mgc.def("peak2ems", &Metric::peak2ems);
    mgc.def("linspace", &Metric::linspace);
}

template <class T>
void wrap_metric_MGC_direct() {
    using Metric = metric::MGC_direct;
    auto mgc = bp::class_<Metric>("MGC_direct", bp::no_init);
    mgc.def("__call__", &Metric::operator()<T>);   // FIXME: unsupported argument types
    mgc.def("xcorr", &Metric::xcorr<T>);           // FIXME: unsupported argument types
    mgc.def("center_distance_matrix", &Metric::center_distance_matrix<T>);
    mgc.def("rank_distance_matrix", &Metric::rank_distance_matrix<T>);
    mgc.def("rank_distance_matrix", &Metric::rank_distance_matrix<T>);
    mgc.def("center_ranked_distance_matrix", &Metric::center_ranked_distance_matrix<T>);
    mgc.def("local_covariance", &Metric::local_covariance<T>);
    mgc.def("normalize_generalized_correlation", &Metric::normalize_generalized_correlation<T>);
    mgc.def("rational_approximation", &Metric::rational_approximation<T>);
    mgc.def("normal_CDF_inverse", &Metric::normal_CDF_inverse<T>);
    mgc.def("icdf_normal", &Metric::icdf_normal<T>);
    mgc.def("significant_local_correlation", &Metric::significant_local_correlation<T>);
    mgc.def("frobeniusNorm", &Metric::frobeniusNorm<T>);
    mgc.def("max_in_matrix_regarding_second_boolean_matrix", &Metric::max_in_matrix_regarding_second_boolean_matrix<T>);
    mgc.def("optimal_local_generalized_correlation", &Metric::optimal_local_generalized_correlation<T>);
}

void export_metric_MGC()
{
    // TODO: add more metrics
    wrap_metric_MGC<std::vector<double>, metric::Euclidian<double>, metric::Euclidian<double>>();
    wrap_metric_MGC_direct<double>();
}
