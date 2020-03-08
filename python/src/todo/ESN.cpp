#include "../metric_types.hpp"
#include "modules/mapping/ESN.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

template <typename recType, typename Metric>
void register_wrapper_ESN() {
    using Mapping = metric::ESN<recType, Metric>;
    auto dspcc = bp::class_<Mapping>("ESN", bp::init<size_t, double, double, double, size_t, double>(
        (
            bp::arg("w_size") = 500,
            bp::arg("w_connections") = 10,
            bp::arg("w_sr") = 0.6,
            bp::arg("alpha") = 0.5,
            bp::arg("washout") = 1,
            bp::arg("beta") = 0.5
        )
    ));
    dspcc.def("train", &Mapping::train);
    dspcc.def("predict", &Mapping::predict);
}

void export_metric_ESN() {
    register_wrapper_ESN<double, metric::Euclidian<double>>();
}
