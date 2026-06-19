#include <cassert>
#include <cmath>
#include <string>
#include <vector>

#include <blaze/Math.h>

#include "metric/distance.hpp"
#include "metric/engine.hpp"
#include "metric/utils/dnn.hpp"

namespace {

using Matrix = blaze::DynamicMatrix<double>;
using record_type = std::vector<double>;

auto is_finite(double value) -> bool
{
	return std::isfinite(value);
}

auto add_identity_dense_layer(metric::dnn::Network<double> &network, std::size_t input_size, std::size_t output_size)
	-> void
{
	metric::dnn::FullyConnected<double, metric::dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.0, 0.0);
	network.addLayer(layer);
}

auto make_autoencoder_network(double learning_rate = 0.001) -> metric::dnn::Network<double>
{
	metric::dnn::Network<double> network;
	add_identity_dense_layer(network, 2, 1);
	add_identity_dense_layer(network, 1, 2);
	network.setOutput(metric::dnn::RegressionMSE<double>());
	network.setOptimizer(metric::dnn::RMSProp<double>(learning_rate, 1.0e-8, 0.0));
	network.layers[0]->setParameters({{0.10, 0.08}, {0.00}});
	network.layers[1]->setParameters({{0.09, 0.07}, {0.00, 0.00}});
	return network;
}

auto features_from_records(const std::vector<record_type> &records) -> Matrix
{
	metric::dnn::VectorRecordCodec<record_type, double> codec(records.front().size());
	return codec.encode_batch(records);
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

auto initial_reconstruction(const std::vector<record_type> &records) -> std::vector<record_type>
{
	metric::dnn::AutoencoderModel<double> model(make_autoencoder_network());
	const auto features = features_from_records(records);
	const auto reconstructed = model.decode(model.encode(features));
	metric::dnn::VectorRecordCodec<record_type, double> codec(records.front().size());
	return codec.decode_batch(reconstructed);
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

	const auto targets = metric::mappings::phate_geometry_targets<decltype(space), double>(space, geometry);
	assert(targets.coordinates.size() == space.size());
	assert(targets.dimensions == 1);
	assert(targets.diffusion_steps == geometry.diffusion_steps);
	assert(targets.method == "diffusion_potential_anchor_coordinates");
	for (std::size_t index = 0; index < space.size(); ++index) {
		const auto found = targets.coordinates.find(space.id(index));
		assert(found != targets.coordinates.end());
		assert(found->second.size() == 1);
		assert(is_finite(found->second[0]));
	}
	assert(targets.coordinates.at(space.id(0))[0] < targets.coordinates.at(space.id(4))[0]);

	metric::dnn::TrainingSpec<double> training;
	training.epochs = 160;
	training.batch_size = records.size();
	training.shuffle = false;
	training.seed = 23;
	training.gradient_clip_norm = 20.0;

	auto mapping = metric::mappings::native_phate_autoencoder(
		metric::dnn::AutoencoderModel<double>(make_autoencoder_network()), geometry, training, 0.05, 1.0);
	static_assert(metric::Mapping_v<decltype(mapping), decltype(space)>);

	const auto initial_restored = initial_reconstruction(records);
	const auto initial_reconstruction_error = reconstruction_mse(records, initial_restored);
	auto model = metric::mappings::fit(mapping, space);
	static_assert(metric::MappingModel_v<decltype(model), decltype(space)>);

	const auto &report = model.training_report();
	assert(report.epochs.size() == training.epochs);
	assert(report.epochs.front().terms.size() == 2);
	const auto initial_bottleneck = find_epoch_term(report.epochs.front(), "bottleneck_coordinate_mse");
	const auto final_bottleneck = find_epoch_term(report.epochs.back(), "bottleneck_coordinate_mse");
	assert(final_bottleneck < initial_bottleneck);

	const auto reduced = metric::mappings::transform(model, space);
	assert(reduced.mapping == "native_autoencoder");
	assert(reduced.strategy == "native_dnn");
	assert(reduced.inverse_supported);
	assert(reduced.space.size() == space.size());
	assert(reduced.space.record(reduced.space.id(0)).size() == geometry.dimensions);
	assert(reduced.space.distance(reduced.space.id(0), reduced.space.id(1)) <
		   reduced.space.distance(reduced.space.id(0), reduced.space.id(4)));

	const auto restored = model.inverse_transform(reduced);
	const auto final_reconstruction_error = reconstruction_mse(records, restored);
	assert(final_reconstruction_error < initial_reconstruction_error);

	return 0;
}
