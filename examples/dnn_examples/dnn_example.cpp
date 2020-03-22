#include "../../modules/utils/dnn.hpp"
#include "../../modules/utils/dnn/Utils/Random.h"
#include "../../modules/utils/datasets.hpp"
#include "../../modules/mapping/autoencoder.hpp"

#include <iostream>
#include <chrono>


using namespace std;
using namespace metric;


template <typename Scalar>
blaze::DynamicMatrix<Scalar> getRandomMatrix(const size_t rows, const size_t columns)
{
	blaze::DynamicMatrix<Scalar> m(rows, columns);

	for (size_t i = 0UL; i < rows; i++) {
		for (size_t j = 0UL; j < columns; j++) {
			m(i, j) = blaze::rand<Scalar>(-1, 1);
		}
	}

	return m;
}

template<typename T>
void printVector(const T& vector)
{
	for (auto e: vector) {
		cout << +e << " ";
	}
	cout << endl;
}

template<typename T>
void printMatrix(const T& vector, size_t rows, size_t columns)
{
	cout << setw(3);
	for (auto i = 0; i < rows; ++i) {
		for (auto j = 0; j < columns; ++j) {
			cout << setw(3) << +vector[i * columns + j] << " ";
		}
		cout << endl;
	}
}

template<typename T>
int vectorDiff(const vector<T>& vector1, const vector<T>& vector2)
{
	if (vector1.size() != vector2.size()) {
		cout << "v1.size() != v2.size()" << endl;
		return -1;
	}

	int diff = 0;
	for (size_t i = 0; i < vector1.size(); ++i) {
		diff += (int(vector1[i]) - int(vector2[i]));
	}

	return diff;
}

int main()
{
	/* Load data */
	Datasets datasets;
	/* shape: [batches, rows, cols, channels] */
	auto [labels, shape, features] = datasets.getMnist("data.cereal");

	if (shape.empty()) {
		cout << "Data file is empty. Exiting." << endl;
		return EXIT_FAILURE;
	}


	auto json = R"({
					"0":
						{
							"type": "FullyConnected",
							"inputSize": 784,
							"outputSize": 128,
							"activation": "ReLU"
						},
					"1":
						{
							"type": "FullyConnected",
							"inputSize": 128,
							"outputSize": 64,
							"activation": "ReLU"
						},
					"2":
						{
							"type": "FullyConnected",
							"inputSize": 64,
							"outputSize": 32,
							"activation": "ReLU"
						},
					"3":
						{
							"type": "FullyConnected",
							"inputSize": 32,
							"outputSize": 64,
							"activation": "ReLU"
						},
					"4":
						{
							"type": "FullyConnected",
							"inputSize": 64,
							"outputSize": 128,
							"activation": "ReLU"
						},
					"5":
						{
							"type": "FullyConnected",
							"inputSize": 128,
							"outputSize": 784,
							"activation": "Sigmoid"
						},
					"train":
						{
							"loss": "RegressionMSE",
							"optimizer": {"type": "RMSProp",
											"learningRate": 0.01,
											"eps": 1e-6,
											"decay": 0.9}
						}
					}
				)"_json;


	auto jsonConv = R"({
					"0":
						{
							"type": "Conv2d",
							"inputWidth": 28,
							"inputHeight": 28,
							"inputChannels": 1,
							"outputChannels": 16,
							"kernelWidth": 4,
							"kernelHeight": 4,
							"stride": 2,
							"activation": "ReLU"
						},
					"1":
						{
							"type": "Conv2dTranspose",
							"inputWidth": 13,
							"inputHeight": 13,
							"inputChannels": 16,
							"outputChannels": 1,
							"kernelWidth": 4,
							"kernelHeight": 4,
							"stride": 2,
							"activation": "Sigmoid"
						},
					"train":
						{
							"loss": "RegressionMSE",
							"optimizer": {"type": "RMSProp",
											"learningRate": 0.01,
											"eps": 1e-6,
											"decay": 0.9}
						}
					}
				)"_json;

	Autoencoder<uint8_t, double> autoencoder(jsonConv.dump());
	autoencoder.setCallback(dnn::VerboseCallback<double>());

	cout << "Train" << endl;
	autoencoder.train(features, 1, 256);

	cout << "Sample:" << endl;
	vector<uint8_t> sample(features.begin(), features.begin() + shape[1] * shape[2]);
	printMatrix(sample, shape[1], shape[2]);

	cout << "prediction" << endl;
	auto prediction = autoencoder.predict(sample);
	printMatrix(prediction, shape[1], shape[2]);

	cout << "latent vector" << endl;
	vector<double> latentVector = autoencoder.encode(sample);
	printVector(latentVector);

	int t = vectorDiff(prediction, autoencoder.decode(latentVector));
	cout << "test:" << t << endl;

	return EXIT_SUCCESS;
}
