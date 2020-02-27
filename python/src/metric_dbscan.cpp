#include "modules/mapping.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

template<typename recType, typename Metric, typename T>
void register_wrapper_dbscan() {
    using Matrix = metric::Matrix<recType, Metric, T>;
    bp::def<const Metric&, T eps, std::size_t minpts>("dbscan", &metric::dbscan<recType, Metric, T>)
}

void export_metric_dbscan() {
    register_wrapper_dbscan<double, metric::Grid4,>();
}
