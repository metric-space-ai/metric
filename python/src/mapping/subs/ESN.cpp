#include "modules/mapping/ESN.hpp"

#include "../../metric_types.hpp"

#include <boost/python.hpp>
#include <vector>

namespace py = boost::python;

template <typename recType, typename Metric>
void register_wrapper_ESN() {
    using Mapping = metric::ESN<recType, Metric>;
    using Container = std::vector<recType>;
    auto dspcc = py::class_<Mapping>("ESN", py::init<size_t, double, double, double, size_t, double>(
        (
            py::arg("w_size") = 500,
            py::arg("w_connections") = 10,
            py::arg("w_sr") = 0.6,
            py::arg("alpha") = 0.5,
            py::arg("washout") = 1,
            py::arg("beta") = 0.5
        )
    ));
    void (Mapping::*train)(const Container&, const Container&) = &Mapping::train;
    Container (Mapping::*predict)(const Container&) = &Mapping::predict;
    dspcc.def("train", train);
    dspcc.def("predict", predict);
}

void export_metric_ESN() {
    register_wrapper_ESN<std::vector<double>, metric::Euclidian<double>>();
}

BOOST_PYTHON_MODULE(_esn) {
    export_metric_ESN();
}