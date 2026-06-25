#include <cassert>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/core/metadata.hpp>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"

namespace {

using record_type = std::vector<double>;

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool { return std::abs(lhs - rhs) <= tolerance; }

auto add_identity_dense_layer(mtrc::solve::parametric::dnn::Network<double> &network, std::size_t input_size, std::size_t output_size)
	-> void
{
	mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.0, 0.0);
	network.addLayer(layer);
}

auto make_autoencoder_network() -> mtrc::solve::parametric::dnn::Network<double>
{
	mtrc::solve::parametric::dnn::Network<double> network;
	add_identity_dense_layer(network, 2, 1);
	add_identity_dense_layer(network, 1, 2);
	network.setOutput(mtrc::solve::parametric::dnn::RegressionMSE<double>());
	network.setOptimizer(mtrc::solve::parametric::dnn::RMSProp<double>(0.001, 1.0e-8, 0.0));
	network.layers[0]->setParameters({{0.10, 0.08}, {0.00}});
	network.layers[1]->setParameters({{0.09, 0.07}, {0.00, 0.00}});
	return network;
}

template <typename Result> auto assert_same_latent_space(const Result &lhs, const Result &rhs) -> void
{
	assert(lhs.mapping == rhs.mapping);
	assert(lhs.strategy == rhs.strategy);
	assert(lhs.source_record_count == rhs.source_record_count);
	assert(lhs.inverse_supported == rhs.inverse_supported);
	assert(lhs.space.size() == rhs.space.size());
	for (std::size_t index = 0; index < lhs.space.size(); ++index) {
		const auto lhs_record = lhs.space.record(lhs.space.id(index));
		const auto rhs_record = rhs.space.record(rhs.space.id(index));
		assert(lhs_record.size() == rhs_record.size());
		for (std::size_t column = 0; column < lhs_record.size(); ++column) {
			assert(close(lhs_record[column], rhs_record[column]));
		}
	}
}

auto assert_same_records(const std::vector<record_type> &lhs, const std::vector<record_type> &rhs) -> void
{
	assert(lhs.size() == rhs.size());
	for (std::size_t row = 0; row < lhs.size(); ++row) {
		assert(lhs[row].size() == rhs[row].size());
		for (std::size_t column = 0; column < lhs[row].size(); ++column) {
			assert(close(lhs[row][column], rhs[row][column]));
		}
	}
}

auto has_pipeline_component(const mtrc::core::Metadata &components, const std::string &role, const std::string &name) -> bool
{
	for (const auto &component : components) {
		if (component.at("role") == role && component.at("name") == name) {
			return true;
		}
	}
	return false;
}

} // namespace

int main()
{
	const std::vector<record_type> records{{0.0, 0.0}, {0.5, 0.5}, {1.0, 1.0}, {1.5, 1.5}, {2.0, 2.0}};
	auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});

	mtrc::modify::map::DiffusionCoordinateSpec<double> geometry;
	geometry.dimensions = 1;
	geometry.diffusion_steps = 2;
	geometry.kernel_scale = 1.0;
	geometry.max_dense_records = records.size();
	const auto targets = mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
		space, geometry, "distance_table_pairwise_distances", "exponential_affinity_kernel",
		"lazy_row_normalized_diffusion_operator");

	mtrc::modify::map::CoordinateCalibrationSpec<double> calibration;
	calibration.steps = 80;
	calibration.batch_size = records.size();
	calibration.shuffle = false;
	calibration.seed = 23;
	calibration.gradient_clip_norm = 20.0;

	auto pipeline_builder = mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_builder(0.05, 1.0)
								.use_distance_table_pairwise_distances()
								.use_exponential_affinity_kernel()
								.use_lazy_row_normalized_diffusion_operator();
	auto pipeline = mtrc::modify::compose::parametric_diffusion_coordinates(
		pipeline_builder, mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network()), geometry, calibration);
	auto mapping_artifact = mtrc::modify::map::derive_from(pipeline, space);
	assert(mapping_artifact.has_pipeline_plan());
	assert(mapping_artifact.pipeline_plan().has_component("pairwise_distances", "distance_table_pairwise_distances"));
	assert(mapping_artifact.pipeline_plan().has_component("affinity_kernel", "exponential_affinity_kernel"));
	assert(mapping_artifact.pipeline_plan().has_component("diffusion_operator", "lazy_row_normalized_diffusion_operator"));
	const auto objective =
		mtrc::modify::map::parametric_diffusion_coordinate_objective(mapping_artifact.coordinate_solver(), targets, 0.05, 1.0);

	const auto artifact = mtrc::modify::map::make_parametric_diffusion_coordinate_artifact(mapping_artifact, objective, calibration, geometry,
																				   targets, space.version());
	assert(artifact.manifest.at("format") == "metric.parametric_diffusion_coordinate_artifact");
	assert(artifact.manifest.at("format_version") == 1);
	assert(artifact.manifest.at("backend") == "native_dnn");
	assert(artifact.manifest.at("mapping").at("name") == "parametric_diffusion_coordinates");
	assert(artifact.manifest.at("mapping").at("strategy") == "native_metric_diffusion_coordinate_solver");
	assert(artifact.manifest.at("pipeline").at("name") == "parametric_diffusion_coordinate_pipeline");
	assert(artifact.manifest.at("pipeline").at("components").size() == pipeline.component_count());
	assert(has_pipeline_component(artifact.manifest.at("pipeline").at("components"), "pairwise_distances",
								  "distance_table_pairwise_distances"));
	assert(has_pipeline_component(artifact.manifest.at("pipeline").at("components"), "affinity_kernel",
								  "exponential_affinity_kernel"));
	assert(has_pipeline_component(artifact.manifest.at("pipeline").at("components"), "diffusion_operator",
								  "lazy_row_normalized_diffusion_operator"));
	assert(artifact.manifest.at("diffusion_coordinates").at("spec").at("dimensions") == geometry.dimensions);
	assert(artifact.manifest.at("diffusion_coordinates").at("spec").at("diffusion_steps") == geometry.diffusion_steps);
	assert(artifact.manifest.at("diffusion_coordinates").at("spec").at("max_dense_records") == geometry.max_dense_records);
	assert(artifact.manifest.at("diffusion_coordinates").at("targets").at("target_count") == records.size());
	assert(artifact.manifest.at("diffusion_coordinates").at("targets").at("pairwise_distances") ==
		   "distance_table_pairwise_distances");
	assert(artifact.manifest.at("diffusion_coordinates").at("targets").at("affinity_kernel") == "exponential_affinity_kernel");
	assert(artifact.manifest.at("diffusion_coordinates").at("targets").at("diffusion_operator") ==
		   "lazy_row_normalized_diffusion_operator");
	assert(artifact.manifest.at("diffusion_coordinates").at("targets").at("dense_distance_evaluations") ==
		   records.size() * records.size());
	assert(artifact.manifest.at("loss").at("terms").size() == 2);
	assert(artifact.manifest.at("source").at("record_count") == records.size());
	assert(artifact.manifest.at("source").at("space_version") == space.version());
	assert(artifact.diagnostics.at("step_count") == calibration.steps);

	// tc1: calibration metadata is recorded on the derived artifact and serialized into
	// the artifact's manifest and diagnostics blocks.
	assert(mapping_artifact.calibration_report().epochs.size() == calibration.steps);
	assert(mapping_artifact.calibration_report().epochs.front().terms.size() == 2);
	assert(artifact.diagnostics.at("steps").size() == calibration.steps);
	assert(artifact.diagnostics.at("stopped_early") == false);
	assert(artifact.manifest.at("calibration_spec").at("calibration_seed") == calibration.seed);
	assert(artifact.manifest.at("calibration_spec").at("calibration_steps") == calibration.steps);

	auto loaded = mtrc::modify::map::load_parametric_diffusion_coordinate_artifact<record_type, double>(artifact);

	// tc1 (load contract): calibration metadata is provenance-only. Loading rebuilds
	// the network from the artifact, so the loaded mapping_artifact has no calibration report.
	assert(loaded.calibration_report().epochs.empty());
	assert(loaded.mapping_name() == mapping_artifact.mapping_name());
	assert(loaded.strategy_name() == mapping_artifact.strategy_name());
	assert(loaded.source_record_count() == mapping_artifact.source_record_count());
	assert(loaded.latent_dimension() == mapping_artifact.latent_dimension());
	assert(loaded.has_pipeline_plan());
	assert(loaded.pipeline_plan().has_component("pairwise_distances", "distance_table_pairwise_distances"));
	assert(loaded.pipeline_plan().has_component("affinity_kernel", "exponential_affinity_kernel"));
	assert(loaded.pipeline_plan().has_component("diffusion_operator", "lazy_row_normalized_diffusion_operator"));

	const auto latent = mtrc::modify::map::transform(mapping_artifact, space);
	const auto loaded_latent = mtrc::modify::map::transform(loaded, space);
	assert_same_latent_space(latent, loaded_latent);
	assert_same_records(mapping_artifact.inverse_transform(latent), loaded.inverse_transform(loaded_latent));

	// lc3: the one-to-one source lineage survives the artifact roundtrip.
	assert(latent.source_records == loaded_latent.source_records);
	assert(latent.representative_records == loaded_latent.representative_records);
	assert(latent.source_record_count == loaded_latent.source_record_count);

	// tc4: two independent fits with identical inputs and fixed seed produce
	// byte-identical serialized networks, identical final loss, and identical
	// latents -- the artifact is reproducible.
	{
		auto pipeline_a = mtrc::modify::compose::parametric_diffusion_coordinates(
			pipeline_builder, mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network()), geometry,
			calibration);
		auto pipeline_b = mtrc::modify::compose::parametric_diffusion_coordinates(
			pipeline_builder, mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network()), geometry,
			calibration);
		auto model_a = mtrc::modify::map::derive_from(pipeline_a, space);
		auto model_b = mtrc::modify::map::derive_from(pipeline_b, space);
		const auto artifact_a = mtrc::modify::map::make_parametric_diffusion_coordinate_artifact(model_a, objective, calibration,
																						  geometry, targets, space.version());
		const auto artifact_b = mtrc::modify::map::make_parametric_diffusion_coordinate_artifact(model_b, objective, calibration,
																						  geometry, targets, space.version());
		assert(artifact_a.network_binary == artifact_b.network_binary);
		assert(artifact_a.diagnostics.at("steps").back().at("objective_value") ==
			   artifact_b.diagnostics.at("steps").back().at("objective_value"));
		assert_same_latent_space(mtrc::modify::map::transform(model_a, space),
								 mtrc::modify::map::transform(model_b, space));
	}

	// Framing / negative cases: the loader admits only the native_dnn backend and
	// the vector record codec.
	{
		auto foreign_backend = artifact;
		foreign_backend.manifest["backend"] = "some_other_backend";
		bool rejected_backend = false;
		try {
			(void)mtrc::modify::map::load_parametric_diffusion_coordinate_artifact<record_type, double>(foreign_backend);
		} catch (const std::invalid_argument &) {
			rejected_backend = true;
		}
		assert(rejected_backend);

		auto foreign_codec = artifact;
		foreign_codec.manifest["codec"]["type"] = "RecordCoordinateCodec";
		bool rejected_codec = false;
		try {
			(void)mtrc::modify::map::load_parametric_diffusion_coordinate_artifact<record_type, double>(foreign_codec);
		} catch (const std::invalid_argument &) {
			rejected_codec = true;
		}
		assert(rejected_codec);
	}

	return 0;
}
