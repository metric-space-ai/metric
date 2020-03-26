#include "modules/mapping/kmedoids.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <tuple>

namespace py = pybind11;

template <typename recType, typename Metric, typename T = typename Metric::distance_type>
py::tuple kmedoids(const metric::Matrix<recType, Metric>& DM, int k) {
    auto result = metric::kmedoids(DM, k);
    return py::make_tuple(std::get<0>(result), std::get<1>(result), std::get<2>(result));
}

template <typename recType, typename Metric, typename T>
void register_wrapper_kmedoids(py::module& m) {
    m.def("kmedoids", &kmedoids<recType, Metric, T>,
        py::arg("dm"),
        py::arg("k")
    );
}

void export_metric_kmedoids(py::module& m) {
    register_wrapper_kmedoids<std::vector<double>, metric::Euclidian<double>, double>(m);
}
