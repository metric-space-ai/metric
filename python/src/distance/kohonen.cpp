#include "modules/distance/k-structured/kohonen_distance.hpp"

#include <boost/python.hpp>
#include <vector>

namespace py = boost::python;

/*
template <typename D,
            typename Sample,
            typename Graph = metric::Grid4,
            typename Metric = metric::Euclidian<D>,
	        typename Distribution = std::uniform_real_distribution<typename Sample::value_type>>
    // D is a distance return type


	kohonen_distance(metric::SOM<Sample, Graph, Metric, Distribution> som_model);   <---------- ???
*/

template<typename DistanceType, typename Sample, typename Graph, typename Metric>
void register_wrapper_kohonen() {
    using Class = metric::kohonen_distance<DistanceType, Sample>;
    auto metric = py::class_<Class>("Kohonen", py::no_init);
    metric.def(py::init<const std::vector<Sample>&, Graph, Metric, double, double, size_t>(
        (
            py::arg("samples"),
            py::arg("graph"),
            py::arg("metric"),
            py::arg("start_learn_rate") = 0.8,
            py::arg("finish_learn_rate") = 0.0,
            py::arg("iterations") = 20
        )
    ));
    metric.def(py::init<const std::vector<Sample>&, size_t, size_t>(
        (
            py::arg("samples"),
            py::arg("nodes_width"),
            py::arg("nodes_height")
        )
    ));
    metric.def("__call__", &Class::operator(),
        (
            py::arg("sample1"),
            py::arg("sample2")
        ),
        "Compute the EMD for two records in the Kohonen space."
    );
    metric.def("print_shortest_path", &Class::print_shortest_path,
        (
            py::arg("from_node"),
            py::arg("to_node")
        ),
        "Recursive function that reconstructs the shortest backwards node by node."
    );
}

void export_metric_kohonen() {
    using DistanceType = double;
    using SampleType = std::vector<DistanceType>;
    register_wrapper_kohonen<DistanceType, SampleType, metric::Grid4, metric::Euclidian<DistanceType>>();
}
