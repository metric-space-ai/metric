#include "modules/transform/wavelet.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

void export_metric_wavelet(py::module& m) {
    using T = double;
    using Container = std::vector<T>;
    m.def("dwt", &wavelet::dwt<Container>,
        py::arg("x"),
        py::arg("wavelet_type")
    );
    m.def("idwt", &wavelet::idwt<Container>);
    m.def("wmaxlev", &wavelet::wmaxlev);
}
