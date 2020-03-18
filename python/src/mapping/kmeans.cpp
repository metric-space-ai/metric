#include <chrono>  // FIXME
#include "modules/mapping/kmeans.hpp"

#include <boost/python.hpp>
#include <vector>
#include <tuple>

namespace py = boost::python;

template<typename T>
py::tuple kmeans(const std::vector<std::vector<T>>& data,
                 int k,
                 int maxiter = 200,
                 std::string distance_measure = "euclidian") {
    auto result = metric::kmeans(data, k, maxiter, distance_measure);
    return py::make_tuple(std::get<0>(result), std::get<1>(result), std::get<2>(result));
}

template <typename T>
void register_wrapper_kmeans() {
    py::def("kmeans", &kmeans<T>,
        (
            py::arg("data"),
            py::arg("k") = 0.5,
            py::arg("maxiter") = 200,
            py::arg("distance_measure") = "euclidian"   // TODO: fix typo
        )
    );
}

void export_metric_kmeans() {
    register_wrapper_kmeans<double>();
}
