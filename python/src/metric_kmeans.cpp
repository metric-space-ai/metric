#include "modules/mapping/kmeans.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

template <typename T>
void register_wrapper_kmeans() {
    bp::def("kmeans", &metric::kmeans<T>,
        (
            bp::arg("data"),
            bp::arg("k") = 0.5,
            bp::arg("maxiter") = 200,
            bp::arg("distance_measure") = "euclidian"   // TODO: fix typo
        )
    );
}

void export_metric_kmeans() {
    register_wrapper_kmeans<double>();
}
