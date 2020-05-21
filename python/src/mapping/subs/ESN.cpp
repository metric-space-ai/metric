#include "modules/distance/k-related/Standards.hpp"
#include "modules/mapping/ESN.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

template <typename RecType, typename Metric>
void register_wrapper_ESN(py::module& m) {
    using Class = metric::ESN<RecType, Metric>;
    using Container = std::vector<RecType>;
    auto esn = py::class_<Class>(m, "ESN");
    esn.def(py::init<size_t, double, double, double, size_t, double>(),
        py::arg("w_size") = 500,
        py::arg("w_connections") = 10,
        py::arg("w_sr") = 0.6,
        py::arg("alpha") = 0.5,
        py::arg("washout") = 1,
        py::arg("beta") = 0.5
    );
    void (Class::*train)(const Container&, const Container&) = &Class::train;
    Container (Class::*predict)(const Container&) = &Class::predict;
    esn.def("train", train);
    esn.def("predict", predict);
}

void export_metric_ESN(py::module& m) {
    using Value = double;
    using RecType = std::vector<Value>;
    register_wrapper_ESN<RecType, metric::Euclidean<Value>>(m);
}

PYBIND11_MODULE(esn, m) {
    export_metric_ESN(m);
}