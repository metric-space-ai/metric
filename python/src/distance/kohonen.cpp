#include "modules/distance/k-structured/kohonen_distance.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

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
void register_wrapper_kohonen(py::module& m) {
    using Class = metric::kohonen_distance<DistanceType, Sample>;
    auto metric = py::class_<Class>(m, "Kohonen");
    metric.def(py::init<const std::vector<Sample>&, Graph, Metric, double, double, size_t>(),
        py::arg("samples"),
        py::arg("graph"),
        py::arg("metric"),
        py::arg("start_learn_rate") = 0.8,
        py::arg("finish_learn_rate") = 0.0,
        py::arg("iterations") = 20
    );
    metric.def(py::init<const std::vector<Sample>&, size_t, size_t>(),
        py::arg("samples"),
        py::arg("nodes_width"),
        py::arg("nodes_height")
    );
    metric.def("__call__", &Class::operator(),
        "Compute the EMD for two records in the Kohonen space.",
        py::arg("sample1"),
        py::arg("sample2")
    );
    metric.def("print_shortest_path", &Class::print_shortest_path,
        "Recursive function that reconstructs the shortest backwards node by node.",
        py::arg("from_node"),
        py::arg("to_node")
    );
}

void export_metric_kohonen(py::module& m) {
    using DistanceType = double;
    using SampleType = std::vector<DistanceType>;
    register_wrapper_kohonen<DistanceType, SampleType, metric::Grid4, metric::Euclidian<DistanceType>>(m);
}
