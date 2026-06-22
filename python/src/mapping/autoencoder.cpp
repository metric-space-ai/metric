#include "metric/mapping/autoencoder.hpp"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <vector>

namespace py = pybind11;

template <typename InputDataType, typename Scalar> void register_wrapper_autoencoder(py::module &m)
{
	using Mapping = mtrc::Autoencoder<InputDataType, Scalar>;
	auto encoder = py::class_<Mapping>(m, "Autoencoder");
	encoder.def(py::init<const std::string &>(), py::arg("jsonString"));
	encoder.def("train", &Mapping::train);
	encoder.def("encode", &Mapping::encode);
	encoder.def("decode", &Mapping::decode);
	encoder.def("predict", &Mapping::predict);
}

void export_metric_autoencoder(py::module &m) { register_wrapper_autoencoder<uint8_t, double>(m); }
