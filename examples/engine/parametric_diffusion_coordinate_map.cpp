#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>

namespace {

using record_type = std::vector<double>;

auto record_kind_name(mtrc::record_kind kind) -> std::string
{
	switch (kind) {
	case mtrc::record_kind::custom:
		return "custom";
	case mtrc::record_kind::vector:
		return "vector";
	case mtrc::record_kind::aligned_vector:
		return "aligned_vector";
	case mtrc::record_kind::string:
		return "string";
	case mtrc::record_kind::sequence:
		return "sequence";
	case mtrc::record_kind::structured:
		return "structured";
	}
	return "custom";
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

} // namespace

int main()
{
	static_assert(mtrc::metric_traits<mtrc::Euclidean<double>>::law == mtrc::metric_law::metric,
				  "Euclidean source metric should satisfy the metric law");
	static_assert(mtrc::metric_traits<mtrc::Euclidean<double>>::records == mtrc::record_kind::aligned_vector,
				  "the parametric coordinate solver fixture uses aligned vector records");

	const std::vector<record_type> records{{0.0, 0.0}, {0.5, 0.5}, {1.0, 1.0}, {1.5, 1.5}, {2.0, 2.0}};
	auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});

	mtrc::space::storage::DistanceTable<decltype(space)> matrix(space);
	mtrc::space::storage::KnnGraphIndex<decltype(space)> graph(space, 2);
	const auto graph_stats = graph.stats_against(matrix, records.size());
	assert(graph_stats.nodes == records.size());
	assert(graph_stats.recall_validated);
	assert(graph_stats.sampled_recall == 1.0);

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto runtime_diagnostics =
		mtrc::space::storage::diagnostics_for_space(space, materialized_policy, "knn_graph_index", "learnable_map");
	assert(runtime_diagnostics.policy_name == "exact_materialized_serial");
	assert(runtime_diagnostics.representation == "knn_graph_index");
	assert(runtime_diagnostics.supported);

	const auto source_neighbors = mtrc::find_neighbors(space, space.id(0), mtrc::count{2}, materialized_policy);
	assert(source_neighbors.representation == "distance_table");
	assert(source_neighbors.size() == 2);
	assert(source_neighbors[0].id == space.id(1));

	const auto source_groups = mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(2), materialized_policy);
	assert(source_groups.algorithm == "kmedoids");
	assert(source_groups.cluster_count == 2);
	assert(source_groups.record_count == records.size());
	assert(source_groups.representation == "distance_table");

	const auto source_outliers = mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(0.8, 2), materialized_policy);
	assert(source_outliers.strategy == "dbscan_density_outlier");
	assert(source_outliers.unassigned_count == 0);
	assert(source_outliers.representation == "distance_table");

	mtrc::modify::map::DiffusionCoordinateSpec<double> geometry;
	geometry.dimensions = 1;
	geometry.diffusion_steps = 2;
	geometry.kernel_scale = 1.0;

	const auto pipeline_builder = mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_builder(0.05, 1.0)
									  .use_distance_table_pairwise_distances()
									  .use_exponential_affinity_kernel()
									  .use_lazy_row_normalized_diffusion_operator();
	const auto targets = mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
		space, geometry, "distance_table_pairwise_distances", "exponential_affinity_kernel",
		"lazy_row_normalized_diffusion_operator");
	assert(targets.coordinates.size() == records.size());
	assert(targets.dimensions == geometry.dimensions);
	assert(targets.diffusion_steps == geometry.diffusion_steps);
	assert(targets.record_count == records.size());
	assert(targets.dense_distance_evaluations == records.size() * records.size());
	assert(targets.kernel_scale == geometry.kernel_scale);
	assert(targets.method == "diffusion_potential_anchor_coordinates");
	assert(targets.pairwise_distances == "distance_table_pairwise_distances");
	assert(targets.affinity_kernel == "exponential_affinity_kernel");
	assert(targets.diffusion_operator == "lazy_row_normalized_diffusion_operator");

	mtrc::modify::map::CoordinateCalibrationSpec<double> calibration;
	calibration.steps = 160;
	calibration.batch_size = records.size();
	calibration.shuffle = false;
	calibration.seed = 23;
	calibration.gradient_clip_norm = 20.0;

	mtrc::solve::parametric::LinearCoordinateSolverSpec<double> solver_spec;
	solver_spec.input_dimensions = records.front().size();
	solver_spec.coordinate_dimensions = geometry.dimensions;
	solver_spec.learning_rate = 0.001;
	solver_spec.encoder_weights = {0.10, 0.08};
	solver_spec.encoder_bias = {0.00};
	solver_spec.decoder_weights = {0.09, 0.07};
	solver_spec.decoder_bias = {0.00, 0.00};

	auto pipeline = mtrc::modify::compose::parametric_diffusion_coordinates(
		pipeline_builder, mtrc::solve::parametric::make_linear_coordinate_solver(solver_spec), geometry, calibration);
	static_assert(mtrc::Mapping_v<decltype(pipeline), decltype(space)>);
	assert(pipeline.name() == "parametric_diffusion_coordinate_pipeline");
	assert(pipeline.codec() == "vector_record_codec");
	assert(pipeline.pairwise_distances() == "distance_table_pairwise_distances");
	assert(pipeline.affinity_kernel() == "exponential_affinity_kernel");
	assert(pipeline.diffusion_operator() == "lazy_row_normalized_diffusion_operator");
	assert(pipeline.has_component("codec", "vector_record_codec"));
	assert(pipeline.has_component("pairwise_distances", "distance_table_pairwise_distances"));
	assert(pipeline.has_component("affinity_kernel", "exponential_affinity_kernel"));
	assert(pipeline.has_component("diffusion_operator", "lazy_row_normalized_diffusion_operator"));
	assert(pipeline.has_component("target_generator", "diffusion_potential_anchor_coordinates"));
	assert(pipeline.has_component("coordinate_calibration", "native_coordinate_calibration"));
	assert(pipeline.has_component("loss", "reconstruction_mse_loss"));

	auto mapping_artifact = mtrc::modify::map::derive_from(pipeline, space);
	assert(mapping_artifact.has_pipeline_plan());
	assert(mapping_artifact.pipeline_plan().has_component("pairwise_distances", "distance_table_pairwise_distances"));
	assert(mapping_artifact.pipeline_plan().has_component("affinity_kernel", "exponential_affinity_kernel"));
	assert(mapping_artifact.pipeline_plan().has_component("diffusion_operator", "lazy_row_normalized_diffusion_operator"));
	auto latent = mtrc::modify::map::transform(mapping_artifact, space);
	auto restored = mapping_artifact.inverse_transform(latent);
	const std::vector<record_type> new_records{{0.25, 0.25}, {1.75, 1.75}};
	auto new_space = mtrc::make_space(new_records, mtrc::Euclidean<double>{});
	const auto vector_baseline_anchor = mtrc::find_neighbors(space, new_records.front(), mtrc::count{1});
	assert(vector_baseline_anchor.size() == 1);
	auto new_latent = mtrc::modify::map::transform(mapping_artifact, new_space);

	const auto &report = mapping_artifact.calibration_report();
	const auto calibrated_steps = mtrc::solve::parametric::coordinate_calibration_step_count(report);
	const auto initial_coordinate_error = mtrc::solve::parametric::coordinate_calibration_target_error(
		report, mtrc::solve::parametric::CoordinateCalibrationPoint::first);
	const auto final_coordinate_error = mtrc::solve::parametric::coordinate_calibration_target_error(
		report, mtrc::solve::parametric::CoordinateCalibrationPoint::last);
	assert(final_coordinate_error < initial_coordinate_error);
	assert(calibrated_steps == calibration.steps);
	assert(latent.inverse_supported);
	assert(latent.mapping == "parametric_diffusion_coordinates");
	assert(latent.strategy == "native_metric_diffusion_coordinate_solver");
	assert(latent.space.record(latent.space.id(0)).size() == geometry.dimensions);
	assert(latent.space.distance(latent.space.id(0), latent.space.id(1)) <
		   latent.space.distance(latent.space.id(0), latent.space.id(4)));
	assert(new_latent.space.size() == new_records.size());
	assert(new_latent.space.record(new_latent.space.id(0)).size() == geometry.dimensions);
	assert(new_latent.source_record_count == new_records.size());
	assert(vector_baseline_anchor[0].id == space.id(0));

	const auto preservation = mtrc::modify::map::neighbor_preservation(space, latent, 1);
	assert(preservation.recall >= 0.8);
	const auto oos_stability = mtrc::modify::map::out_of_sample_neighbor_stability(mapping_artifact, space, new_space, 1);
	assert(oos_stability.anchor_recall >= 0.5);

	const auto reconstruction_error = reconstruction_mse(records, restored);
	assert(std::isfinite(reconstruction_error));

	std::cout << "source record_kind = " << record_kind_name(mtrc::metric_traits<mtrc::Euclidean<double>>::records)
			  << "\n";
	std::cout << "source metric_law = " << mtrc::metric_law_name(mtrc::metric_traits<mtrc::Euclidean<double>>::law)
			  << "\n";
	std::cout << "source fixture size = " << records.size() << "\n";
	std::cout << "source codec = vector_record_codec\n";
	std::cout << "source nearest neighbors = " << source_neighbors.size() << " via " << source_neighbors.representation
			  << "\n";
	std::cout << "source groups = " << source_groups.cluster_count << " via " << source_groups.representation << "\n";
	std::cout << "source DBSCAN density-unassigned records = " << source_outliers.unassigned_count << " via " << source_outliers.representation
			  << "\n";
	std::cout << "runtime policy = " << runtime_diagnostics.policy_name << " via " << runtime_diagnostics.representation
			  << "\n";
	std::cout << "pipeline = " << pipeline.name() << "\n";
	std::cout << "pipeline components = " << pipeline.component_count() << "\n";
	std::cout << "pipeline codec = " << pipeline.codec() << "\n";
	std::cout << "pipeline distance provider = " << pipeline.pairwise_distances() << "\n";
	std::cout << "pipeline affinity kernel = " << pipeline.affinity_kernel() << "\n";
	std::cout << "pipeline diffusion operator = " << pipeline.diffusion_operator() << "\n";
	std::cout << "diffusion steps = " << targets.diffusion_steps << "\n";
	std::cout << "diffusion dense evaluations = " << targets.dense_distance_evaluations << "\n";
	std::cout << "diffusion kernel scale = " << targets.kernel_scale << "\n";
	std::cout << "diffusion graph recall = " << graph_stats.sampled_recall << "\n";
	std::cout << "calibration steps = " << calibrated_steps << "\n";
	std::cout << "mapping = " << latent.mapping << "\n";
	std::cout << "strategy = " << latent.strategy << "\n";
	std::cout << "parametric diffusion coordinate target error: " << initial_coordinate_error << " -> "
			  << final_coordinate_error << "\n";
	std::cout << "parametric diffusion coordinate neighbor preservation = " << preservation.recall << "\n";
	std::cout << "parametric diffusion coordinate reconstruction MSE: " << reconstruction_error << "\n";
	std::cout << "vector baseline out-of-sample support = nearest_anchor_only\n";
	std::cout << "vector baseline inverse support = no_decoder\n";
	std::cout << "vector baseline first anchor = " << vector_baseline_anchor[0].id.index() << "\n";
	std::cout << "out-of-sample support = parametric_space_transform\n";
	std::cout << "inverse support = " << (latent.inverse_supported ? "decoder_inverse_transform" : "none") << "\n";
	std::cout << "out-of-sample records = " << new_latent.space.size() << "\n";
	std::cout << "out-of-sample anchor recall = " << oos_stability.anchor_recall << "\n";
	std::cout << "out-of-sample first-anchor match = " << oos_stability.first_anchor_match_rate << "\n";
	std::cout << "out-of-sample mapped-anchor rank = " << oos_stability.average_mapped_best_source_rank << "\n";
	std::cout << "deterministic seed = " << calibration.seed << "\n";

	return 0;
}
