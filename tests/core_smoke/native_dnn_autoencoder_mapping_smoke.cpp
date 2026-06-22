#include <cassert>
#include <cmath>
#include <memory>
#include <type_traits>
#include <vector>

#include <metric/numeric/Math.h>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"
#include "metric/solve/parametric/dnn.hpp"

namespace {

using Matrix = mtrc::numeric::DynamicMatrix<double>;

auto is_finite(double value) -> bool { return std::isfinite(value); }

auto add_identity_dense_layer(mtrc::solve::parametric::dnn::Network<double> &network, std::size_t input_size, std::size_t output_size)
	-> void
{
	mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.0, 0.0);
	network.addLayer(layer);
}

auto make_autoencoder_network(double learning_rate = 0.002) -> mtrc::solve::parametric::dnn::Network<double>
{
	mtrc::solve::parametric::dnn::Network<double> network;
	add_identity_dense_layer(network, 2, 1);
	add_identity_dense_layer(network, 1, 2);
	network.setOutput(mtrc::solve::parametric::dnn::RegressionMSE<double>());
	network.setOptimizer(mtrc::solve::parametric::dnn::RMSProp<double>(learning_rate, 1.0e-8, 0.0));
	network.layers[0]->setParameters({{0.20, 0.10}, {0.00}});
	network.layers[1]->setParameters({{0.15, -0.10}, {0.00, 0.00}});
	return network;
}

auto reconstruction_objective() -> mtrc::solve::parametric::dnn::CompositeLoss<double>
{
	mtrc::solve::parametric::dnn::CompositeLoss<double> objective;
	objective.add(std::make_shared<mtrc::solve::parametric::dnn::ReconstructionMSELoss<double>>());
	return objective;
}

} // namespace

int main()
{
	const Matrix features{{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}};

	{
		mtrc::solve::parametric::dnn::EncodedDataset<double> dataset = mtrc::solve::parametric::dnn::EncodedDataset<double>::from_features(features, 7);
		auto objective = reconstruction_objective();
		mtrc::solve::parametric::dnn::TrainingSpec<double> spec;
		spec.epochs = 8;
		spec.batch_size = 2;
		spec.seed = 13;
		spec.shuffle = true;
		spec.gradient_clip_norm = 10.0;

		mtrc::solve::parametric::dnn::AutoencoderModel<double> model(make_autoencoder_network());
		assert(model.backend_name() == "native_dnn");
		assert(model.topology().bottleneck_layer == 0);
		assert(model.topology().decoder_layers.size() == 1);

		const mtrc::solve::parametric::dnn::NativeDnnTrainer<double> trainer;
		const auto report = trainer.fit(model, dataset, objective, spec);
		assert(report.epochs.size() == spec.epochs);
		assert(!report.stopped_early);
		assert(report.epochs.front().batch_count == 2);
		assert(report.epochs.back().terms.size() == 1);
		assert(report.epochs.back().terms[0].name == "reconstruction_mse");
		assert(is_finite(report.epochs.back().total_loss));
		assert(model.training_report().epochs.size() == spec.epochs);

		const auto latent = model.encode(features);
		assert(latent.rows() == features.rows());
		assert(latent.columns() == 1);

		const auto reconstructed = model.decode(latent);
		assert(reconstructed.rows() == features.rows());
		assert(reconstructed.columns() == features.columns());
	}

	{
		using record_type = std::vector<double>;

		auto source = mtrc::make_space(std::vector<record_type>{{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}},
										 mtrc::Euclidean<double>{});

		mtrc::solve::parametric::dnn::TrainingSpec<double> spec;
		spec.epochs = 6;
		spec.batch_size = 2;
		spec.seed = 17;
		spec.shuffle = false;

		auto mapping = mtrc::modify::map::native_autoencoder(
			mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network()), reconstruction_objective(), spec);
		static_assert(mtrc::Mapping_v<decltype(mapping), decltype(source)>);

		auto model = mtrc::modify::map::fit(mapping, source);
		static_assert(mtrc::MappingModel_v<decltype(model), decltype(source)>);
		assert(model.source_record_count() == source.size());
		assert(model.latent_dimension() == 1);
		assert(model.training_report().epochs.size() == spec.epochs);

		const auto reduced = mtrc::modify::map::transform(model, source);
		using reduced_type = typename std::decay<decltype(reduced)>::type;
		static_assert(std::is_same<typename reduced_type::space_type::record_type, std::vector<double>>::value);
		assert(reduced.mapping == "native_autoencoder");
		assert(reduced.strategy == "native_dnn");
		assert(reduced.representation == "metric_space");
		assert(reduced.inverse_supported);
		assert(reduced.source_record_count == source.size());
		assert(reduced.source_records.size() == source.size());
		assert(reduced.representative_records[0] == source.id(0));
		assert(reduced.space.size() == source.size());
		assert(reduced.space.record(reduced.space.id(0)).size() == 1);
		assert(is_finite(reduced.space.distance(reduced.space.id(0), reduced.space.id(1))));

		const auto restored = model.inverse_transform(reduced);
		assert(restored.size() == source.size());
		for (const auto &record : restored) {
			assert(record.size() == 2);
			for (const auto value : record) {
				assert(is_finite(value));
			}
		}
	}

	return 0;
}
