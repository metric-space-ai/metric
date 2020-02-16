#include "../../modules/utils/dnn.hpp"
#include "../../modules/utils/dnn/Utils/Random.h"
#include "../../modules/utils/dnn/Utils/datasets.hpp"
//#include "../../modules/mapping/autoencoder.hpp"

#include <iostream>
#include <chrono>


using namespace std;
using namespace metric::dnn;

template<typename T>
void printMatrix(const T& m)
{
	cout << setw(3);
	for (auto i = 0; i < m.rows(); ++i) {
		for (auto j = 0; j < m.columns(); ++j) {
			cout << setw(3) << m(i, j) << " ";
		}
		cout << endl;
	}
}


int main()
{
	/* Load data */
	Datasets datasets;
	auto [labels, shape, features] = datasets.getMnist("data.cereal");

	if (shape.empty()) {
		cout << "Data file is empty. Exiting." << endl;
		return EXIT_FAILURE;
	}


	//Autoencoder<uint8_t, double> autoencoder(features, shape[1] * shape[2], 255);

	Network<double> network;
	/*network.addLayer(FullyConnected<double, ReLU<double>>(28 * 28, 200));
	network.addLayer(FullyConnected<double, ReLU<double>>(200, 80));
	network.addLayer(FullyConnected<double, Sigmoid<double>>(80, 10));
	*/

	network.addLayer(Conv2d<double, ReLU<double>>(28, 28, 1, 1, 3, 3));
	network.addLayer(MaxPooling<double, Identity<double>>(26, 26, 1, 2, 2));
	//network.addLayer(Conv2d<double, ReLU<double>>(26, 26, 64, 8, 3, 3));
	//network.addLayer(Conv2d<double, ReLU<double>>(26, 26, 8, 8, 3, 3));
	//network.addLayer(FullyConnected<double, Sigmoid<double>>(169, 10));
	network.addLayer(FullyConnected<double, Softmax<double>>(169, 10));

	network.setOptimizer(RMSProp<double>());

	network.setOutput(MultiClassEntropy<double>());
	//network.setOutput(RegressionMSE<double>());

	network.setCallback(VerboseCallback<double>());

	network.init(0, 0.01, 123);


	/* Convert input data */
	std::vector<double> featuresScalar(features.begin(), features.end());
	blaze::DynamicMatrix<double> featuresBlaze(shape[0], shape[1] * shape[2], featuresScalar.data());
	featuresBlaze /= double(255);

	std::vector<int> labelsInteger(labels.begin(), labels.end());
	//blaze::DynamicMatrix<int, blaze::columnMajor> labelsBlaze(shape[0], 1, labelsInteger.data());
	blaze::DynamicMatrix<double, blaze::columnMajor> labelsBlaze(shape[0], 10);
	labelsBlaze = 0;
	for (auto i = 0; i < labels.size(); ++i) {
		labelsBlaze(i, labelsInteger[i]) = 1;
	}


	network.fit(featuresBlaze, labelsBlaze, 1024, 10, 123);

	auto prediction = network.predict(blaze::submatrix(featuresBlaze, 10, 0, 1, featuresBlaze.columns()));
	//std::cout << labelsBlaze(10, 0) << std::endl;
	printMatrix(blaze::submatrix(labelsBlaze, 10, 0, 1, 10));
	printMatrix(prediction);

/*
	cout << "Train" << endl;
	autoencoder.train(1, 256);

	cout << "Sample:" << endl;
	vector<uint8_t> sample(features.begin(), features.begin() + shape[1] * shape[2]);
	printMatrix(sample, shape[1], shape[2]);

	cout << "prediction" << endl;
	auto prediction = autoencoder.predict(sample);
	printMatrix(prediction, shape[1], shape[2]);

	cout << "latent vector" << endl;
	vector<double> latentVector = autoencoder.encode(sample);
	printVector(latentVector);

	float t = vectorDiff(prediction, autoencoder.decode(latentVector));
	cout << "test:" << t << endl;
*/
	return EXIT_SUCCESS;
}
