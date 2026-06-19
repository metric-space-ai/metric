#include <cassert>
#include <cmath>
#include <vector>

#include <blaze/Math.h>

#include "metric/utils/dnn.hpp"

namespace {

using Matrix = blaze::DynamicMatrix<double>;
using Parameters = std::vector<std::vector<double>>;

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool
{
	return std::abs(lhs - rhs) <= tolerance;
}

auto assert_same_shape(const Matrix &matrix, std::size_t rows, std::size_t columns) -> void
{
	assert(matrix.rows() == rows);
	assert(matrix.columns() == columns);
}

auto parameters_close(const Parameters &lhs, const Parameters &rhs, double tolerance = 1.0e-12) -> bool
{
	if (lhs.size() != rhs.size()) {
		return false;
	}
	for (std::size_t group = 0; group < lhs.size(); ++group) {
		if (lhs[group].size() != rhs[group].size()) {
			return false;
		}
		for (std::size_t index = 0; index < lhs[group].size(); ++index) {
			if (!close(lhs[group][index], rhs[group][index], tolerance)) {
				return false;
			}
		}
	}
	return true;
}

auto parameters_changed(const Parameters &before, const Parameters &after, double tolerance = 1.0e-12) -> bool
{
	return !parameters_close(before, after, tolerance);
}

auto add_identity_dense_layer(metric::dnn::Network<double> &network, std::size_t input_size, std::size_t output_size)
	-> void
{
	metric::dnn::FullyConnected<double, metric::dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.0, 0.0);
	network.addLayer(layer);
}

auto make_two_layer_network() -> metric::dnn::Network<double>
{
	metric::dnn::Network<double> network;
	add_identity_dense_layer(network, 2, 2);
	add_identity_dense_layer(network, 2, 1);
	network.setOptimizer(metric::dnn::RMSProp<double>(0.01, 1.0e-8, 0.0));
	network.layers[0]->setParameters({{1.0, 0.0, 0.0, 1.0}, {0.0, 0.0}});
	network.layers[1]->setParameters({{1.0, -1.0}, {0.5}});
	return network;
}

} // namespace

int main()
{
	{
		metric::dnn::Network<double> network;
		add_identity_dense_layer(network, 2, 1);
		network.setOptimizer(metric::dnn::RMSProp<double>(0.01, 1.0e-8, 0.0));
		network.layers[0]->setParameters({{1.0, 2.0}, {0.5}});

		const Matrix samples{{1.0, 2.0}, {3.0, 4.0}};
		std::vector<Matrix> activations;
		network.forward_all(samples, &activations);
		assert(activations.size() == 1);
		assert_same_shape(activations[0], 2, 1);
		assert(close(activations[0](0, 0), 5.5));
		assert(close(activations[0](1, 0), 11.5));

		const auto before = network.layers[0]->getParameters();
		std::vector<Matrix> gradients(1);
		gradients[0] = Matrix{{1.0}, {1.0}};
		network.backprop_from_layer_gradients(samples, gradients);
		network.apply_optimizer();
		const auto after = network.layers[0]->getParameters();
		assert(parameters_changed(before, after));
	}

	{
		auto network = make_two_layer_network();
		const Matrix samples{{2.0, 3.0}, {4.0, 5.0}};

		std::vector<Matrix> activations;
		network.forward_all(samples, &activations);
		assert(activations.size() == 2);
		assert_same_shape(activations[0], 2, 2);
		assert_same_shape(activations[1], 2, 1);
		assert(close(activations[0](0, 0), 2.0));
		assert(close(activations[0](0, 1), 3.0));
		assert(close(activations[1](0, 0), -0.5));
		assert(close(activations[1](1, 0), -0.5));

		const auto first_before = network.layers[0]->getParameters();
		const auto second_before = network.layers[1]->getParameters();
		std::vector<Matrix> gradients(2);
		gradients[1] = Matrix{{1.0}, {-1.0}};
		network.backprop_from_layer_gradients(samples, gradients);
		network.apply_optimizer();
		assert(parameters_changed(first_before, network.layers[0]->getParameters()));
		assert(parameters_changed(second_before, network.layers[1]->getParameters()));
	}

	{
		auto network = make_two_layer_network();
		const Matrix samples{{2.0, 3.0}, {4.0, 5.0}};
		const auto first_before = network.layers[0]->getParameters();
		const auto second_before = network.layers[1]->getParameters();

		std::vector<Matrix> gradients(2);
		gradients[0] = Matrix{{1.0, 0.0}, {0.0, -1.0}};
		network.backprop_from_layer_gradients(samples, gradients);
		network.apply_optimizer();

		assert(parameters_changed(first_before, network.layers[0]->getParameters()));
		assert(parameters_close(second_before, network.layers[1]->getParameters()));
	}

	return 0;
}
