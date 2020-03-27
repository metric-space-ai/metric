#include "modules/mapping/ESN.hpp"

#include "../../metric_types.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

template <typename recType, typename Metric>
void register_wrapper_ESN(py::module& m) {
    using Mapping = metric::ESN<recType, Metric>;
    using Container = std::vector<recType>;
    auto esn = py::class_<Mapping>(m, "ESN");
    esn.def(py::init<size_t, double, double, double, size_t, double>(),
        py::arg("w_size") = 500,
        py::arg("w_connections") = 10,
        py::arg("w_sr") = 0.6,
        py::arg("alpha") = 0.5,
        py::arg("washout") = 1,
        py::arg("beta") = 0.5
    );
    void (Mapping::*train)(const Container&, const Container&) = &Mapping::train;
    Container (Mapping::*predict)(const Container&) = &Mapping::predict;
    esn.def("train", train);
    esn.def("predict", predict);
}

void export_metric_ESN(py::module& m) {
    register_wrapper_ESN<std::vector<double>, metric::Euclidian<double>>(m);
}

PYBIND11_MODULE(_esn, m) {
    export_metric_ESN(m);
}