#include "modules/transform/wavelet.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <tuple>

namespace py = pybind11;

void export_metric_wavelet(py::module& m) {
    using T = double;
    using Container = std::vector<T>;
    m.def("dwt", +[](const Container& x, int waveletType) {
        auto result = wavelet::dwt(x, waveletType);
        return py::make_tuple(std::get<0>(result), std::get<1>(result));
    });
    m.def("idwt", &wavelet::idwt<Container>);
    m.def("wmaxlev", &wavelet::wmaxlev);
}
