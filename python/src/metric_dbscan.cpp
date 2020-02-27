#include "modules/mapping.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

template<typename recType, typename Metric, typename T>
void register_wrapper_dbscan() {
    using Mapping = metric::dbscan<recType, Metric, T>;
    using Matrix = metric::Matrix<recType, Metric, T>;
    def<const Metric&, T eps, std::size_t minpts>("dbscan", &metric::dbscan)
}

void export_metric_dbscan() {
    register_wrapper_dbscan<double, metric::Grid4,>();
}
