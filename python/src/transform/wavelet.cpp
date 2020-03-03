#include "modules/transform/wavelet_new.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

void export_metric_wavelet() {
    using T = double;
    // using Container = std::vector<T>;    // Container unknown, uncomment if needed other container than std::vector
    bp::def<std::tuple<std::vector<T>, std::vector<T>> (*) (const std::vector<T>&, int)>("dwt", wavelet::dwt<T>);
    // bp::def("dwt", wavelet::dwt<Container>);
    bp::def<std::vector<T> (*)(std::vector<T>, std::vector<T>, int, int)>("idwt", &wavelet::idwt<T>);
    // bp::def("idwt", &wavelet::idwt<Container>);
    bp::def("wmaxlev", &wavelet::wmaxlev);
}
