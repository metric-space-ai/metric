// Conv2d backpropagation and SGD smoke test.
//
// This pins the native DNN solver contract used by metric-space fitted maps:
// each observation is one row, parameter derivatives are averaged over
// observations, and upstream activation gradients remain unnormalized for
// earlier layers in the chain.

#include <cassert>
#include <cmath>
#include <cstddef>
#include <sstream>
#include <type_traits>
#include <vector>

#include <metric/numeric/Math.h>

#include "metric/solve/parametric/dnn.hpp"

namespace {

namespace dnn = mtrc::solve::parametric::dnn;

using Matrix = mtrc::numeric::DynamicMatrix<double>;
using RowVector = mtrc::numeric::DynamicVector<double, mtrc::numeric::rowVector>;
using ColumnMatrix = mtrc::numeric::DynamicMatrix<double, mtrc::numeric::columnMajor>;
using Conv2d = dnn::Conv2d<double, dnn::Identity<double>>;

auto close(double lhs, double rhs, double tolerance = 1.0e-7) -> bool
{
	return std::abs(lhs - rhs) <= tolerance * (1.0 + std::abs(lhs));
}

auto weighted_mean_output(Conv2d &layer, const Matrix &input, const Matrix &upstream) -> double
{
	layer.forward(input);
	const auto &output = layer.output();
	assert(output.rows() == upstream.rows());
	assert(output.columns() == upstream.columns());

	double value = 0.0;
	for (std::size_t row = 0; row < output.rows(); ++row) {
		for (std::size_t column = 0; column < output.columns(); ++column) {
			value += output(row, column) * upstream(row, column);
		}
	}
	return value / static_cast<double>(output.rows());
}

auto exercise_conv2d_valid_backprop() -> void
{
	Conv2d layer(/*inputWidth=*/3, /*inputHeight=*/3, /*inputChannels=*/1, /*outputChannels=*/2,
				 /*kernelWidth=*/2, /*kernelHeight=*/2);

	std::vector<std::vector<double>> parameters{
		{0.20, -0.30, 0.40, 0.10, -0.50, 0.25, 0.15, -0.20},
		{0.05, -0.10},
	};
	layer.setParameters(parameters);

	const Matrix input{{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
					   {1.0, -1.0, 0.5, 2.0, -2.0, 1.5, 3.0, -3.0, 2.5}};
	const Matrix upstream{{0.5, -0.25, 0.75, -0.5, 1.0, 0.25, -0.75, 0.5},
						  {-0.4, 0.6, -0.2, 0.8, -0.3, 0.9, 0.7, -0.6}};

	layer.forward(input);
	layer.backprop(input, upstream);
	const auto derivatives = layer.get_derivatives();
	const auto input_gradient = layer.backprop_data();

	const double eps = 1.0e-6;
	for (std::size_t group = 0; group < parameters.size(); ++group) {
		for (std::size_t index = 0; index < parameters[group].size(); ++index) {
			auto perturbed = parameters;
			perturbed[group][index] += eps;
			layer.setParameters(perturbed);
			const double value_plus = weighted_mean_output(layer, input, upstream);

			perturbed[group][index] -= 2.0 * eps;
			layer.setParameters(perturbed);
			const double value_minus = weighted_mean_output(layer, input, upstream);

			layer.setParameters(parameters);
			const double numeric = (value_plus - value_minus) / (2.0 * eps);
			const std::size_t flat_index = group == 0 ? index : parameters[0].size() + index;
			assert(close(derivatives[flat_index], numeric));
		}
	}

	for (std::size_t row = 0; row < input.rows(); ++row) {
		for (std::size_t column = 0; column < input.columns(); ++column) {
			Matrix plus(input);
			Matrix minus(input);
			plus(row, column) += eps;
			minus(row, column) -= eps;
			const double value_plus = weighted_mean_output(layer, plus, upstream);
			const double value_minus = weighted_mean_output(layer, minus, upstream);
			const double numeric = (value_plus - value_minus) / (2.0 * eps);
			const double analytic = input_gradient(row, column) / static_cast<double>(input.rows());
			assert(close(analytic, numeric));
		}
	}
}

auto exercise_conv2d_zero_padding_backprop() -> void
{
	Conv2d layer(/*inputWidth=*/2, /*inputHeight=*/2, /*inputChannels=*/1, /*outputChannels=*/1,
				 /*kernelWidth=*/3, /*kernelHeight=*/3, /*stride=*/1, /*isZeroPadding=*/true);

	std::vector<std::vector<double>> parameters{{0.10, -0.20, 0.30, -0.40, 0.50, -0.60, 0.70, -0.80, 0.90}, {0.0}};
	layer.setParameters(parameters);

	const Matrix input{{1.0, 2.0, 3.0, 4.0}, {-1.0, 0.5, -0.5, 1.5}};
	const Matrix upstream{{0.25, -0.50, 0.75, -1.00}, {1.25, -1.50, 1.75, -2.00}};

	layer.forward(input);
	layer.backprop(input, upstream);
	const auto derivatives = layer.get_derivatives();

	const double eps = 1.0e-6;
	for (std::size_t index : {0UL, 4UL, 8UL, 9UL}) {
		auto perturbed = parameters;
		if (index < parameters[0].size()) {
			perturbed[0][index] += eps;
		} else {
			perturbed[1][index - parameters[0].size()] += eps;
		}
		layer.setParameters(perturbed);
		const double value_plus = weighted_mean_output(layer, input, upstream);

		if (index < parameters[0].size()) {
			perturbed[0][index] -= 2.0 * eps;
		} else {
			perturbed[1][index - parameters[0].size()] -= 2.0 * eps;
		}
		layer.setParameters(perturbed);
		const double value_minus = weighted_mean_output(layer, input, upstream);

		layer.setParameters(parameters);
		const double numeric = (value_plus - value_minus) / (2.0 * eps);
		assert(close(derivatives[index], numeric));
	}
}

auto exercise_sgd_updates() -> void
{
	static_assert(!std::is_abstract<dnn::SGD<double>>::value, "SGD must implement the Optimizer interface");

	dnn::SGD<double> optimizer(0.1, 0.01);
	RowVector weights{1.0, -2.0};
	const RowVector gradients{0.5, -1.0};
	optimizer.update(gradients, weights);
	assert(close(weights[0], 0.949, 1.0e-12));
	assert(close(weights[1], -1.898, 1.0e-12));

	ColumnMatrix matrix(2, 2);
	matrix(0, 0) = 1.0;
	matrix(1, 0) = -1.0;
	matrix(0, 1) = 2.0;
	matrix(1, 1) = -2.0;

	ColumnMatrix matrix_grad(2, 2);
	matrix_grad(0, 0) = 0.2;
	matrix_grad(1, 0) = -0.4;
	matrix_grad(0, 1) = 0.6;
	matrix_grad(1, 1) = -0.8;

	optimizer.update(matrix_grad, matrix);
	assert(close(matrix(0, 0), 0.979, 1.0e-12));
	assert(close(matrix(1, 0), -0.959, 1.0e-12));
	assert(close(matrix(0, 1), 1.938, 1.0e-12));
	assert(close(matrix(1, 1), -1.918, 1.0e-12));

	const auto json = optimizer.toJson();
	assert(json["type"].get<std::string>() == "SGD");
	assert(close(json["learningRate"].get<double>(), 0.1, 0.0));
	assert(close(json["decay"].get<double>(), 0.01, 0.0));
}

auto exercise_conv2d_network_artifact_roundtrip() -> void
{
	dnn::Network<double> network;
	network.addLayer(Conv2d(/*inputWidth=*/3, /*inputHeight=*/3, /*inputChannels=*/1, /*outputChannels=*/1,
							/*kernelWidth=*/2, /*kernelHeight=*/2));
	network.setOutput(dnn::RegressionMSE<double>());
	network.setOptimizer(dnn::RMSProp<double>(0.001, 1.0e-8, 0.9));

	const std::vector<std::vector<double>> parameters{{0.2, -0.1, 0.4, 0.3}, {0.05}};
	network.layers[0]->setParameters(parameters);

	const Matrix input{{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
					   {1.0, -1.0, 0.5, 2.0, -2.0, 1.5, 3.0, -3.0, 2.5}};
	const Matrix expected = network.predict(input);

	std::stringstream stream;
	network.save(stream);

	dnn::Network<double> loaded;
	loaded.load(stream);
	const Matrix actual = loaded.predict(input);

	assert(actual.rows() == expected.rows());
	assert(actual.columns() == expected.columns());
	for (std::size_t row = 0; row < actual.rows(); ++row) {
		for (std::size_t column = 0; column < actual.columns(); ++column) {
			assert(close(actual(row, column), expected(row, column), 0.0));
		}
	}

	const auto json = loaded.toJson();
	assert(json["0"]["type"].get<std::string>() == "Conv2d");
	assert(json["0"]["activation"].get<std::string>() == "Identity");
}

} // namespace

int main()
{
	exercise_conv2d_valid_backprop();
	exercise_conv2d_zero_padding_backprop();
	exercise_sgd_updates();
	exercise_conv2d_network_artifact_roundtrip();
	return 0;
}
