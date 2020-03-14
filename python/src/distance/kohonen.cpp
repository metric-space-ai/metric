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

	kohonen_distance(std::vector<Sample>& samples,
	                size_t nodesWidth,
	                size_t nodesHeight);
	kohonen_distance(std::vector<Sample>& samples,
                    Graph graph,
                    Metric metric = Metric(),
                    double start_learn_rate = 0.8,
                    double finish_learn_rate = 0.0,
                    size_t iterations = 20,
                    Distribution distribution = Distribution(-1, 1));

*/

template<typename DistanceType, typename Sample>
void register_wrapper_kohonen() {
    using Metric = metric::kohonen_distance<DistanceType, Sample>;
    auto metric = py::class_<Metric>("Kohonen", py::no_init);
    metric.def(py::init<const std::vector<Sample>&, size_t, size_t>(
        (
            py::arg("samples"),
            py::arg("nodes_width"),
            py::arg("nodes_height")
        )
    ));
    metric.def("__call__", &Metric::operator());
}

void export_metric_kohonen() {
    register_wrapper_kohonen<double, std::vector<double>>();
}
