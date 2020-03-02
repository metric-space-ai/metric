#include "modules/mapping/autoencoder.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

template<typename InputDataType, typename Scalar>
void register_wrapper_autoencoder() {
    using Mapping = MiniDNN::Autoencoder<InputDataType, Scalar>;
    auto encoder = bp::class_<Mapping>("Autoencoder");
    encoder.def(bp::init<std::vector<InputDataType>, size_t, InputDataType>(
        (
            bp::arg("input_data"),
            bp::arg("features_length") = 1,
            bp::arg("norm_value") = 0
        )
    ));
    encoder.def("set_data_shape", &Mapping::setDataShape);
    encoder.def("load_train_data", &Mapping::loadTrainData);
    encoder.def("train", &Mapping::train);
    encoder.def("encode", &Mapping::encode);
    encoder.def("decode", &Mapping::decode);
    encoder.def("predict", &Mapping::predict);

}

void export_metric_autoencoder() {
    register_wrapper_autoencoder<uint8_t, double>();
}
