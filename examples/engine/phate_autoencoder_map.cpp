#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <metric/distance.hpp>
#include <metric/engine.hpp>
#include <metric/utils/dnn.hpp>

namespace {

using record_type = std::vector<double>;

auto add_identity_dense_layer(metric::dnn::Network<double> &network, std::size_t input_size, std::size_t output_size)
	-> void
{
	metric::dnn::FullyConnected<double, metric::dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.0, 0.0);
	network.addLayer(layer);
}

auto make_autoencoder_network() -> metric::dnn::Network<double>
{
	metric::dnn::Network<double> network;
	add_identity_dense_layer(network, 2, 1);
	add_identity_dense_layer(network, 1, 2);
	network.setOutput(metric::dnn::RegressionMSE<double>());
	network.setOptimizer(metric::dnn::RMSProp<double>(0.001, 1.0e-8, 0.0));
	network.layers[0]->setParameters({{0.10, 0.08}, {0.00}});
	network.layers[1]->setParameters({{0.09, 0.07}, {0.00, 0.00}});
	return network;
}

auto reconstruction_mse(const std::vector<record_type> &lhs, const std::vector<record_type> &rhs) -> double
{
	assert(lhs.size() == rhs.size());
	double squared_error = 0.0;
	for (std::size_t row = 0; row < lhs.size(); ++row) {
		assert(lhs[row].size() == rhs[row].size());
		for (std::size_t column = 0; column < lhs[row].size(); ++column) {
			const auto delta = lhs[row][column] - rhs[row][column];
			squared_error += delta * delta;
		}
	}
	return squared_error / static_cast<double>(lhs.size());
}

auto find_epoch_term(const metric::dnn::EpochReport<double> &epoch, const std::string &name) -> double
{
	for (const auto &term : epoch.terms) {
		if (term.name == name) {
			return term.value;
		}
	}
	assert(false);
	return 0.0;
}

} // namespace

int main()
{
	const std::vector<record_type> records{{0.0, 0.0}, {0.5, 0.5}, {1.0, 1.0}, {1.5, 1.5}, {2.0, 2.0}};
	auto space = metric::make_space(records, metric::Euclidean<double>{});

	metric::mappings::PhateGeometrySpec<double> geometry;
	geometry.dimensions = 1;
	geometry.diffusion_steps = 2;
	geometry.kernel_scale = 1.0;

	metric::dnn::TrainingSpec<double> training;
	training.epochs = 160;
	training.batch_size = records.size();
	training.shuffle = false;
	training.seed = 23;
	training.gradient_clip_norm = 20.0;

	auto mapping = metric::mappings::native_phate_autoencoder(
		metric::dnn::AutoencoderModel<double>(make_autoencoder_network()), geometry, training, 0.05, 1.0);
	auto model = metric::mappings::fit(mapping, space);
	auto latent = metric::mappings::transform(model, space);
	auto restored = model.inverse_transform(latent);

	const auto &report = model.training_report();
	const auto initial_bottleneck = find_epoch_term(report.epochs.front(), "bottleneck_coordinate_mse");
	const auto final_bottleneck = find_epoch_term(report.epochs.back(), "bottleneck_coordinate_mse");
	assert(final_bottleneck < initial_bottleneck);
	assert(latent.inverse_supported);
	assert(latent.space.record(latent.space.id(0)).size() == geometry.dimensions);
	assert(latent.space.distance(latent.space.id(0), latent.space.id(1)) <
		   latent.space.distance(latent.space.id(0), latent.space.id(4)));

	const auto reconstruction_error = reconstruction_mse(records, restored);
	assert(std::isfinite(reconstruction_error));

	std::cout << "PHATE-AE bottleneck loss: " << initial_bottleneck << " -> " << final_bottleneck << "\n";
	std::cout << "PHATE-AE reconstruction MSE: " << reconstruction_error << "\n";

	return 0;
}
