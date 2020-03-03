#include "modules/mapping/affprop.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

template <typename recType, typename Metric, typename T>
void register_wrapper_affprop() {
    bp::def("affprop", &metric::affprop<recType, Metric, T>,
        (
            bp::arg("dm"),
            bp::arg("preference") = 0.5,
            bp::arg("maxiter") = 200,
            bp::arg("tol") = 1.0e-6,
            bp::arg("damp") = 0.5
        )
    );
}

void export_metric_affprop() {
    register_wrapper_affprop<std::vector<double>, metric::Euclidian<double>, float>();
}

BOOST_PYTHON_MODULE(_affprop) {
    export_metric_affprop();
}