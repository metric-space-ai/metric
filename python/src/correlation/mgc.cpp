/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/correlation/mgc.hpp"
#include "metric_types.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <string>
#include <vector>

class NotUsed {};

namespace py = pybind11;

template<typename Metric1, typename Metric2>
metric::MGC<NotUsed, Metric1, NotUsed, Metric2> createMGC(Metric1 metric1, Metric2 metric2) {
    return metric::MGC<NotUsed, Metric1, NotUsed, Metric2>(metric1, metric2);
}

template <class ValueType, class Metric1, class Metric2>
void wrap_metric_MGC(py::module& m, const std::string& prefix) {
    using Container = std::vector<std::vector<ValueType>>;
    using Class = metric::MGC<NotUsed, Metric1, NotUsed, Metric2>;
    m.def("create_mgc", &createMGC<Metric1, Metric2>,
        "internal method to create MGC instances",
        py::arg("metric1"),
        py::arg("metric2")
    );

    const std::string name = std::string("MGC__") + prefix;
    auto mgc = py::class_<Class>(m, name.c_str());
    auto corr_ptr = &Class::template operator()<Container, Container>;
    mgc.def("__call__", corr_ptr,
        "Multiscale Graph Correlation between a and b",
        py::arg("a"),
        py::arg("b")
    );

    auto xcorr_ptr = &Class::template xcorr<Container, Container>;
    mgc.def("xcorr", xcorr_ptr,
        "Return vector of MGC values calculated for different data shifts",
        py::arg("a"),
        py::arg("b"),
        py::arg("n")
    );
    auto estimate_ptr = &Class::template estimate<Container, Container>;
    mgc.def("estimate", estimate_ptr,
        "Estimate of the correlation between a and b",
        py::arg("a"),
        py::arg("b"),
        py::arg("b_sample_size") = 250,
        py::arg("threshold") = 0.05,
        py::arg("max_iterations") = 1000
    );
// should be private
//    m.def("compute_distance_matrix", &computeDistanceMatrix<Container, Metric1>);
//    m.def("compute_distance_matrix", &computeDistanceMatrix<Container, Metric2>);
}

template <class T>
void wrap_metric_MGC_direct(py::module& m) {
    using Metric = metric::MGC_direct;
    auto mgc = py::class_<Metric>(m, "MGC_direct");
    mgc.def("__call__", &Metric::xcorr<T>);   // FIXME: unsupported argument types
    mgc.def("xcorr", &Metric::xcorr<T>);      // FIXME: unsupported argument types
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
    wrap_metric_MGC<T, metric::Euclidean<T>, metric::Euclidean<T>>(m, "Euclidean_Euclidean");
    wrap_metric_MGC<T, metric::Euclidean<T>, metric::Manhatten<T>>(m, "Euclidean_Manhatten");
//    wrap_metric_MGC<T, metric::Euclidean<T>, metric::Chebyshev<T>>(m);
    wrap_metric_MGC<T, metric::Euclidean<T>, metric::P_norm<T>>(m, "Euclidean_Pnorm");

    wrap_metric_MGC<T, metric::Manhatten<T>, metric::Euclidean<T>>(m, "Manhatten_Euclidean");
    wrap_metric_MGC<T, metric::Manhatten<T>, metric::Manhatten<T>>(m, "Manhatten_Manhatten");
//    wrap_metric_MGC<T, metric::Manhatten<T>, metric::Chebyshev<T>>(m);
    wrap_metric_MGC<T, metric::Manhatten<T>, metric::P_norm<T>>(m, "Manhatten_Pnorm");

//    wrap_metric_MGC<T, metric::Chebyshev<T>, metric::Euclidean<T>>();
//    wrap_metric_MGC<T, metric::Chebyshev<T>, metric::Manhatten<T>>();
//    wrap_metric_MGC<T, metric::Chebyshev<T>, metric::Chebyshev<T>>();
//    wrap_metric_MGC<T, metric::Chebyshev<T>, metric::P_norm<T>>();

    wrap_metric_MGC<T, metric::P_norm<T>, metric::Euclidean<T>>(m, "Pnorm_Euclidean");
    wrap_metric_MGC<T, metric::P_norm<T>, metric::Manhatten<T>>(m, "Pnorm_Manhatten");
//    wrap_metric_MGC<T, metric::P_norm<T>, metric::Chebyshev<T>>();
    wrap_metric_MGC<T, metric::P_norm<T>, metric::P_norm<T>>(m, "Pnorm_Pnorm");

    wrap_metric_MGC_direct<T>(m);
}


PYBIND11_MODULE(mgc, m) {
    export_metric_MGC(m);
}
