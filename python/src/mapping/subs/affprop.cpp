#include "modules/mapping/affprop.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <tuple>

namespace py = pybind11;

template<typename recType, typename Metric, typename T>
py::tuple affprop(const metric::Matrix<recType, Metric>& DM,
                  T preference = 0.5,
                  int maxiter = 200,
                  T tol = 1.0e-6,
                  T damp = 0.5) {
    auto result = metric::affprop(DM, preference, maxiter, tol, damp);
    return py::make_tuple(std::get<0>(result), std::get<1>(result), std::get<2>(result));
}

template <typename recType, typename Metric, typename T>
void register_wrapper_affprop(py::module& m) {
    m.def("affprop", &affprop<recType, Metric, T>,
        py::arg("dm"),
        py::arg("preference") = 0.5,
        py::arg("maxiter") = 200,
        py::arg("tol") = 1.0e-6,
        py::arg("damp") = 0.5
    );
}

void export_metric_affprop(py::module& m) {
    register_wrapper_affprop<std::vector<double>, metric::Euclidian<double>, float>(m);
}

PYBIND11_MODULE(_affprop, m) {
    export_metric_affprop(m);
}