#include "metric_types.hpp"
#include "modules/distance/k-random/VOI.hpp"    // FIXME: and this
#include "modules/mapping/KOC.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <typeindex>
#include <tuple>
#include <random>
#include <optional>

namespace py = pybind11;


template <typename recType, typename Graph, typename Metric, typename Distribution>
auto create_KOC(
    Graph graph,
    Metric metric,
    Distribution distribution,
    double anomaly_sigma = 1.0,
    double start_learn_rate = 0.8,
    double finish_learn_rate = 0.0,
    size_t iterations = 20,
    std::optional<double> nbh_start_size = std::nullopt,
    double nbh_range_decay = 2.0,
    std::optional<long long> random_seed = std::nullopt
) -> metric::KOC_details::KOC<recType, Graph, Metric, Distribution>
{
    return metric::KOC_details::KOC<recType, Graph, Metric, Distribution>(
        graph,
        metric,
        anomaly_sigma,
        start_learn_rate,
        finish_learn_rate,
        iterations,
        distribution,
        nbh_start_size.value_or(std::sqrt(double(graph.getNodesNumber()))),
        nbh_range_decay,
        random_seed.value_or(std::chrono::system_clock::now().time_since_epoch().count())
    );
}


template <typename Record, class Graph, class Metric, class Distribution = std::normal_distribution<double>>
void wrap_metric_KOC(py::module& m) {
    using Factory = metric::KOC_factory<Record, Graph, Metric, Distribution>;
    using KOC = typename Factory::KOC;
    using value_type = typename Factory::T;

    // KOC factory
    m.def("KOC", &create_KOC<Record, Graph, Metric, Distribution>,
        py::arg("graph"),
        py::arg("metric"),
        py::arg("distribution"),
        py::arg("anomaly_sigma") = 1.0,
        py::arg("start_learn_rate") = 0.8,
        py::arg("finish_learn_rate") = 0.0,
        py::arg("iterations") = 20,
        py::arg("nbh_start_size") = (double *) nullptr,
        py::arg("nbh_range_decay") = 2.0,
        py::arg("random_seed") = (long long *) nullptr
    );

    // KOC implementation
    const std::string className = "KOC_" + getGraphName<Graph>() + "_" + getMetricName<Metric>();
    auto koc = py::class_<KOC>(m, className.c_str());
    std::vector<bool> (KOC::*check_if_anomaly1)(const std::vector<Record>&) = &KOC::check_if_anomaly;
    bool (KOC::*check_if_anomaly2)(const Record&) = &KOC::check_if_anomaly;
    koc.def("train", &KOC::train,
        py::arg("samples"),
        py::arg("num_clusters"),
        py::arg("min_cluster_size") = 1
    );
    koc.def("top_outliers", +[](KOC& self, const std::vector<Record>& samples, int count){
            auto result = self.top_outliers(samples, count);
            return py::make_tuple(std::get<0>(result), std::get<1>(result));
        },
        py::arg("samples"),
        py::arg("count") = 10
    );
    koc.def("assign_to_clusters", &KOC::assign_to_clusters, (py::arg("samples")));
    koc.def("check_if_anomaly", check_if_anomaly1, (py::arg("samples")));
    koc.def("check_if_anomaly", check_if_anomaly2, (py::arg("sample")));
}

// TODO: make loop over metrics and graphs
// TODO: add distribution
// TODO: add python graphs, metrics and distribution
void export_metric_KOC(py::module& m) {
    using Value = double;
    using Container = std::vector<double>;
    wrap_metric_KOC<Container, metric::Grid4, metric::Euclidean<Value>>(m);
    wrap_metric_KOC<Container, metric::Grid4, metric::Manhatten<Value>>(m);
    wrap_metric_KOC<Container, metric::Grid4, metric::Chebyshev<Value>>(m);
    wrap_metric_KOC<Container, metric::Grid4, metric::P_norm<Value>>(m);

    wrap_metric_KOC<Container, metric::Grid6, metric::Euclidean<Value>>(m);
    wrap_metric_KOC<Container, metric::Grid6, metric::Manhatten<Value>>(m);
    wrap_metric_KOC<Container, metric::Grid6, metric::Chebyshev<Value>>(m);
    wrap_metric_KOC<Container, metric::Grid6, metric::P_norm<Value>>(m);

    wrap_metric_KOC<Container, metric::Grid8, metric::Euclidean<Value>>(m);
    wrap_metric_KOC<Container, metric::Grid8, metric::Manhatten<Value>>(m);
    wrap_metric_KOC<Container, metric::Grid8, metric::Chebyshev<Value>>(m);
    wrap_metric_KOC<Container, metric::Grid8, metric::P_norm<Value>>(m);
}

PYBIND11_MODULE(koc, m) {
    export_metric_KOC(m);
}