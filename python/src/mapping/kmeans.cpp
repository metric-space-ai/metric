#include <chrono>  // FIXME
#include "modules/mapping/kmeans.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <tuple>

namespace py = pybind11;

template<typename T>
py::tuple kmeans(const std::vector<std::vector<T>>& data,
                 int k,
                 int maxiter = 200,
                 std::string distance_measure = "euclidian") {
    auto result = metric::kmeans(data, k, maxiter, distance_measure);
    return py::make_tuple(std::get<0>(result), std::get<1>(result), std::get<2>(result));
}

template <typename T>
void register_wrapper_kmeans(py::module& m) {
    m.def("kmeans", &kmeans<T>,
        py::arg("data"),
        py::arg("k") = 0.5,
        py::arg("maxiter") = 200,
        py::arg("distance_measure") = "euclidian"   // TODO: fix typo
    );
}

void export_metric_kmeans(py::module& m) {
    register_wrapper_kmeans<double>(m);
}
