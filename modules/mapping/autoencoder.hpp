#ifndef _METRIC_MAPPING_AUTOENCODER_HPP
#define _METRIC_MAPPING_AUTOENCODER_HPP

#include "../utils/dnn.hpp"

#include <chrono>

namespace metric
{

template<typename InputDataType, typename Scalar>
class Autoencoder : public dnn::Network<Scalar>
{
public:
    /* Empty constructor */
    Autoencoder();

    /* Construct with provided data */
    Autoencoder(const std::string &jsonString);

    ~Autoencoder() = default;

    void setNormValue(InputDataType _normValue = 0);

    void train(const std::vector<InputDataType>& data, size_t epochs, size_t batchSize);

    /* Return latent vector */
    std::vector<Scalar> encode(const std::vector<InputDataType> &data);

    /* Return data with size samplesSize * featuresLength */
    std::vector<InputDataType> decode(const std::vector<Scalar>& data);

    /* Predict by autoencoder */
    std::vector<InputDataType> predict(const std::vector<InputDataType> data);

private:
    using typename dnn::Network<Scalar>::Matrix;

    Matrix trainData;
    Scalar normValue;

    Matrix convertData(const std::vector<InputDataType> &inputData);
    std::vector<InputDataType> convertToOutput(const Matrix &data, bool doDenormalization = true);

    /* Load data (previous data will be erased) */
    void loadTrainData(const std::vector<InputDataType> data);
};

}

#include "autoencoder.cpp"

#endif
