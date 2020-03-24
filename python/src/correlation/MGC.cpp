/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/correlation/mgc.hpp"
#include "../metric_types.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <string>
#include <vector>

class NotUsed {};

namespace py = pybind11;

template <class ValueType, class Metric1, class Metric2>
void wrap_metric_MGC(py::module& m) {
    using Metric = metric::MGC<NotUsed, Metric1, NotUsed, Metric2>;
    using Container = std::vector<std::vector<ValueType>>;
    std::string name = "MGC_" + getMetricName<Metric1>() + "_" + getMetricName<Metric2>();
    auto mgc = py::class_<Metric>(m, name.c_str(), "Multiscale Graph Correlation");
    mgc.def(py::init<>(),
        "Construct Multiscale Graph Correlation"
    );
    mgc.def(py::init<const Metric1&, const Metric2&>(),
        "Construct Multiscale Graph Correlation",
        py::arg("m1"),
        py::arg("m2")
    );
    // both methods means the same, but bind function much easier
    mgc.def("__call__", &Metric::template operator()<Container, Container>,
        "Return correlation betweeen a and b",
        py::arg("a"),
        py::arg("b")
    );
    mgc.def("xcorr", &Metric::template xcorr<Container, Container>,
        "Return vector of mgc values calculated for different data shifts",
        py::arg("a"),
        py::arg("b"),
        py::arg("n")
    );
    mgc.def("estimate", &Metric::template estimate<Container, Container>,
        "Return estimate of the correlation between a and b",
        py::arg("a"),
        py::arg("b"),
        py::arg("b_sample_size") = 250,
        py::arg("threshold") = 0.05,
        py::arg("max_iterations") = 1000
    );

//    mgc.def("compute_distance_matrix", &Metric::template computeDistanceMatrix<Container, Metric+[](Metric& self,  FIXME: Metric parameter undefined
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
void wrap_metric_MGC_direct(py::module& m) {
    using Metric = metric::MGC_direct;
    auto mgc = py::class_<Metric>(m, "MGC_direct");
    mgc.def("__call__", &Metric::xcorr<T>);   // FIXME: unsupported argument types
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

void export_metric_MGC(py::module& m)
{
    using T = double;
    // TODO: loop
    wrap_metric_MGC<T, metric::Euclidian<T>, metric::Euclidian<T>>(m);
    wrap_metric_MGC<T, metric::Euclidian<T>, metric::Manhatten<T>>(m);
//    wrap_metric_MGC<T, metric::Euclidian<T>, metric::Chebyshev<T>>(m);
    wrap_metric_MGC<T, metric::Euclidian<T>, metric::P_norm<T>>(m);

    wrap_metric_MGC<T, metric::Manhatten<T>, metric::Euclidian<T>>(m);
    wrap_metric_MGC<T, metric::Manhatten<T>, metric::Manhatten<T>>(m);
//    wrap_metric_MGC<T, metric::Manhatten<T>, metric::Chebyshev<T>>(m);
    wrap_metric_MGC<T, metric::Manhatten<T>, metric::P_norm<T>>(m);

//    wrap_metric_MGC<T, metric::Chebyshev<T>, metric::Euclidian<T>>();
//    wrap_metric_MGC<T, metric::Chebyshev<T>, metric::Manhatten<T>>();
//    wrap_metric_MGC<T, metric::Chebyshev<T>, metric::Chebyshev<T>>();
//    wrap_metric_MGC<T, metric::Chebyshev<T>, metric::P_norm<T>>();

    wrap_metric_MGC<T, metric::P_norm<T>, metric::Euclidian<T>>(m);
    wrap_metric_MGC<T, metric::P_norm<T>, metric::Manhatten<T>>(m);
//    wrap_metric_MGC<T, metric::P_norm<T>, metric::Chebyshev<T>>();
    wrap_metric_MGC<T, metric::P_norm<T>, metric::P_norm<T>>(m);

    wrap_metric_MGC_direct<T>(m);
}
