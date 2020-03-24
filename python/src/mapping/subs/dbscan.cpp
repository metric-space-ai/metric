#include "modules/mapping/dbscan.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <tuple>
#include <vector>

namespace py = pybind11;

template<typename recType, typename Metric, typename T>
py::tuple dbscan(const metric::Matrix<recType, Metric>& dm, T eps, std::size_t minpts) {
    auto result = metric::dbscan(dm, eps, minpts);
    return py::make_tuple(std::get<0>(result), std::get<1>(result), std::get<2>(result));
}

template<typename recType, typename Metric, typename T>
void register_wrapper_dbscan(py::module& m) {
    m.def("dbscan", &dbscan<recType, Metric, T>);
}

void export_metric_dbscan(py::module& m) {
    register_wrapper_dbscan<std::vector<double>, metric::Euclidian<double>, double>(m);
}

PYBIND11_MODULE(_dbscan, m) {
    export_metric_dbscan(m);
}