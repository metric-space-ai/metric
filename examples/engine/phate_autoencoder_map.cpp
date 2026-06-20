#include <cassert>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <metric/distance.hpp>
#include <metric/engine.hpp>
#include <metric/utils/dnn.hpp>

namespace {

using record_type = std::vector<double>;

auto metric_law_name(metric::metric_law law) -> std::string
{
	switch (law) {
	case metric::metric_law::distance:
		return "distance";
	case metric::metric_law::metric:
		return "metric";
	case metric::metric_law::pseudo_metric:
		return "pseudo_metric";
	case metric::metric_law::unknown:
		return "unknown";
	}
	return "unknown";
}

auto record_kind_name(metric::record_kind kind) -> std::string
{
	switch (kind) {
	case metric::record_kind::custom:
		return "custom";
	case metric::record_kind::vector:
		return "vector";
	case metric::record_kind::aligned_vector:
		return "aligned_vector";
	case metric::record_kind::string:
		return "string";
	case metric::record_kind::sequence:
		return "sequence";
	case metric::record_kind::structured:
		return "structured";
	}
	return "custom";
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
	static_assert(metric::metric_traits<metric::Euclidean<double>>::law == metric::metric_law::metric,
				  "Euclidean source metric should satisfy the metric law");
	static_assert(metric::metric_traits<metric::Euclidean<double>>::records == metric::record_kind::aligned_vector,
				  "the native autoencoder fixture uses aligned vector records");

	const std::vector<record_type> records{{0.0, 0.0}, {0.5, 0.5}, {1.0, 1.0}, {1.5, 1.5}, {2.0, 2.0}};
	auto space = metric::make_space(records, metric::Euclidean<double>{});

	metric::representations::MatrixCache<decltype(space)> matrix(space);
	metric::representations::KnnGraphIndex<decltype(space)> graph(space, 2);
	const auto graph_stats = graph.stats_against(matrix, records.size());
	assert(graph_stats.nodes == records.size());
	assert(graph_stats.recall_validated);
	assert(graph_stats.sampled_recall == 1.0);

	const auto materialized_policy = metric::runtime::materialized(metric::runtime::exact());
	const auto runtime_diagnostics =
		metric::runtime::diagnostics_for_space(space, materialized_policy, "knn_graph_index", "learnable_map");
	assert(runtime_diagnostics.policy_name == "exact_materialized_serial");
	assert(runtime_diagnostics.representation == "knn_graph_index");
	assert(runtime_diagnostics.supported);

	const auto source_neighbors =
		metric::find_neighbors(space, space.id(0), metric::count{2}, materialized_policy);
	assert(source_neighbors.representation == "matrix_cache");
	assert(source_neighbors.size() == 2);
	assert(source_neighbors[0].id == space.id(1));

	const auto source_groups = metric::find_groups(space, metric::strategies::k_medoids(2), materialized_policy);
	assert(source_groups.algorithm == "kmedoids");
	assert(source_groups.cluster_count == 2);
	assert(source_groups.record_count == records.size());
	assert(source_groups.representation == "matrix_cache");

	const auto source_outliers = metric::find_outliers(space, metric::strategies::dbscan(0.8, 2), materialized_policy);
	assert(source_outliers.strategy == "dbscan_noise");
	assert(source_outliers.noise_count == 0);
	assert(source_outliers.representation == "matrix_cache");

	metric::mappings::PhateGeometrySpec<double> geometry;
	geometry.dimensions = 1;
	geometry.diffusion_steps = 2;
	geometry.kernel_scale = 1.0;

	const auto targets = metric::mappings::phate_geometry_targets<decltype(space), double>(space, geometry);
	assert(targets.coordinates.size() == records.size());
	assert(targets.dimensions == geometry.dimensions);
	assert(targets.diffusion_steps == geometry.diffusion_steps);
	assert(targets.kernel_scale == geometry.kernel_scale);
	assert(targets.method == "diffusion_potential_anchor_coordinates");

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
	const std::vector<record_type> new_records{{0.25, 0.25}, {1.75, 1.75}};
	auto new_space = metric::make_space(new_records, metric::Euclidean<double>{});
	auto new_latent = metric::mappings::transform(model, new_space);

	const auto &report = model.training_report();
	const auto initial_bottleneck = find_epoch_term(report.epochs.front(), "bottleneck_coordinate_mse");
	const auto final_bottleneck = find_epoch_term(report.epochs.back(), "bottleneck_coordinate_mse");
	assert(final_bottleneck < initial_bottleneck);
	assert(report.epochs.size() == training.epochs);
	assert(latent.inverse_supported);
	assert(latent.space.record(latent.space.id(0)).size() == geometry.dimensions);
	assert(latent.space.distance(latent.space.id(0), latent.space.id(1)) <
		   latent.space.distance(latent.space.id(0), latent.space.id(4)));
	assert(new_latent.space.size() == new_records.size());
	assert(new_latent.space.record(new_latent.space.id(0)).size() == geometry.dimensions);
	assert(new_latent.source_record_count == new_records.size());

	const auto reconstruction_error = reconstruction_mse(records, restored);
	assert(std::isfinite(reconstruction_error));

	std::cout << "source record_kind = "
			  << record_kind_name(metric::metric_traits<metric::Euclidean<double>>::records) << "\n";
	std::cout << "source metric_law = " << metric_law_name(metric::metric_traits<metric::Euclidean<double>>::law)
			  << "\n";
	std::cout << "source fixture size = " << records.size() << "\n";
	std::cout << "source codec = vector_record_codec\n";
	std::cout << "source nearest neighbors = " << source_neighbors.size() << " via "
			  << source_neighbors.representation << "\n";
	std::cout << "source groups = " << source_groups.cluster_count << " via " << source_groups.representation << "\n";
	std::cout << "source dbscan noise = " << source_outliers.noise_count << " via "
			  << source_outliers.representation << "\n";
	std::cout << "runtime policy = " << runtime_diagnostics.policy_name << " via "
			  << runtime_diagnostics.representation << "\n";
	std::cout << "diffusion steps = " << targets.diffusion_steps << "\n";
	std::cout << "diffusion kernel scale = " << targets.kernel_scale << "\n";
	std::cout << "diffusion graph recall = " << graph_stats.sampled_recall << "\n";
	std::cout << "training epochs = " << report.epochs.size() << "\n";
	std::cout << "PHATE-AE bottleneck loss: " << initial_bottleneck << " -> " << final_bottleneck << "\n";
	std::cout << "PHATE-AE reconstruction MSE: " << reconstruction_error << "\n";
	std::cout << "out-of-sample support = parametric_space_transform\n";
	std::cout << "out-of-sample records = " << new_latent.space.size() << "\n";
	std::cout << "deterministic seed = " << training.seed << "\n";

	return 0;
}
