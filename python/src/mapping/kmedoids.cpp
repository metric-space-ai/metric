#include "modules/mapping/kmedoids.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

template <typename recType, typename Metric, typename T>
void register_wrapper_kmedoids() {
    bp::def("kmedoids", &metric::kmedoids<recType, Metric, T>,
        (
            bp::arg("dm"),
            bp::arg("k")
        )
    );
}

void export_metric_kmedoids() {
    register_wrapper_kmedoids<std::vector<double>, metric::Euclidian<double>, double>();
}
