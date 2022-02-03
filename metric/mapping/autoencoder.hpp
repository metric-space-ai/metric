#include "../utils/dnn.hpp"

#include <chrono>

namespace metric {
template <typename InputDataType, typename Scalar> class Autoencoder : public dnn::Network<Scalar> {
  private:
	using typename dnn::Network<Scalar>::Matrix;

	Matrix trainData;
	Scalar normValue;

	Matrix convertData(const std::vector<InputDataType> &inputData);
	std::vector<InputDataType> convertToOutput(const Matrix &data, bool doDenormalization = true);

	/* Load data (previous data will be erased) */
	void loadTrainData(const std::vector<InputDataType> data);

  public:
	/* Empty constructor */
	Autoencoder();

	/* Construct with provided data */
	Autoencoder(const std::string &jsonString);

	~Autoencoder() = default;

	void setNormValue(InputDataType _normValue = 0);

	void train(const std::vector<InputDataType> data, size_t epochs, size_t batchSize);

	/* Return latent vector */
	std::vector<Scalar> encode(const std::vector<InputDataType> &data);

	/* Return data with size samplesSize * featuresLength */
	std::vector<InputDataType> decode(const std::vector<Scalar> &data);

	/* Predict by autoencoder */
	std::vector<InputDataType> predict(const std::vector<InputDataType> data);
};

template <typename InputDataType, typename Scalar> Autoencoder<InputDataType, Scalar>::Autoencoder() : normValue(255) {}

template <typename InputDataType, typename Scalar>
Autoencoder<InputDataType, Scalar>::Autoencoder(const std::string &jsonString) : Autoencoder()
{
	this->constructFromJsonString(jsonString);
}

template <typename InputDataType, typename Scalar>
void Autoencoder<InputDataType, Scalar>::loadTrainData(const std::vector<InputDataType> data)
{
	trainData = convertData(data);
}

template <typename InputDataType, typename Scalar>
void Autoencoder<InputDataType, Scalar>::train(const std::vector<InputDataType> data, size_t epochs, size_t batchSize)
{
	loadTrainData(data);

	this->fit(trainData, trainData, batchSize, epochs, 123);
}

template <typename InputDataType, typename Scalar>
std::vector<InputDataType> Autoencoder<InputDataType, Scalar>::predict(const std::vector<InputDataType> data)
{
	auto t1 = std::chrono::high_resolution_clock::now();
	auto prediction = dnn::Network<Scalar>::predict(convertData(data));
	auto t2 = std::chrono::high_resolution_clock::now();
	auto d = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	std::cout << "Prediction time: " << d.count() << " s" << std::endl;

	return convertToOutput(prediction);
}

template <typename InputDataType, typename Scalar>
typename Autoencoder<InputDataType, Scalar>::Matrix
Autoencoder<InputDataType, Scalar>::convertData(const std::vector<InputDataType> &inputData)
{
	assert(this->num_layers() > 0);

	/* Convert features to scalar type */
	std::vector<Scalar> dataScalar(inputData.begin(), inputData.end());

	auto featuresLength = this->layers[0]->inputSize;
	Matrix data(dataScalar.size() / featuresLength, featuresLength, dataScalar.data());

	/* Norm features [0..1] */
	if (normValue != 0) {
		data /= Scalar(normValue);
	}

	return data;
}

template <typename InputDataType, typename Scalar>
void Autoencoder<InputDataType, Scalar>::setNormValue(InputDataType _normValue)
{
	normValue = _normValue;
}

template <typename InputDataType, typename Scalar>
std::vector<InputDataType> Autoencoder<InputDataType, Scalar>::convertToOutput(const Matrix &data,
																			   bool doDenormalization)
{
	Matrix temp(data);
	if (doDenormalization && (normValue != 0)) {
		temp *= normValue;
	}

	std::vector<InputDataType> output;
	for (auto i = 0; i < temp.rows(); ++i) {
		auto dataPointer = blaze::row(temp, i).data();
		std::vector<Scalar> vectorScalar(dataPointer, dataPointer + temp.columns());

		output.insert(output.end(), vectorScalar.begin(), vectorScalar.end());
	}

	return output;
}

template <typename InputDataType, typename Scalar>
std::vector<Scalar> Autoencoder<InputDataType, Scalar>::encode(const std::vector<InputDataType> &data)
{
	assert(this->num_layers() % 2 == 0);

	auto input = convertData(data);

	const size_t encoderLastLayerNumber = this->num_layers() / 2;

	this->layers[0]->forward(input);
	for (size_t i = 1; i < encoderLastLayerNumber; i++) {
		this->layers[i]->forward(this->layers[i - 1]->output());
	}

	Matrix output = this->layers[encoderLastLayerNumber - 1]->output();

	std::vector<Scalar> vectorScalar(output.data(), output.data() + output.columns());

	return vectorScalar;
}

template <typename InputDataType, typename Scalar>
std::vector<InputDataType> Autoencoder<InputDataType, Scalar>::decode(const std::vector<Scalar> &data)
{
	assert(this->num_layers() % 2 == 0);

	Matrix latentVector(1, data.size(), data.data());

	const size_t decoderFirstLayerNumber = this->num_layers() / 2;

	this->layers[decoderFirstLayerNumber]->forward(latentVector);
	for (size_t i = decoderFirstLayerNumber + 1; i < this->num_layers(); i++) {
		this->layers[i]->forward(this->layers[i - 1]->output());
	}

	Matrix output = this->layers[this->num_layers() - 1]->output();

	return convertToOutput(output);
}

} // namespace metric
