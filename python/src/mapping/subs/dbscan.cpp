#include "modules/mapping/dbscan.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

template<typename recType, typename Metric, typename T>
void register_wrapper_dbscan() {
    bp::def("dbscan", &metric::dbscan<recType, Metric, T>);
}

void export_metric_dbscan() {
    register_wrapper_dbscan<std::vector<float>, metric::Euclidian<float>, float>();
}

BOOST_PYTHON_MODULE(_dbscan) {
    export_metric_dbscan();
}