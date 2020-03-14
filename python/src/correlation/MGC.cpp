/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/correlation/mgc.hpp"
#include "../stl_wrappers.hpp"
#include "../metric_types.hpp"

#include <boost/python.hpp>
#include <string>

class NotUsed {};

namespace py = boost::python;

template <class RecType, class Metric1, class Metric2>
void wrap_metric_MGC() {
    using Metric = metric::MGC<NotUsed, Metric1, NotUsed, Metric2>;
    using Container = std::vector<RecType>;
    std::string name = "MGC_" + getMetricName<Metric1>() + "_" + getMetricName<Metric2>();
    auto mgc = py::class_<Metric>(name.c_str(), "Multiscale Graph Correlation");
    mgc.def("__call__", +[](Metric& self,
        const WrapStlMatrix<double>& a,
        const WrapStlMatrix<double>& b) {
            return self(a, b);
        },
        (
            py::arg("a"),
            py::arg("b")
        ),
        "Return correlation betweeen a and b"
    );
    mgc.def("estimate", +[](Metric& self,
        const WrapStlMatrix<double>& a,
        const WrapStlMatrix<double>& b,
        size_t b_sample_size = 250,
        double threshold = 0.05,
        size_t max_iterations = 1000) {
            return self.estimate(a, b, b_sample_size, threshold, max_iterations);
        },
        (
            py::arg("a"),
            py::arg("b"),
            py::arg("b_sample_size") = 250,
            py::arg("threshold") = 0.05,
            py::arg("max_iterations") = 1000
        ),
        "Return estimate of the correlation between a and b"
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
    auto mgc = py::class_<Metric>("MGC_direct");
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
    using T = double;
    // TODO: loop
    wrap_metric_MGC<std::vector<T>, metric::Euclidian<T>, metric::Euclidian<T>>();
    wrap_metric_MGC<std::vector<T>, metric::Euclidian<T>, metric::Manhatten<T>>();
//    wrap_metric_MGC<std::vector<T>, metric::Euclidian<T>, metric::Chebyshev<T>>();
    wrap_metric_MGC<std::vector<T>, metric::Euclidian<T>, metric::P_norm<T>>();

    wrap_metric_MGC<std::vector<T>, metric::Manhatten<T>, metric::Euclidian<T>>();
    wrap_metric_MGC<std::vector<T>, metric::Manhatten<T>, metric::Manhatten<T>>();
//    wrap_metric_MGC<std::vector<T>, metric::Manhatten<T>, metric::Chebyshev<T>>();
    wrap_metric_MGC<std::vector<T>, metric::Manhatten<T>, metric::P_norm<T>>();

//    wrap_metric_MGC<std::vector<T>, metric::Chebyshev<T>, metric::Euclidian<T>>();
//    wrap_metric_MGC<std::vector<T>, metric::Chebyshev<T>, metric::Manhatten<T>>();
//    wrap_metric_MGC<std::vector<T>, metric::Chebyshev<T>, metric::Chebyshev<T>>();
//    wrap_metric_MGC<std::vector<T>, metric::Chebyshev<T>, metric::P_norm<T>>();

    wrap_metric_MGC<std::vector<T>, metric::P_norm<T>, metric::Euclidian<T>>();
    wrap_metric_MGC<std::vector<T>, metric::P_norm<T>, metric::Manhatten<T>>();
//    wrap_metric_MGC<std::vector<T>, metric::P_norm<T>, metric::Chebyshev<T>>();
    wrap_metric_MGC<std::vector<T>, metric::P_norm<T>, metric::P_norm<T>>();

    wrap_metric_MGC_direct<T>();
}
