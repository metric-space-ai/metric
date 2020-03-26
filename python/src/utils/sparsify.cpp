#include "modules/utils/graph/sparsify.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

template<typename Tv>
void register_wrapper_sparsify(py::module& m) {

    m.def("sparsify_effective_resistance", &metric::sparsify_effective_resistance<Tv>);
    m.def("sparsify_spanning_tree", &metric::sparsify_spanning_tree<Tv>);
}

void export_metric_sparsify(py::module& m) {
    register_wrapper_sparsify<double>(m);
}