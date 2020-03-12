#include "modules/mapping/kmeans.hpp"

#include <boost/python.hpp>
#include <vector>
#include <tuple>

namespace bp = boost::python;

template<typename T>
bp::tuple kmeans(const std::vector<std::vector<T>>& data,
                 int k,
                 int maxiter = 200,
                 std::string distance_measure = "euclidian") {
    auto result = metric::kmeans(data, k, maxiter, distance_measure);
    return bp::make_tuple(std::get<0>(result), std::get<1>(result), std::get<2>(result));
}

template <typename T>
void register_wrapper_kmeans() {
    bp::def("kmeans", &kmeans<T>,
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
