#include "modules/transform/wavelet.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

void export_metric_wavelet() {
    using T = double;
    using Container = std::vector<T>;
    bp::def("dwt", wavelet::dwt<Container>);
    bp::def("idwt", &wavelet::idwt<Container>);
    bp::def("wmaxlev", &wavelet::wmaxlev);
}
