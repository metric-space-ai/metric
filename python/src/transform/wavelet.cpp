#include "modules/transform/wavelet.hpp"

#include <boost/python.hpp>
#include <vector>
#include <tuple>

namespace py = boost::python;

void export_metric_wavelet() {
    using T = double;
    using Container = std::vector<T>;
    py::def("dwt", +[](const Container& x, int waveletType) {
        auto result = wavelet::dwt(x, waveletType);
        return py::make_tuple(std::get<0>(result), std::get<1>(result));
    });
    py::def("idwt", &wavelet::idwt<Container>);
    py::def("wmaxlev", &wavelet::wmaxlev);
}
