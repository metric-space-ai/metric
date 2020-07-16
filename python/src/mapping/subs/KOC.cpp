#include "metric_types.hpp"
#include "distance/custom.hpp"

#include "modules/mapping/KOC.hpp"

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <typeindex>
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
) -> metric::KOC<recType, Graph, Metric, Distribution>
{
    return metric::KOC<recType, Graph, Metric, Distribution>(
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
    using Class = metric::KOC<Record, Graph, Metric, Distribution>;
    using value_type = typename Class::T;

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
    const std::string className = "KOC_" + metric::getTypeName<Graph>() + "_" + metric::getTypeName<Metric>();
    auto koc = py::class_<Class>(m, className.c_str());
    std::vector<bool> (Class::*check_if_anomaly1)(const std::vector<Record>&) = &Class::check_if_anomaly;
    bool (Class::*check_if_anomaly2)(const Record&) = &Class::check_if_anomaly;
    koc.def("train", &Class::train,
        py::arg("samples"),
        py::arg("num_clusters"),
        py::arg("min_cluster_size") = 1
    );
    koc.def("top_outliers", &Class::top_outliers,
        py::arg("samples"),
        py::arg("count") = 10
    );
    koc.def("assign_to_clusters", &Class::assign_to_clusters, (py::arg("samples")));
    koc.def("check_if_anomaly", check_if_anomaly1, (py::arg("samples")));
    koc.def("check_if_anomaly", check_if_anomaly2, (py::arg("sample")));
}

// TODO: add distribution
// TODO: add python graphs and distribution

void export_metric_KOC(py::module& m) {
    using Value = double;
    using Container = std::vector<Value>;

    using MetricTypes = boost::mpl::vector<
        metric::Euclidean<Value>
        ,metric::Manhatten<Value>
        ,metric::Chebyshev<Value>
        ,metric::P_norm<Value>
        ,std::function<Value(const Container&, const Container&)>
    >;

    using GraphTypes = boost::mpl::vector<
        metric::Grid4
        ,metric::Grid6
        ,metric::Grid8
    >;

    boost::mpl::for_each<MetricTypes, boost::mpl::make_identity<boost::mpl::_1>>([&](auto metr) {
        using MetricType = typename decltype(metr)::type;
        boost::mpl::for_each<GraphTypes, boost::mpl::make_identity<boost::mpl::_1>>([&](auto graph) {
            using GraphType = typename decltype(graph)::type;
            wrap_metric_KOC<Container, GraphType, MetricType>(m);
        });
    });
}

PYBIND11_MODULE(koc, m) {
    export_metric_KOC(m);
}