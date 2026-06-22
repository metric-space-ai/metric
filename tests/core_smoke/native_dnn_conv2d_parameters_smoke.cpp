// Conv2d parameter save/load smoke test.
//
// Regression coverage for the Conv2d::getParameters out-of-bounds write: the
// returned weight/bias groups used to be default-constructed (empty) vectors
// that were then std::copy-ed into without a prior resize -- a heap overflow on
// exactly the path Network::save takes (`layers[i]->getParameters()`). This test
// pins the fix by asserting the returned groups are correctly sized and that a
// getParameters/setParameters roundtrip preserves the full layer function. It
// uses the same Conv2d::getParameters/setParameters calls Network::save/load use,
// so it covers the save path even though Conv2d itself is not JSON-serializable.
//
// Framing: Conv2d stays a native solver layer under mtrc::solve::parametric::dnn;
// this is solver-component coverage, not an ML product surface.

#include <cassert>
#include <cmath>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <vector>

#include <metric/numeric/Math.h>

#include "metric/solve/parametric/dnn.hpp"

namespace {

using Matrix = mtrc::numeric::DynamicMatrix<double>;
using Conv2d = mtrc::solve::parametric::dnn::Conv2d<double, mtrc::solve::parametric::dnn::Identity<double>>;

auto close(double lhs, double rhs, double tolerance) -> bool { return std::abs(lhs - rhs) <= tolerance; }

// Deterministic, non-constant input: nobs observations, each row carries the
// inputChannels * inputWidth * inputHeight features the layer expects.
auto make_input(std::size_t nobs, std::size_t features) -> Matrix
{
	Matrix input(nobs, features);
	for (std::size_t row = 0; row < nobs; ++row) {
		for (std::size_t column = 0; column < features; ++column) {
			input(row, column) = std::sin(0.37 * static_cast<double>(row + 1) + 0.11 * static_cast<double>(column + 1));
		}
	}
	return input;
}

auto outputs_differ(const Matrix &lhs, const Matrix &rhs) -> bool
{
	assert(lhs.rows() == rhs.rows());
	assert(lhs.columns() == rhs.columns());
	for (std::size_t row = 0; row < lhs.rows(); ++row) {
		for (std::size_t column = 0; column < lhs.columns(); ++column) {
			if (!close(lhs(row, column), rhs(row, column), 1.0e-9)) {
				return true;
			}
		}
	}
	return false;
}

auto outputs_equal(const Matrix &lhs, const Matrix &rhs) -> void
{
	assert(lhs.rows() == rhs.rows());
	assert(lhs.columns() == rhs.columns());
	for (std::size_t row = 0; row < lhs.rows(); ++row) {
		for (std::size_t column = 0; column < lhs.columns(); ++column) {
			// Identical parameters drive the identical deterministic forward pass,
			// so the restored output must match bit-for-bit (no near-tie here).
			assert(close(lhs(row, column), rhs(row, column), 0.0));
		}
	}
}

} // namespace

int main()
{
	// 3x3 single-channel input, two output channels, 2x2 kernel, valid convolution.
	const std::size_t input_width = 3;
	const std::size_t input_height = 3;
	const std::size_t input_channels = 1;
	const std::size_t output_channels = 2;
	const std::size_t kernel_width = 2;
	const std::size_t kernel_height = 2;

	const std::size_t kernel_size = input_channels * output_channels * kernel_width * kernel_height; // 8
	const std::size_t bias_size = output_channels;													// 2

	Conv2d layer(input_width, input_height, input_channels, output_channels, kernel_width, kernel_height);

	std::mt19937 rng(12345);
	layer.init(0.0, 0.5, rng);

	// --- getParameters returns correctly-sized groups (the out-of-bounds fix). ---
	const auto parameters = layer.getParameters();
	assert(parameters.size() == 2);
	assert(parameters[0].size() == kernel_size);
	assert(parameters[1].size() == bias_size);
	for (double value : parameters[0]) {
		assert(std::isfinite(value));
	}
	for (double value : parameters[1]) {
		assert(std::isfinite(value));
	}

	// The saved kernels must be non-trivial so the roundtrip below is not vacuous.
	double kernel_magnitude = 0.0;
	for (double value : parameters[0]) {
		kernel_magnitude += std::abs(value);
	}
	assert(kernel_magnitude > 1.0e-6);

	// --- The forward map is exactly restored across a getParameters/setParameters
	//     roundtrip (meaningful state preservation, not a finiteness check). ---
	const std::size_t nobs = 4;
	const auto input = make_input(nobs, input_channels * input_width * input_height);
	assert(layer.getOutputSize() == output_channels * 2 * 2);

	layer.forward(input);
	const Matrix saved_output = layer.output();

	// Perturb with a different random initialization; the function must change so
	// the restore step proves something.
	std::mt19937 other_rng(987654321);
	layer.init(0.0, 0.5, other_rng);
	layer.forward(input);
	const Matrix perturbed_output = layer.output();
	assert(outputs_differ(saved_output, perturbed_output));

	// Restore the saved parameters: read-back values and the forward output must match.
	layer.setParameters(parameters);
	const auto restored = layer.getParameters();
	assert(restored[0].size() == kernel_size);
	assert(restored[1].size() == bias_size);
	for (std::size_t index = 0; index < kernel_size; ++index) {
		assert(close(restored[0][index], parameters[0][index], 0.0));
	}
	for (std::size_t index = 0; index < bias_size; ++index) {
		assert(close(restored[1][index], parameters[1][index], 0.0));
	}
	layer.forward(input);
	outputs_equal(layer.output(), saved_output);

	// --- A modified, correctly shaped parameter set round-trips through the
	//     write path and changes the forward output predictably. ---
	{
		auto modified = parameters;
		for (double &value : modified[0]) {
			value += 0.25;
		}
		for (double &value : modified[1]) {
			value -= 0.10;
		}
		layer.setParameters(modified);
		const auto read_back = layer.getParameters();
		for (std::size_t index = 0; index < kernel_size; ++index) {
			assert(close(read_back[0][index], modified[0][index], 0.0));
		}
		for (std::size_t index = 0; index < bias_size; ++index) {
			assert(close(read_back[1][index], modified[1][index], 0.0));
		}
		layer.forward(input);
		assert(outputs_differ(layer.output(), saved_output));
		// Restore the canonical parameters for the guard checks below.
		layer.setParameters(parameters);
	}

	// --- Shape guards reject malformed parameter sets on the public load path. ---
	{
		bool rejected = false;
		try {
			layer.setParameters({parameters[0]}); // wrong group count
		} catch (const std::invalid_argument &) {
			rejected = true;
		}
		assert(rejected);
	}
	{
		auto bad = parameters;
		bad[0].push_back(0.0); // wrong kernel size
		bool rejected = false;
		try {
			layer.setParameters(bad);
		} catch (const std::invalid_argument &) {
			rejected = true;
		}
		assert(rejected);
	}
	{
		auto bad = parameters;
		bad[1].pop_back(); // wrong bias size
		bool rejected = false;
		try {
			layer.setParameters(bad);
		} catch (const std::invalid_argument &) {
			rejected = true;
		}
		assert(rejected);
	}

	// The canonical parameters survive the rejected updates intact.
	layer.forward(input);
	outputs_equal(layer.output(), saved_output);

	return 0;
}
