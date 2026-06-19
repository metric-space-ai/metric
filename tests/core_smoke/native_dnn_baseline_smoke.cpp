#include <cassert>
#include <cmath>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include <blaze/Math.h>

#include "metric/mapping/autoencoder.hpp"
#include "metric/utils/dnn.hpp"

namespace {

using Matrix = blaze::DynamicMatrix<double>;
using RowVector = blaze::DynamicVector<double, blaze::rowVector>;

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool
{
	return std::abs(lhs - rhs) <= tolerance;
}

auto small_autoencoder_json() -> std::string
{
	return R"({
		"0": {
			"type": "FullyConnected",
			"inputSize": 2,
			"outputSize": 1,
			"activation": "Identity"
		},
		"1": {
			"type": "FullyConnected",
			"inputSize": 1,
			"outputSize": 2,
			"activation": "Identity"
		},
		"train": {
			"loss": "RegressionMSE",
			"optimizer": {
				"type": "RMSProp",
				"learningRate": 0.001,
				"eps": 1e-8,
				"decay": 0.99
			}
		}
	})";
}

auto assert_same_shape(const Matrix &matrix, std::size_t rows, std::size_t columns) -> void
{
	assert(matrix.rows() == rows);
	assert(matrix.columns() == columns);
}

} // namespace

int main()
{
	static_assert(std::is_base_of<metric::dnn::Optimizer<double>, metric::dnn::RMSProp<double>>::value);

	{
		metric::dnn::RegressionMSE<double> mse;
		const Matrix predicted{{1.0, 2.0}, {3.0, 4.0}};
		const Matrix target{{1.0, 1.0}, {5.0, 4.0}};

		mse.evaluate(predicted, target);
		assert_same_shape(mse.backprop_data(), 2, 2);
		assert(close(mse.backprop_data()(0, 0), 0.0));
		assert(close(mse.backprop_data()(0, 1), 2.0));
		assert(close(mse.backprop_data()(1, 0), -4.0));
		assert(close(mse.backprop_data()(1, 1), 0.0));
		assert(close(mse.loss(), 2.5));
	}

	{
		metric::dnn::RMSProp<double> optimizer(0.0001, 1.0e-8, 0.99);
		RowVector weights{-0.021, 1.03, -0.05, -0.749, 0.009};
		const RowVector gradients{1.0, -3.24, -0.60, 2.79, 1.82};

		optimizer.update(gradients, weights);
		assert(close(weights[0], -0.0220, 1.0e-4));
		assert(close(weights[1], 1.0310, 1.0e-4));
		assert(close(weights[2], -0.0490, 1.0e-4));
		assert(close(weights[3], -0.7500, 1.0e-4));
		assert(close(weights[4], 0.0080, 1.0e-4));

		optimizer.update(gradients, weights);
		assert(close(weights[0], -0.0227, 1.0e-4));
		assert(close(weights[1], 1.0317, 1.0e-4));
		assert(close(weights[2], -0.0483, 1.0e-4));
		assert(close(weights[3], -0.7507, 1.0e-4));
		assert(close(weights[4], 0.0073, 1.0e-4));
	}

	{
		metric::dnn::Network<double> network;
		network.addLayer(metric::dnn::FullyConnected<double, metric::dnn::Identity<double>>(2, 2));
		network.setOutput(metric::dnn::RegressionMSE<double>());
		network.setOptimizer(metric::dnn::RMSProp<double>(0.001, 1.0e-8, 0.99));
		network.init(0.0, 0.01, 7);

		const Matrix samples{{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}};
		const auto before = network.predict(samples);
		assert_same_shape(before, 3, 2);

		const auto fit_result = network.fit(samples, samples, 2, 2, 11);
		assert(fit_result);
		const auto after = network.predict(samples);
		assert_same_shape(after, 3, 2);
	}

	{
		metric::Autoencoder<double, double> autoencoder(small_autoencoder_json());
		autoencoder.setNormValue(0);
		const std::vector<double> samples{0.0, 1.0, 1.0, 0.0};
		autoencoder.train(samples, 1, 1);

		const std::vector<double> sample{0.25, 0.75};
		const auto prediction = autoencoder.predict(sample);
		const auto latent = autoencoder.encode(sample);
		const auto decoded = autoencoder.decode(latent);
		assert(prediction.size() == sample.size());
		assert(latent.size() == 1);
		assert(decoded.size() == sample.size());

		std::stringstream archive;
		autoencoder.save(archive);

		metric::Autoencoder<double, double> loaded;
		loaded.setNormValue(0);
		loaded.load(archive);
		const auto loaded_prediction = loaded.predict(sample);
		assert(loaded_prediction.size() == prediction.size());
		for (std::size_t index = 0; index < prediction.size(); ++index) {
			assert(close(loaded_prediction[index], prediction[index], 1.0e-9));
		}
	}

	return 0;
}
