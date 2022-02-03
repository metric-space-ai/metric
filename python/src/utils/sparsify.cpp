#include "metric/utils/graph/sparsify.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

template<typename Tv>
void register_wrapper_sparsify(py::module& m) {
    m.def("sparsify_effective_resistance", &metric::sparsify_effective_resistance<Tv>,
        "Apply Spielman-Srivastava sparsification: sampling by effective resistances.",
        py::arg("a"),
        py::arg("ep") = 0.3,
        py::arg("matrix_conc_const") = 4.0,
        py::arg("jl_fac") = 4.0
    );
    m.def("sparsify_spanning_tree", &metric::sparsify_spanning_tree<Tv>,
        "Apply Kruskal's algorithm.",
        py::arg("a"),
        py::arg("minimum") = false
    );
}

void export_metric_sparsify(py::module& m) {
    register_wrapper_sparsify<double>(m);
}