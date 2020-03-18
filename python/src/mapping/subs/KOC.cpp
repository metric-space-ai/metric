#include "../../metric_types.hpp"
#include "modules/mapping/kmeans.hpp"   // FIXME: someone forgot this in KOC
#include "modules/distance/k-random/VOI.hpp"    // FIXME: and this
#include "modules/mapping/KOC.hpp"


#include <boost/python.hpp>
#include <typeindex>
#include <tuple>

namespace py = boost::python;

/*
  koc = KOC(rec_type="double", graph="grid6", metric="euclidean", distribution="uniform")

  constructor will be a factory
  it will pickup pre-built C++ class
  all template methods of this class will be generated for popular cases only
*/
template <typename Record, class Graph, class Metric>
void wrap_metric_KOC() {
    using Factory = metric::KOC_factory<Record, Graph, Metric>;
    using KOC = typename Factory::KOC;
    using value_type = typename Factory::T;

    std::string className = "KOC_factory_" + getGraphName<Graph>() + "_" + getMetricName<Metric>();

    auto factory = py::class_<Factory>(className.c_str());

    factory.def(
        py::init<size_t, double, double, double, size_t, value_type, value_type>(
            (
                py::arg("nodes_number"),
                py::arg("anomaly_sigma"),
                py::arg("start_learn_rate")= 0.8,
                py::arg("finish_learn_rate") = 0.0,
                py::arg("iterations") = 20,
                py::arg("distribution_min") = -1,
                py::arg("distribution_max") = 1
            )
        )
    );
    factory.def(
        py::init<size_t, size_t, double, double, double, size_t, value_type, value_type>(
            (
                py::arg("nodes_width") = 5,
                py::arg("nodes_height") = 4,
                py::arg("anomaly_sigma") = 1.0,
                py::arg("start_learn_rate") = 0.8,
                py::arg("finish_learn_rate") = 0.0,
                py::arg("iterations") = 20,
                py::arg("distribution_min") = -1,
                py::arg("distribution_max") = 1
            )
        )
    );
    factory.def(
        py::init<size_t, double, double, double, size_t, value_type, value_type, double, double, long long>(
            (
                py::arg("nodes_number"),
                py::arg("anomaly_sigma"),
                py::arg("start_learn_rate"),
                py::arg("finish_learn_rate"),
                py::arg("iterations"),
                py::arg("distribution_min"),
                py::arg("distribution_max"),
                py::arg("neighborhood_start_size"),
                py::arg("neigbour_range_decay"),
                py::arg("random_seed")
            )
        )
    );
    factory.def(
        py::init<size_t, size_t, double, double, double, size_t, value_type, value_type, double, double, long long>(
            (
                py::arg("nodes_width"),
                py::arg("nodes_height"),
                py::arg("anomaly_sigma"),
                py::arg("start_learn_rate"),
                py::arg("finish_learn_rate"),
                py::arg("iterations"),
                py::arg("distribution_min"),
                py::arg("distribution_max"),
                py::arg("neighborhood_start_size"),
                py::arg("neigbour_range_decay"),
                py::arg("random_seed")
            )
        )
    );

    factory.def("__call__", &Factory::operator(), "construct KOC",
        (
            py::arg("samples"),
            py::arg("num_clusters"),
            py::arg("min_cluster_size") = 1
        )
    );

    // KOC
    className = "KOC_" + getGraphName<Graph>() + "_" + getMetricName<Metric>();
    auto koc = py::class_<KOC>(className.c_str(), py::no_init);
    std::vector<bool> (KOC::*check_if_anomaly1)(const std::vector<Record>&) = &KOC::check_if_anomaly;
    bool (KOC::*check_if_anomaly2)(const Record&) = &KOC::check_if_anomaly;
    koc.def("train", &KOC::train,
        (
            py::arg("samples"),
            py::arg("num_clusters"),
            py::arg("min_cluster_size") = 1
        )
    );
    koc.def("top_outliers", +[](KOC& self, const std::vector<Record>& samples, int count){
        auto result = self.top_outliers(samples, count);
        return py::make_tuple(std::get<0>(result), std::get<1>(result));
    },
        (
            py::arg("samples"),
            py::arg("count") = 10
        )
    );
    koc.def("assign_to_clusters", &KOC::assign_to_clusters, (py::arg("samples")));
    koc.def("check_if_anomaly", check_if_anomaly1, (py::arg("samples")));
    koc.def("check_if_anomaly", check_if_anomaly2, (py::arg("sample")));
}

// TODO: make loop over metrics and graphs
// TODO: add distribution
void export_metric_KOC() {
    wrap_metric_KOC<std::vector<double>, metric::Grid6, metric::Euclidian<double>>();
    wrap_metric_KOC<std::vector<double>, metric::Grid6, metric::Manhatten<double>>();
    wrap_metric_KOC<std::vector<double>, metric::Grid6, metric::Chebyshev<double>>();
    wrap_metric_KOC<std::vector<double>, metric::Grid6, metric::P_norm<double>>();
}

BOOST_PYTHON_MODULE(_koc) {
    export_metric_KOC();
}