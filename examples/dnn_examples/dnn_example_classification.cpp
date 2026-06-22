#include "metric/utils/datasets.hpp"
#include "metric/solve/parametric/dnn.hpp"
#include "metric/solve/parametric/dnn/core/Utils/Random.h"
// #include "metric/mapping/autoencoder.hpp"

#include <chrono>
#include <iostream>

using namespace std;
using namespace mtrc::solve::parametric::dnn;

template <typename T> void printMatrix(const T &m)
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

	// Autoencoder<uint8_t, double> autoencoder(features, shape[1] * shape[2], 255);

	Network<double> network;
	/*network.addLayer(FullyConnected<double, ReLU<double>>(28 * 28, 200));
	network.addLayer(FullyConnected<double, ReLU<double>>(200, 80));
	network.addLayer(FullyConnected<double, Sigmoid<double>>(80, 10));
	*/

	network.addLayer(Conv2d<double, ReLU<double>>(28, 28, 1, 1, 3, 3));
	// Spatial downsampling 26x26 -> 13x13. MaxPooling is intentionally unsupported by the native
	// solver (see metric/solve/parametric/dnn/core/Layer/MaxPooling.h); a stride-2 convolution is
	// the supported way to downsample. (26 - 2) / 2 + 1 = 13, so the output is 13*13*1 = 169.
	network.addLayer(Conv2d<double, ReLU<double>>(26, 26, 1, 1, 2, 2, 2));
	// network.addLayer(Conv2d<double, ReLU<double>>(26, 26, 64, 8, 3, 3));
	// network.addLayer(Conv2d<double, ReLU<double>>(26, 26, 8, 8, 3, 3));
	// network.addLayer(FullyConnected<double, Sigmoid<double>>(169, 10));
	network.addLayer(FullyConnected<double, Sigmoid<double>>(169, 10));

	network.setOptimizer(RMSProp<double>());

	network.setOutput(MultiClassEntropy<double>());
	// network.setOutput(RegressionMSE<double>());

	network.setCallback(VerboseCallback<double>());

	network.init(0, 0.01, 123);

	/* Convert input data */
	std::vector<double> featuresScalar(features.begin(), features.end());
	mtrc::numeric::DynamicMatrix<double> featuresNumeric(shape[0], shape[1] * shape[2], featuresScalar.data());
	featuresNumeric /= double(255);

	std::vector<int> labelsInteger(labels.begin(), labels.end());
	// mtrc::numeric::DynamicMatrix<int, mtrc::numeric::columnMajor> labelsNumeric(shape[0], 1,
	// labelsInteger.data());
	mtrc::numeric::DynamicMatrix<double, mtrc::numeric::columnMajor> labelsNumeric(shape[0], 10);
	labelsNumeric = 0;
	for (auto i = 0; i < labels.size(); ++i) {
		labelsNumeric(i, labelsInteger[i]) = 1;
	}

	network.fit(featuresNumeric, labelsNumeric, 1024, 10, 123);

	auto prediction = network.predict(mtrc::numeric::submatrix(featuresNumeric, 10, 0, 1, featuresNumeric.columns()));
	// std::cout << labelsNumeric(10, 0) << std::endl;
	printMatrix(mtrc::numeric::submatrix(labelsNumeric, 10, 0, 1, 10));
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
