#include <cassert>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include <blaze/Math.h>

#include "metric/utils/dnn.hpp"

namespace {

using Matrix = blaze::DynamicMatrix<double>;
using record_type = std::vector<double>;

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool
{
	return std::abs(lhs - rhs) <= tolerance;
}

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
	network.layers[0]->setParameters({{0.20, 0.10}, {0.00}});
	network.layers[1]->setParameters({{0.15, -0.10}, {0.00, 0.00}});
	return network;
}

auto artifact_objective() -> metric::dnn::CompositeLoss<double>
{
	metric::dnn::BottleneckCoordinateMSELoss<double>::target_table_type targets;
	targets.emplace(metric::dnn::SampleId::from_index(0), std::vector<double>{-0.75});
	targets.emplace(metric::dnn::SampleId::from_index(1), std::vector<double>{-0.25});
	targets.emplace(metric::dnn::SampleId::from_index(2), std::vector<double>{0.25});
	targets.emplace(metric::dnn::SampleId::from_index(3), std::vector<double>{0.75});

	metric::dnn::CompositeLoss<double> objective;
	objective.add(std::make_shared<metric::dnn::ReconstructionMSELoss<double>>(), 0.25);
	objective.add(std::make_shared<metric::dnn::BottleneckCoordinateMSELoss<double>>(0, std::move(targets)), 1.5);
	return objective;
}

auto assert_same_matrix(const Matrix &lhs, const Matrix &rhs) -> void
{
	assert(lhs.rows() == rhs.rows());
	assert(lhs.columns() == rhs.columns());
	for (std::size_t row = 0; row < lhs.rows(); ++row) {
		for (std::size_t column = 0; column < lhs.columns(); ++column) {
			assert(close(lhs(row, column), rhs(row, column)));
		}
	}
}

} // namespace

int main()
{
	const std::vector<record_type> records{{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}};
	metric::dnn::VectorRecordCodec<record_type, double> codec(2);
	const auto features = codec.encode_batch(records);
	auto dataset = metric::dnn::EncodedDataset<double>::from_features(features, 42);

	auto objective = artifact_objective();
	metric::dnn::TrainingSpec<double> spec;
	spec.epochs = 5;
	spec.batch_size = 2;
	spec.seed = 19;
	spec.shuffle = true;
	spec.gradient_clip_norm = 5.0;

	metric::dnn::AutoencoderModel<double> model(make_autoencoder_network());
	const metric::dnn::NativeDnnTrainer<double> trainer;
	trainer.fit(model, dataset, objective, spec);

	const auto artifact =
		metric::dnn::make_native_autoencoder_artifact(model, codec, objective, spec, records.size(),
													  dataset.source_space_version());
	assert(artifact.manifest.at("format") == "metric.native_autoencoder_artifact");
	assert(artifact.manifest.at("format_version") == 1);
	assert(artifact.manifest.at("backend") == "native_dnn");
	assert(artifact.manifest.at("scalar_type") == "double");
	assert(artifact.manifest.at("network").at("serialization") == "cereal_binary");
	assert(artifact.manifest.at("network").at("byte_count").get<std::size_t>() == artifact.network_cereal.size());
	assert(artifact.manifest.at("network_json").at("train").at("optimizer").at("type") == "RMSProp");
	assert(artifact.manifest.at("autoencoder_topology").at("bottleneck_layer") == model.topology().bottleneck_layer);
	assert(artifact.manifest.at("codec").at("type") == "VectorRecordCodec");
	assert(artifact.manifest.at("codec").at("feature_count") == codec.feature_count());
	assert(artifact.manifest.at("loss").at("terms").size() == 2);
	assert(artifact.manifest.at("loss").at("terms").at(0).at("weight") == 0.25);
	assert(artifact.manifest.at("loss").at("terms").at(1).at("term").at("target_count") == records.size());
	assert(artifact.manifest.at("training_spec").at("epochs") == spec.epochs);
	assert(artifact.manifest.at("source").at("record_count") == records.size());
	assert(artifact.manifest.at("source").at("feature_count") == codec.feature_count());
	assert(artifact.manifest.at("source").at("space_version") == dataset.source_space_version());
	assert(artifact.diagnostics.at("epoch_count") == spec.epochs);
	assert(artifact.diagnostics.at("epochs").size() == spec.epochs);

	auto loaded = metric::dnn::load_native_autoencoder_model(artifact);
	assert(loaded.backend_name() == model.backend_name());
	assert(loaded.latent_dimension() == model.latent_dimension());
	assert(loaded.topology().bottleneck_layer == model.topology().bottleneck_layer);
	assert_same_matrix(model.encode(features), loaded.encode(features));
	assert_same_matrix(model.decode(model.encode(features)), loaded.decode(loaded.encode(features)));

	return 0;
}
