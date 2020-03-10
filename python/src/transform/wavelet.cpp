#include "modules/transform/wavelet.hpp"

#include <boost/python.hpp>
#include <vector>
#include <tuple>

namespace bp = boost::python;

void export_metric_wavelet() {
    using T = double;
    using Container = std::vector<T>;
    bp::def("dwt", +[](const Container& x, int waveletType) {
        auto result = wavelet::dwt(x, waveletType);
        return bp::make_tuple(std::get<0>(result), std::get<1>(result));
    });
    bp::def("idwt", &wavelet::idwt<Container>);
    bp::def("wmaxlev", &wavelet::wmaxlev);
}
