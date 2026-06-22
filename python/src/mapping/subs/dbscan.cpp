#include "metric/mapping/dbscan.hpp"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>

namespace py = pybind11;

template <typename RecType, typename Metric, typename T> void register_wrapper_dbscan(py::module &m)
{
	m.def("dbscan", &mtrc::dbscan<RecType, Metric, T>);
}

void export_metric_dbscan(py::module &m)
{
	register_wrapper_dbscan<std::vector<double>, mtrc::Euclidean<double>, double>(m);
}

PYBIND11_MODULE(dbscan, m) { export_metric_dbscan(m); }