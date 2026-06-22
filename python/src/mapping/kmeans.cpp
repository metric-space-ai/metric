#include "metric/mapping/kmeans.hpp"
#include <chrono> // FIXME

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>

namespace py = pybind11;

template <typename T> void register_wrapper_kmeans(py::module &m)
{
	m.def("kmeans", &mtrc::kmeans<T>, py::arg("data"), py::arg("k") = 0.5, py::arg("maxiter") = 200,
		  py::arg("metric") = "Euclidean", // TODO: fix typo
		  py::arg("random_seed") = -1);
}

void export_metric_kmeans(py::module &m) { register_wrapper_kmeans<double>(m); }
