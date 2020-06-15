#include "modules/mapping/Redif.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

template <typename Value, typename Metric>
void register_wrapper_Redif(py::module& m) {
    using Class = metric::Redif<Value, Metric>;
    using Container = std::vector<std::vector<Value>>;

    auto cls = py::class_<Class>(m, "Redif");
    cls.def(py::init<const Container&, size_t, size_t, Metric>(),
        py::arg("train_data"),
        py::arg("n_neighbors") = 10,
        py::arg("n_iter") = 15,
        py::arg("metric") = Metric()
    );
    std::tuple<Container, std::vector<size_t>> (Class::*encode_vector)(const Container&) = &Class::encode;
    cls.def("encode", encode_vector,
        py::arg("x")
	);
    Container (Class::*decode_vector)(const Container&, const std::vector<size_t>&) = &Class::decode;
    cls.def("decode", decode_vector,
        py::arg("xEncoded"),
        py::arg("l_idx")
	);
}

void export_metric_Redif(py::module& m) {
    using Value = double;
    register_wrapper_Redif<Value, metric::Euclidean<Value>>(m);
}

PYBIND11_MODULE(redif, m) {
    export_metric_Redif(m);
}