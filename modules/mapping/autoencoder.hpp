#include "../utils/dnn.hpp"

#include <chrono>


namespace MiniDNN
{
	template<typename InputDataType, typename Scalar>
	class Autoencoder
	{
		private:
			using Matrix = blaze::DynamicMatrix<Scalar, blaze::columnMajor>;

			Network<Scalar> net;
			std::shared_ptr <Optimizer<Scalar>> opt;
			Matrix trainData;
			size_t featuresLength;
			Scalar normValue;

			Matrix convertData(const std::vector<InputDataType> &inputData);
			std::vector<InputDataType> convertToOutput(const Matrix &data, bool doDenormalization = true);

		public:
			static constexpr int LATENT_VECTOR_SIZE = 32;

			/* Empty constructor */
			Autoencoder() = default;

			/* Construct with provided data */
			Autoencoder(const std::vector<InputDataType> inputData, size_t featuresLength, InputDataType normValue = 0);

			~Autoencoder() = default;

			void setDataShape(size_t _featuresLength, InputDataType _normValue = 0);

			/* Load data (previous data will be erased) */
			void loadTrainData(const std::vector<InputDataType> data);


			void train(size_t epochs, size_t batchSize);

			/* Return latent vector */
			std::vector<Scalar> encode(const std::vector<InputDataType> &data);

			/* Return data with size samplesSize * featuresLength */
			std::vector<InputDataType> decode(const std::vector<Scalar>& data);

			/* Predict by autoencoder */
			std::vector<InputDataType> predict(const std::vector<InputDataType> data);
	};

	template<typename InputDataType, typename Scalar>
	Autoencoder<InputDataType, Scalar>::Autoencoder(const std::vector<InputDataType> inputData,
	                                                size_t featuresLength, InputDataType normValue) :
	                                                                featuresLength(featuresLength), normValue(normValue)
	{
		/* Create layers */
		net.addLayer(FullyConnected<Scalar, ReLU<Scalar>>(featuresLength, 1024));
		net.addLayer(FullyConnected<Scalar, ReLU<Scalar>>(1024, 256));
		net.addLayer(FullyConnected<Scalar, ReLU<Scalar>>(256, 64));
		net.addLayer(FullyConnected<Scalar, ReLU<Scalar>>(64, 256));
		net.addLayer(FullyConnected<Scalar, ReLU<Scalar>>(256, 1024));
		net.addLayer(FullyConnected<Scalar, ReLU<Scalar>>(1024, featuresLength));
		//net.addLayer(Conv2d<Scalar, ReLU<Scalar>>(featuresLength, 28, 1, 1, 5, 5));
		//net.addLayer(Conv2dTranspose<Scalar, Sigmoid<Scalar>>(24, 24, 1, 1, 5, 5));

		net.setCallback(VerboseCallback<Scalar>());
		/* Set output layer */

		/* Create optimizer object */
		opt = std::make_shared < RMSProp < Scalar >> (RMSProp<Scalar>());
		//opt->learningRate = 0.01;

		/* Set callback function object */

		net.setOutput(RegressionMSE<Scalar>());
		/* Initialize parameters with N(0, 0.01^2) using random seed 123 */
		net.init(0, 0.01, 123);

		loadTrainData(inputData);
	}

	template<typename InputDataType, typename Scalar>
	void Autoencoder<InputDataType, Scalar>::loadTrainData(const std::vector<InputDataType> data)
	{
		trainData = convertData(data);
	}


	template<typename InputDataType, typename Scalar>
	void Autoencoder<InputDataType, Scalar>::train(size_t epochs, size_t batchSize)
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		net.fit(*opt, trainData, trainData, batchSize, epochs, 123);
		auto t2 = std::chrono::high_resolution_clock::now();
		auto d = std::chrono::duration_cast < std::chrono::duration < double >> (t2 - t1);
		std::cout << "Training time: " << d.count() << " s" << std::endl;
	}

	template<typename InputDataType, typename Scalar>
	std::vector<InputDataType> Autoencoder<InputDataType, Scalar>::predict(const std::vector <InputDataType> data)
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		auto prediction = net.predict(convertData(data));
		auto t2 = std::chrono::high_resolution_clock::now();
		auto d = std::chrono::duration_cast < std::chrono::duration < double >> (t2 - t1);
		std::cout << "Prediction time: " << d.count() << " s" << std::endl;

		return convertToOutput(prediction);
	}

	template<typename InputDataType, typename Scalar>
	typename Autoencoder<InputDataType, Scalar>::Matrix Autoencoder<InputDataType, Scalar>::convertData(
			const std::vector<InputDataType> &inputData)
	{
		/* Convert features to scalar type */
		std::vector<Scalar> dataScalar(inputData.begin(), inputData.end());

		Matrix data(featuresLength, dataScalar.size() / featuresLength, dataScalar.data());

		/* Norm features [0..1] */
		if (normValue != 0) {
			data /= Scalar(normValue);
		}
		return data;
	}

	template<typename InputDataType, typename Scalar>
	void Autoencoder<InputDataType, Scalar>::setDataShape(size_t _featuresLength, InputDataType _normValue)
	{
		featuresLength = _featuresLength;
		normValue = _normValue;
	}

	template<typename InputDataType, typename Scalar>
	std::vector<InputDataType> Autoencoder<InputDataType, Scalar>::convertToOutput(const Matrix &data, bool doDenormalization)
	{
		Matrix temp(data);
		if (doDenormalization and (normValue != 0)) {
			temp *= normValue;
		}

		std::vector<InputDataType> output;
		for (auto j = 0; j < temp.columns(); ++j) {
			auto dataPointer = blaze::column(temp, j).data();
			std::vector<Scalar> vectorScalar(dataPointer, dataPointer + temp.rows());
			output.insert(output.end(), vectorScalar.begin(), vectorScalar.end());
		}

		return output;
	}

	template<typename InputDataType, typename Scalar>
	std::vector<Scalar> Autoencoder<InputDataType, Scalar>::encode(const std::vector<InputDataType> &data)
	{
		auto input = convertData(data);
		net.layers[0]->forward(input);

		Matrix output = net.layers[0]->output();

		std::vector<Scalar> vectorScalar(output.data(), output.data() + output.rows());
		return vectorScalar;
	}

	template<typename InputDataType, typename Scalar>
	std::vector<InputDataType> Autoencoder<InputDataType, Scalar>::decode(const std::vector<Scalar> &data)
	{
		Matrix latentVector(data.size(), 1, data.data());
		net.layers[1]->forward(latentVector);

		return convertToOutput(net.layers[1]->output());
	}
}
