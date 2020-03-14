#include "modules/mapping/kmedoids.hpp"

#include <boost/python.hpp>
#include <vector>
#include <tuple>

namespace py = boost::python;

template <typename recType, typename Metric, typename T = typename Metric::distance_type>
py::tuple kmedoids(const metric::Matrix<recType, Metric>& DM, int k) {
    auto result = metric::kmedoids(DM, k);
    return py::make_tuple(std::get<0>(result), std::get<1>(result), std::get<2>(result));
}

template <typename recType, typename Metric, typename T>
void register_wrapper_kmedoids() {
    py::def("kmedoids", &kmedoids<recType, Metric, T>,
        (
            py::arg("dm"),
            py::arg("k")
        )
    );
}

void export_metric_kmedoids() {
    register_wrapper_kmedoids<std::vector<double>, metric::Euclidian<double>, double>();
}
