#include "modules/utils/graph/sparsify.hpp"
#include <boost/python.hpp>

namespace bp = boost::python;

template<typename Tv>
void register_wrapper_sparsify() {

    bp::def("sparsify_effective_resistance", &metric::sparsify_effective_resistance<Tv>);
    bp::def("sparsify_spanning_tree", &metric::sparsify_spanning_tree<Tv>);
}

void export_metric_sparsify() {
    register_wrapper_sparsify<double>();
}