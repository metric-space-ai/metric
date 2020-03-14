#include "modules/mapping/autoencoder.hpp"

#include <boost/python.hpp>
#include <vector>
#include <string>

namespace py = boost::python;

template<typename InputDataType, typename Scalar>
void register_wrapper_autoencoder() {
    using Mapping = metric::Autoencoder<InputDataType, Scalar>;
    auto encoder = py::class_<Mapping>("Autoencoder");
    encoder.def(py::init<const std::string&>((py::arg("jsonString"))));
    encoder.def("train", &Mapping::train);
    encoder.def("encode", &Mapping::encode);
    encoder.def("decode", &Mapping::decode);
    encoder.def("predict", &Mapping::predict);
}

void export_metric_autoencoder() {
    register_wrapper_autoencoder<uint8_t, double>();
}
