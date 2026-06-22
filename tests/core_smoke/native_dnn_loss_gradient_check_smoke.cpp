// Track C9 audit: finite-difference gradient checks, loss-normalization checks,
// and autoencoder bottleneck/topology inference checks for the native DNN solver.
//
// These tests treat the DNN purely as a native solver component and validate the
// mathematics of the loss terms and their normalization contract (see Loss.h):
//   value    = sum_of_squares / N            (averaged over observation rows)
//   gradient = d(N * value) / d(activation)  (the *unnormalized* sum gradient)
// The averaging by N happens inside the layer backprop (FullyConnected divides by
// nobs), so the per-layer parameter derivatives must equal d(value)/d(param) for
// the *mean* composite loss. We verify both relationships against central finite
// differences.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <vector>

#include <metric/numeric/Math.h>

#include "metric/solve/parametric/dnn.hpp"

namespace dnn = mtrc::solve::parametric::dnn;

namespace {

using Matrix = mtrc::numeric::DynamicMatrix<double>;

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool { return std::abs(lhs - rhs) <= tolerance; }

// Relative+absolute closeness, robust for finite-difference comparisons.
auto fd_close(double analytic, double numeric, double tolerance = 1.0e-6) -> bool
{
	return std::abs(analytic - numeric) <= tolerance * (1.0 + std::abs(analytic));
}

auto make_batch(const Matrix &features, const std::vector<std::size_t> &id_indices) -> dnn::DnnBatch<double>
{
	dnn::DnnBatch<double> batch;
	batch.x = features;
	batch.ids.reserve(id_indices.size());
	for (const auto index : id_indices) {
		batch.ids.push_back(dnn::SampleId::from_index(index));
	}
	return batch;
}

auto squared_norm(const Matrix &matrix) -> double
{
	const Matrix copy(matrix);
	return mtrc::numeric::sqrNorm(copy);
}

auto add_identity_dense_layer(dnn::Network<double> &network, std::size_t input_size, std::size_t output_size) -> void
{
	dnn::FullyConnected<double, dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.0, 0.0);
	network.addLayer(layer);
}

} // namespace

int main()
{
	// 1. Per-term finite difference + normalization for ReconstructionMSELoss.
	//    Verifies value == ||P - X||^2 / N and gradient == N * d(value)/d(P).
	{
		const Matrix prediction{{0.3, -0.2, 0.5}, {1.1, 0.4, -0.7}};
		const Matrix target{{0.0, 0.1, 0.2}, {0.9, 0.5, -0.5}};
		const auto batch = make_batch(target, {0, 1});

		const dnn::ReconstructionMSELoss<double> loss;
		const auto evaluation = loss.evaluate(batch, std::vector<Matrix>{prediction});

		const double rows = static_cast<double>(prediction.rows());
		assert(close(evaluation.value, squared_norm(prediction - target) / rows));

		const double eps = 1.0e-6;
		for (std::size_t row = 0; row < prediction.rows(); ++row) {
			for (std::size_t column = 0; column < prediction.columns(); ++column) {
				Matrix plus(prediction);
				Matrix minus(prediction);
				plus(row, column) += eps;
				minus(row, column) -= eps;
				const double value_plus = loss.evaluate(batch, std::vector<Matrix>{plus}).value;
				const double value_minus = loss.evaluate(batch, std::vector<Matrix>{minus}).value;
				const double numeric_grad = rows * (value_plus - value_minus) / (2.0 * eps);
				assert(fd_close(evaluation.gradient(row, column), numeric_grad));
			}
		}
	}

	// 2. Per-term finite difference + normalization for BottleneckCoordinateMSELoss.
	//    Batch ids intentionally reorder the target table to exercise id->row lookup.
	{
		std::map<dnn::SampleId, std::vector<double>> targets;
		targets.emplace(dnn::SampleId::from_index(0), std::vector<double>{1.0, -1.0});
		targets.emplace(dnn::SampleId::from_index(2), std::vector<double>{0.5, 0.25});

		const dnn::BottleneckCoordinateMSELoss<double> loss(0, targets);
		const Matrix bottleneck{{0.4, -0.6}, {0.1, 0.9}};
		const auto batch = make_batch(Matrix(2, 2, 0.0), {2, 0}); // row0->id2, row1->id0

		const auto evaluation = loss.evaluate(batch, std::vector<Matrix>{bottleneck});

		const Matrix target_matrix{{0.5, 0.25}, {1.0, -1.0}};
		const double rows = static_cast<double>(bottleneck.rows());
		assert(close(evaluation.value, squared_norm(bottleneck - target_matrix) / rows));

		const double eps = 1.0e-6;
		for (std::size_t row = 0; row < bottleneck.rows(); ++row) {
			for (std::size_t column = 0; column < bottleneck.columns(); ++column) {
				Matrix plus(bottleneck);
				Matrix minus(bottleneck);
				plus(row, column) += eps;
				minus(row, column) -= eps;
				const double value_plus = loss.evaluate(batch, std::vector<Matrix>{plus}).value;
				const double value_minus = loss.evaluate(batch, std::vector<Matrix>{minus}).value;
				const double numeric_grad = rows * (value_plus - value_minus) / (2.0 * eps);
				assert(fd_close(evaluation.gradient(row, column), numeric_grad));
			}
		}
	}

	// 3. End-to-end parameter finite difference through CompositeLoss + backprop.
	//    Validates that the per-layer derivatives computed by backprop equal the
	//    central-difference gradient of the *mean* composite loss w.r.t. every
	//    weight and bias. This jointly exercises the loss math, the backprop chain,
	//    and the 1/nobs normalization done inside the layers.
	{
		dnn::Network<double> network;
		add_identity_dense_layer(network, 2, 1);
		add_identity_dense_layer(network, 1, 2);
		network.setOutput(dnn::RegressionMSE<double>());
		network.setOptimizer(dnn::RMSProp<double>(0.001, 1.0e-8, 0.0));
		network.layers[0]->setParameters({{0.25, -0.15}, {0.05}});
		network.layers[1]->setParameters({{0.30, -0.40}, {0.02, -0.03}});

		const Matrix samples{{0.0, 1.0}, {1.0, -1.0}, {2.0, 0.5}};
		const auto batch = make_batch(samples, {0, 1, 2});

		std::map<dnn::SampleId, std::vector<double>> bottleneck_targets;
		bottleneck_targets.emplace(dnn::SampleId::from_index(0), std::vector<double>{-0.5});
		bottleneck_targets.emplace(dnn::SampleId::from_index(1), std::vector<double>{0.2});
		bottleneck_targets.emplace(dnn::SampleId::from_index(2), std::vector<double>{0.9});

		dnn::CompositeLoss<double> objective;
		objective.add(std::make_shared<dnn::ReconstructionMSELoss<double>>(), 0.7);
		objective.add(std::make_shared<dnn::BottleneckCoordinateMSELoss<double>>(0, bottleneck_targets), 1.3);

		const auto total_loss = [&]() -> double {
			std::vector<Matrix> activations;
			network.forward_all(batch.x, &activations);
			return objective.evaluate(batch, activations).total_value;
		};

		// Analytic derivatives at the unperturbed parameters.
		std::vector<Matrix> activations;
		network.forward_all(batch.x, &activations);
		const auto evaluation = objective.evaluate(batch, activations);
		assert(evaluation.total_value > 0.0);
		network.backprop_from_layer_gradients(batch.x, evaluation.gradients_by_layer);

		const double eps = 1.0e-6;
		double max_abs_derivative = 0.0;
		for (std::size_t layer = 0; layer < network.layers.size(); ++layer) {
			const auto base_parameters = network.layers[layer]->getParameters();
			const auto derivatives = network.layers[layer]->get_derivatives();

			const std::size_t weight_count = base_parameters[0].size();
			assert(derivatives.size() == weight_count + base_parameters[1].size());

			for (std::size_t group = 0; group < base_parameters.size(); ++group) {
				for (std::size_t index = 0; index < base_parameters[group].size(); ++index) {
					auto perturbed = base_parameters;
					perturbed[group][index] += eps;
					network.layers[layer]->setParameters(perturbed);
					const double value_plus = total_loss();

					perturbed[group][index] -= 2.0 * eps;
					network.layers[layer]->setParameters(perturbed);
					const double value_minus = total_loss();

					network.layers[layer]->setParameters(base_parameters); // restore

					assert(std::isfinite(value_plus) && std::isfinite(value_minus));
					const double numeric_grad = (value_plus - value_minus) / (2.0 * eps);
					const std::size_t flat_index = group == 0 ? index : weight_count + index;
					max_abs_derivative = std::max(max_abs_derivative, std::abs(derivatives[flat_index]));
					assert(fd_close(derivatives[flat_index], numeric_grad));
				}
			}
		}
		// Guard against a vacuous pass: the fixture must exercise non-trivial gradients.
		assert(max_abs_derivative > 1.0e-3);
	}

	// 4. Bottleneck / topology inference is positional and stable for 2..6 layers.
	{
		struct Expected {
			std::size_t layer_count;
			std::size_t bottleneck;
			std::vector<std::size_t> encoder;
			std::vector<std::size_t> decoder;
		};
		const std::vector<Expected> cases{
			{2, 0, {0}, {1}},
			{3, 1, {0, 1}, {2}},
			{4, 1, {0, 1}, {2, 3}},
			{5, 2, {0, 1, 2}, {3, 4}},
			{6, 2, {0, 1, 2}, {3, 4, 5}},
		};
		for (const auto &expected : cases) {
			const auto topology = dnn::infer_autoencoder_topology(expected.layer_count);
			assert(topology.input_layer == 0);
			assert(topology.bottleneck_layer == expected.bottleneck);
			assert(topology.output_layer == expected.layer_count - 1);
			assert(topology.encoder_layers == expected.encoder);
			assert(topology.decoder_layers == expected.decoder);
		}

		bool rejected = false;
		try {
			(void)dnn::infer_autoencoder_topology(1);
		} catch (const std::invalid_argument &) {
			rejected = true;
		}
		assert(rejected);
	}

	// 5. latent_dimension() and encode() agree with the inferred bottleneck width,
	//    even for an asymmetric width profile, since inference is positional.
	{
		dnn::Network<double> network;
		add_identity_dense_layer(network, 2, 3);
		add_identity_dense_layer(network, 3, 1); // narrowest, but inference is positional
		add_identity_dense_layer(network, 1, 3);
		add_identity_dense_layer(network, 3, 2);
		network.setOutput(dnn::RegressionMSE<double>());
		network.setOptimizer(dnn::RMSProp<double>(0.001, 1.0e-8, 0.0));

		dnn::AutoencoderModel<double> model(std::move(network));
		assert(model.topology().bottleneck_layer == 1);
		assert(model.latent_dimension() == 1);

		const Matrix samples{{0.0, 1.0}, {1.0, -1.0}, {2.0, 0.5}};
		const auto latent = model.encode(samples);
		assert(latent.rows() == samples.rows());
		assert(latent.columns() == 1);

		const auto decoded = model.decode(latent);
		assert(decoded.rows() == samples.rows());
		assert(decoded.columns() == 2);
	}

	return 0;
}
