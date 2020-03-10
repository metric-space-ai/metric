#include "modules/mapping/kmedoids.hpp"

#include <boost/python.hpp>
#include <vector>
#include <tuple>

namespace bp = boost::python;

template <typename recType, typename Metric, typename T = typename Metric::distance_type>
bp::tuple kmedoids(const metric::Matrix<recType, Metric>& DM, int k) {
    auto result = metric::kmedoids(DM, k);
    return bp::make_tuple(std::get<0>(result), std::get<1>(result), std::get<2>(result));
}

template <typename recType, typename Metric, typename T>
void register_wrapper_kmedoids() {
    bp::def("kmedoids", &kmedoids<recType, Metric, T>,
        (
            bp::arg("dm"),
            bp::arg("k")
        )
    );
}

void export_metric_kmedoids() {
    register_wrapper_kmedoids<std::vector<double>, metric::Euclidian<double>, double>();
}
