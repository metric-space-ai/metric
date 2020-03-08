#include "../../metric_types.hpp"
#include "modules/mapping/kmeans.hpp"   // FIXME: someone forgot this in KOC
#include "modules/distance/k-random/VOI.hpp"    // FIXME: and this
#include "modules/mapping/KOC.hpp"

#include <boost/python.hpp>
#include <typeindex>

namespace bp = boost::python;

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

    auto factory = bp::class_<Factory>(className.c_str());

    factory.def(
        bp::init<size_t, double, double, size_t, value_type, value_type>(
            (
                bp::arg("nodesNumber"),
                bp::arg("start_learn_rate")= 0.8,
                bp::arg("finish_learn_rate") = 0.0,
                bp::arg("iterations") = 20,
                bp::arg("distribution_min") = -1,
                bp::arg("distribution_max") = 1
            )
        )
    );
    factory.def(
        bp::init<size_t, size_t, double, double, size_t, value_type, value_type>(
            (
                bp::arg("nodesWidth") = 5,
                bp::arg("nodesHeight") = 4,
                bp::arg("start_learn_rate") = 0.8,
                bp::arg("finish_learn_rate") = 0.0,
                bp::arg("iterations") = 20,
                bp::arg("distribution_min") = -1,
                bp::arg("distribution_max") = 1
            )
        )
    );
    factory.def(
        bp::init<size_t, double, double, size_t, value_type, value_type, double, double, long long>(
            (
                bp::arg("nodesNumber"),
                bp::arg("start_learn_rate"),
                bp::arg("finish_learn_rate"),
                bp::arg("iterations"),
                bp::arg("distribution_min"),
                bp::arg("distribution_max"),
                bp::arg("neighborhood_start_size"),
                bp::arg("neigbour_range_decay"),
                bp::arg("random_seed")
            )
        )
    );
    factory.def(
        bp::init<size_t, size_t, double, double, size_t, value_type, value_type, double, double, long long>(
            (
                bp::arg("nodesWidth"),
                bp::arg("nodesHeight"),
                bp::arg("start_learn_rate"),
                bp::arg("finish_learn_rate"),
                bp::arg("iterations"),
                bp::arg("distribution_min"),
                bp::arg("distribution_max"),
                bp::arg("neighborhood_start_size"),
                bp::arg("neigbour_range_decay"),
                bp::arg("random_seed")
            )
        )
    );

    factory.def("__call__", &Factory::operator(), "construct KOC");

    std::vector<bool> (KOC::*check_if_anomaly1)(const std::vector<Record>&, double) = &KOC::check_if_anomaly;
    bool (KOC::*check_if_anomaly2)(const Record&, double) = &KOC::check_if_anomaly;

    // KOC
    className = "KOC_" + getGraphName<Graph>() + "_" + getMetricName<Metric>();
    auto koc = bp::class_<KOC>(className.c_str(), bp::no_init);
    std::vector<int> (KOC::*encode)(const std::vector<Record>&, double) = &KOC::encode;
    koc.def("train", &KOC::train);
    koc.def("result", &KOC::result);
    koc.def("encode", encode);
    koc.def("check_if_anomaly", check_if_anomaly1, (bp::arg("samples"), bp::arg("anomaly_threshold") = 0.0));
    koc.def("check_if_anomaly", check_if_anomaly2, (bp::arg("sample"), bp::arg("anomaly_threshold") = 0.0));
}

// TODO: make loop over metrics and graphs
// TODO: add distribution
void export_metric_KOC() {
    wrap_metric_KOC<std::vector<double>, metric::Grid6, metric::Euclidian<double>>();
    wrap_metric_KOC<std::vector<double>, metric::Grid6, metric::Manhatten<double>>();
    wrap_metric_KOC<std::vector<double>, metric::Grid6, metric::Chebyshev<double>>();
    wrap_metric_KOC<std::vector<double>, metric::Grid6, metric::P_norm<double>>();
}

BOOST_PYTHON_MODULE(_KOC) {
    export_metric_KOC();
}