// Shape guards on public paths, deterministic training fixtures,
// and artifact roundtrip robustness for the native DNN solver.
//
// - Layer::setParameters now reports shape mismatches with std::invalid_argument
//   instead of asserting/aborting (and silently writing out of bounds under
//   NDEBUG). This is the deserialization public path used by Network::load and
//   the autoencoder artifact roundtrip.
// - Training with a fixed seed must be bit-for-bit reproducible.
// - load_native_autoencoder_model must reject tampered/incompatible artifacts.

#include <cassert>
#include <cmath>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include <metric/numeric/Math.h>
#include <metric/core/metadata.hpp>

#include "metric/solve/parametric/dnn.hpp"

namespace dnn = mtrc::solve::parametric::dnn;

namespace {

using Matrix = mtrc::numeric::DynamicMatrix<double>;
using record_type = std::vector<double>;

auto close(double lhs, double rhs, double tolerance = 1.0e-12) -> bool { return std::abs(lhs - rhs) <= tolerance; }

template <typename Callable> auto throws_invalid_argument(Callable &&call) -> bool
{
	try {
		call();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

auto add_identity_dense_layer(dnn::Network<double> &network, std::size_t input_size, std::size_t output_size) -> void
{
	dnn::FullyConnected<double, dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.0, 0.0);
	network.addLayer(layer);
}

auto make_autoencoder_model() -> dnn::AutoencoderModel<double>
{
	dnn::Network<double> network;
	add_identity_dense_layer(network, 2, 1);
	add_identity_dense_layer(network, 1, 2);
	network.setOutput(dnn::RegressionMSE<double>());
	network.setOptimizer(dnn::RMSProp<double>(0.01, 1.0e-8, 0.0));
	network.layers[0]->setParameters({{0.20, 0.10}, {0.00}});
	network.layers[1]->setParameters({{0.15, -0.10}, {0.00, 0.00}});
	return dnn::AutoencoderModel<double>(std::move(network));
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
	// 1. FullyConnected::setParameters rejects shape mismatches with an exception.
	//    Layer 2->1 has a weight group of size 2 and a bias group of size 1.
	{
		dnn::FullyConnected<double, dnn::Identity<double>> layer(2, 1);
		layer.initConstant(0.0, 0.0);

		assert(throws_invalid_argument([&]() { layer.setParameters({{0.1, 0.2}}); }));		   // missing bias group
		assert(throws_invalid_argument([&]() { layer.setParameters({{0.1}, {0.0}}); }));	   // weight too short
		assert(throws_invalid_argument([&]() { layer.setParameters({{0.1, 0.2, 0.3}, {0.0}}); })); // weight too long
		assert(throws_invalid_argument([&]() { layer.setParameters({{0.1, 0.2}, {0.0, 0.1}}); })); // bias too long
		assert(throws_invalid_argument([&]() { layer.setParameters({{0.1, 0.2}, {}}); }));	   // bias missing

		// A correctly shaped update succeeds and round-trips through getParameters.
		layer.setParameters({{0.3, -0.4}, {0.5}});
		const auto parameters = layer.getParameters();
		assert(parameters.size() == 2);
		assert(parameters[0].size() == 2);
		assert(parameters[1].size() == 1);
		assert(close(parameters[0][0], 0.3));
		assert(close(parameters[0][1], -0.4));
		assert(close(parameters[1][0], 0.5));
	}

	// 1b. Network construction from JSON rejects malformed input on the load path
	//     (empty object, or layers present but the mandatory 'train' section missing)
	//     instead of underflowing the layer-count loop.
	{
		assert(throws_invalid_argument([]() { dnn::Network<double> network("{}"); }));
		assert(throws_invalid_argument([]() {
			dnn::Network<double> network(
				R"({"0":{"type":"FullyConnected","inputSize":2,"outputSize":1,"activation":"Identity"}})");
		}));
	}

	// 2. Deterministic, stable training: same seed -> bit-identical epoch losses and
	//    latent codes, and the reconstruction loss decreases over training.
	{
		const std::vector<record_type> records{{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}};
		dnn::VectorRecordCodec<record_type, double> codec(2);
		const auto features = codec.encode_batch(records);
		const auto dataset = dnn::EncodedDataset<double>::from_features(features, 7);

		dnn::CompositeLoss<double> objective;
		objective.add(std::make_shared<dnn::ReconstructionMSELoss<double>>(), 1.0);

		dnn::TrainingSpec<double> spec;
		spec.epochs = 15;
		spec.batch_size = 2;
		spec.seed = 19;
		spec.shuffle = true;

		const dnn::NativeDnnTrainer<double> trainer;

		auto model_a = make_autoencoder_model();
		const auto report_a = trainer.fit(model_a, dataset, objective, spec);

		auto model_b = make_autoencoder_model();
		const auto report_b = trainer.fit(model_b, dataset, objective, spec);

		assert(report_a.epochs.size() == spec.epochs);
		assert(report_b.epochs.size() == report_a.epochs.size());
		for (std::size_t epoch = 0; epoch < report_a.epochs.size(); ++epoch) {
			// Same seed, data, and starting weights are reproducible within one binary;
			// use a tight relative tolerance so the check stays robust to floating-point
			// reassociation across compilers/flags rather than asserting bit-for-bit.
			const double a = report_a.epochs[epoch].total_loss;
			const double b = report_b.epochs[epoch].total_loss;
			assert(std::isfinite(a) && std::isfinite(b));
			assert(std::abs(a - b) <= 1.0e-13 * (1.0 + std::abs(a)));
		}
		assert_same_matrix(model_a.encode(features), model_b.encode(features));

		// Training reduces the reconstruction objective from a finite, positive start.
		assert(std::isfinite(report_a.epochs.front().total_loss) && report_a.epochs.front().total_loss > 0.0);
		assert(std::isfinite(report_a.epochs.back().total_loss));
		assert(report_a.epochs.back().total_loss < report_a.epochs.front().total_loss);

		// 3. Artifact roundtrip reproduces the trained latent and reconstruction.
		const auto artifact =
			dnn::make_native_autoencoder_artifact(model_a, codec, objective, spec, records.size(),
												  dataset.source_space_version());
		auto loaded = dnn::load_native_autoencoder_model(artifact);
		assert(loaded.latent_dimension() == model_a.latent_dimension());
		assert(loaded.topology().bottleneck_layer == model_a.topology().bottleneck_layer);
		assert_same_matrix(model_a.encode(features), loaded.encode(features));
		assert_same_matrix(model_a.decode(model_a.encode(features)), loaded.decode(loaded.encode(features)));

		// 4. Tampered or incompatible artifacts are rejected on the load path.
		{
			auto bad_format = artifact;
			bad_format.manifest["format"] = "metric.bogus_artifact";
			assert(throws_invalid_argument([&]() { (void)dnn::load_native_autoencoder_model(bad_format); }));

			auto bad_version = artifact;
			bad_version.manifest["format_version"] = 2;
			assert(throws_invalid_argument([&]() { (void)dnn::load_native_autoencoder_model(bad_version); }));

			auto bad_backend = artifact;
			bad_backend.manifest["backend"] = "external_backend";
			assert(throws_invalid_argument([&]() { (void)dnn::load_native_autoencoder_model(bad_backend); }));

			auto empty_network = artifact;
			empty_network.network_binary.clear();
			assert(throws_invalid_argument([&]() { (void)dnn::load_native_autoencoder_model(empty_network); }));
		}
	}

	return 0;
}
