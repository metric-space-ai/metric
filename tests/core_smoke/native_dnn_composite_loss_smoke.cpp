#include <cassert>
#include <cmath>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/numeric/Math.h>

#include "metric/solve/parametric/dnn.hpp"

namespace {

using Matrix = mtrc::numeric::DynamicMatrix<double>;
using Parameters = std::vector<std::vector<double>>;

auto close(double lhs, double rhs, double tolerance = 1.0e-8) -> bool { return std::abs(lhs - rhs) <= tolerance; }

auto add_identity_dense_layer(mtrc::solve::parametric::dnn::Network<double> &network, std::size_t input_size, std::size_t output_size)
	-> void
{
	mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.0, 0.0);
	network.addLayer(layer);
}

auto make_autoencoder_network(double learning_rate = 0.01) -> mtrc::solve::parametric::dnn::Network<double>
{
	mtrc::solve::parametric::dnn::Network<double> network;
	add_identity_dense_layer(network, 2, 1);
	add_identity_dense_layer(network, 1, 2);
	network.setOutput(mtrc::solve::parametric::dnn::RegressionMSE<double>());
	network.setOptimizer(mtrc::solve::parametric::dnn::RMSProp<double>(learning_rate, 1.0e-8, 0.0));
	network.layers[0]->setParameters({{0.25, -0.15}, {0.05}});
	network.layers[1]->setParameters({{0.30, -0.40}, {0.02, -0.03}});
	return network;
}

auto make_batch(const Matrix &features) -> mtrc::solve::parametric::dnn::DnnBatch<double>
{
	mtrc::solve::parametric::dnn::DnnBatch<double> batch;
	batch.x = features;
	batch.ids.reserve(features.rows());
	for (std::size_t index = 0; index < features.rows(); ++index) {
		batch.ids.push_back(mtrc::solve::parametric::dnn::SampleId::from_index(index));
	}
	return batch;
}

auto evaluate(mtrc::solve::parametric::dnn::Network<double> &network, const mtrc::solve::parametric::dnn::CompositeLoss<double> &objective,
			  const mtrc::solve::parametric::dnn::DnnBatch<double> &batch) -> mtrc::solve::parametric::dnn::CompositeLossEvaluation<double>
{
	std::vector<Matrix> activations;
	network.forward_all(batch.x, &activations);
	return objective.evaluate(batch, activations);
}

auto train_step(mtrc::solve::parametric::dnn::Network<double> &network, const mtrc::solve::parametric::dnn::CompositeLoss<double> &objective,
				const mtrc::solve::parametric::dnn::DnnBatch<double> &batch) -> mtrc::solve::parametric::dnn::CompositeLossEvaluation<double>
{
	auto evaluation = evaluate(network, objective, batch);
	network.backprop_from_layer_gradients(batch.x, evaluation.gradients_by_layer);
	network.apply_optimizer();
	return evaluation;
}

auto assert_parameters_close(const Parameters &lhs, const Parameters &rhs, double tolerance = 1.0e-8) -> void
{
	assert(lhs.size() == rhs.size());
	for (std::size_t group = 0; group < lhs.size(); ++group) {
		assert(lhs[group].size() == rhs[group].size());
		for (std::size_t index = 0; index < lhs[group].size(); ++index) {
			assert(close(lhs[group][index], rhs[group][index], tolerance));
		}
	}
}

auto assert_networks_close(const mtrc::solve::parametric::dnn::Network<double> &lhs, const mtrc::solve::parametric::dnn::Network<double> &rhs) -> void
{
	assert(lhs.layers.size() == rhs.layers.size());
	for (std::size_t layer = 0; layer < lhs.layers.size(); ++layer) {
		assert_parameters_close(lhs.layers[layer]->getParameters(), rhs.layers[layer]->getParameters());
	}
}

auto find_term_value(const mtrc::solve::parametric::dnn::CompositeLossEvaluation<double> &evaluation, const std::string &name) -> double
{
	for (const auto &term : evaluation.terms) {
		if (term.name == name) {
			return term.value;
		}
	}
	assert(false);
	return 0.0;
}

auto assert_same_shape(const Matrix &matrix, std::size_t rows, std::size_t columns) -> void
{
	assert(matrix.rows() == rows);
	assert(matrix.columns() == columns);
}

template <typename Callable> auto assert_invalid_argument(Callable &&call) -> void
{
	bool rejected = false;
	try {
		call();
	} catch (const std::invalid_argument &) {
		rejected = true;
	}
	assert(rejected);
}

} // namespace

int main()
{
	const Matrix samples{{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}};
	const auto batch = make_batch(samples);

	{
		mtrc::solve::parametric::dnn::RegressionMSE<double> mse;
		const Matrix prediction{{0.0, 0.5}, {1.0, 2.0}};
		const Matrix target{{0.0, 0.0}, {2.0, 2.0}};
		mse.evaluate(prediction, target);

		mtrc::solve::parametric::dnn::DnnBatch<double> mse_batch;
		mse_batch.x = target;
		mse_batch.ids = {mtrc::solve::parametric::dnn::SampleId::from_index(0), mtrc::solve::parametric::dnn::SampleId::from_index(1)};
		const std::vector<Matrix> activations{prediction};
		const mtrc::solve::parametric::dnn::ReconstructionMSELoss<double> loss;
		const auto evaluation = loss.evaluate(mse_batch, activations);
		assert(close(evaluation.value, mse.loss()));
		assert(close(evaluation.gradient(0, 0), mse.backprop_data()(0, 0)));
		assert(close(evaluation.gradient(0, 1), mse.backprop_data()(0, 1)));
		assert(close(evaluation.gradient(1, 0), mse.backprop_data()(1, 0)));
		assert(close(evaluation.gradient(1, 1), mse.backprop_data()(1, 1)));
	}

	{
		std::map<mtrc::solve::parametric::dnn::SampleId, std::vector<double>> bottleneck_targets;
		bottleneck_targets.emplace(mtrc::solve::parametric::dnn::SampleId::from_index(0), std::vector<double>{1.0, 1.0});
		bottleneck_targets.emplace(mtrc::solve::parametric::dnn::SampleId::from_index(2), std::vector<double>{5.0, 7.0});

		mtrc::solve::parametric::dnn::DnnBatch<double> coordinate_batch;
		coordinate_batch.x = Matrix(2, 2, 0.0);
		coordinate_batch.ids = {mtrc::solve::parametric::dnn::SampleId::from_index(2), mtrc::solve::parametric::dnn::SampleId::from_index(0)};

		const mtrc::solve::parametric::dnn::BottleneckCoordinateMSELoss<double> loss(0, bottleneck_targets);
		const std::vector<Matrix> activations{Matrix{{6.0, 4.0}, {2.0, 3.0}}};
		const auto evaluation = loss.evaluate(coordinate_batch, activations);

		assert(close(evaluation.value, 7.5));
		assert_same_shape(evaluation.gradient, 2, 2);
		assert(close(evaluation.gradient(0, 0), 2.0));
		assert(close(evaluation.gradient(0, 1), -6.0));
		assert(close(evaluation.gradient(1, 0), 2.0));
		assert(close(evaluation.gradient(1, 1), 4.0));

		mtrc::solve::parametric::dnn::DnnBatch<double> missing_target_batch = coordinate_batch;
		missing_target_batch.ids = {mtrc::solve::parametric::dnn::SampleId::from_index(1), mtrc::solve::parametric::dnn::SampleId::from_index(0)};
		assert_invalid_argument(
			[&loss, &missing_target_batch, &activations]() { (void)loss.evaluate(missing_target_batch, activations); });

		std::map<mtrc::solve::parametric::dnn::SampleId, std::vector<double>> bad_width_targets;
		bad_width_targets.emplace(mtrc::solve::parametric::dnn::SampleId::from_index(0), std::vector<double>{1.0});
		bad_width_targets.emplace(mtrc::solve::parametric::dnn::SampleId::from_index(2), std::vector<double>{5.0, 7.0});
		const mtrc::solve::parametric::dnn::BottleneckCoordinateMSELoss<double> bad_width_loss(0, std::move(bad_width_targets));
		assert_invalid_argument([&bad_width_loss, &coordinate_batch, &activations]() {
			(void)bad_width_loss.evaluate(coordinate_batch, activations);
		});
	}

	{
		auto legacy = make_autoencoder_network();
		auto composite = make_autoencoder_network();

		mtrc::solve::parametric::dnn::CompositeLoss<double> objective;
		objective.add(std::make_shared<mtrc::solve::parametric::dnn::ReconstructionMSELoss<double>>());

		const auto before = evaluate(composite, objective, batch);
		assert(before.terms.size() == 1);
		assert(before.terms[0].name == "reconstruction_mse");
		assert(close(before.total_value, before.terms[0].weighted_value));

		legacy.fit(samples, samples, 3, 1, 123);
		(void)train_step(composite, objective, batch);
		assert_networks_close(legacy, composite);
	}

	{
		auto supervised = make_autoencoder_network(0.002);
		std::map<mtrc::solve::parametric::dnn::SampleId, std::vector<double>> bottleneck_targets;
		bottleneck_targets.emplace(mtrc::solve::parametric::dnn::SampleId::from_index(0), std::vector<double>{0.0});
		bottleneck_targets.emplace(mtrc::solve::parametric::dnn::SampleId::from_index(1), std::vector<double>{1.0});
		bottleneck_targets.emplace(mtrc::solve::parametric::dnn::SampleId::from_index(2), std::vector<double>{2.0});

		mtrc::solve::parametric::dnn::CompositeLoss<double> objective;
		objective.add(std::make_shared<mtrc::solve::parametric::dnn::ReconstructionMSELoss<double>>(), 0.01);
		objective.add(std::make_shared<mtrc::solve::parametric::dnn::BottleneckCoordinateMSELoss<double>>(0, bottleneck_targets), 1.0);

		const auto initial = evaluate(supervised, objective, batch);
		assert(initial.terms.size() == 2);
		assert(initial.total_value > 0.0);
		const auto initial_bottleneck = find_term_value(initial, "bottleneck_coordinate_mse");

		for (std::size_t step = 0; step < 120; ++step) {
			(void)train_step(supervised, objective, batch);
		}

		const auto final = evaluate(supervised, objective, batch);
		const auto final_bottleneck = find_term_value(final, "bottleneck_coordinate_mse");
		assert(final_bottleneck < initial_bottleneck);
	}

	return 0;
}
