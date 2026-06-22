#include <cassert>
#include <stdexcept>

#include "metric/engine.hpp"

int main()
{
	mtrc::modify::compose::PipelinePlan plan(
		"demo_pipeline", {{"space", "metric_space"},
						  {"pairwise_distances", "exact_space_distances", {{"mode", "exact"}}},
						  {"artifact", "native_mapping_artifact", {}, true}});

	assert(plan.name() == "demo_pipeline");
	assert(plan.component_count() == 3);
	assert(plan.has_component("space", "metric_space"));
	assert(plan.has_component("pairwise_distances", "exact_space_distances"));
	assert(plan.has_component_parameter("pairwise_distances", "exact_space_distances", "mode", "exact"));
	assert(!plan.has_component_parameter("pairwise_distances", "exact_space_distances", "mode", "lazy"));

	plan.validate_required_components(
		{{"space", "metric_space"}, {"pairwise_distances", "exact_space_distances"}});

	bool rejected_missing_required = false;
	try {
		plan.validate_required_components({{"trainer", "native_dnn_autoencoder_trainer"}});
	} catch (const mtrc::PipelineValidationError &) {
		rejected_missing_required = true;
	}
	assert(rejected_missing_required);

	const auto replacement =
		plan.replaced_component("pairwise_distances", "exact_space_distances",
								mtrc::modify::compose::PipelineComponent(
									"pairwise_distances", "distance_table_pairwise_distances", {{"mode", "materialized"}}));
	assert(plan.has_component("pairwise_distances", "exact_space_distances"));
	assert(!plan.has_component("pairwise_distances", "distance_table_pairwise_distances"));
	assert(!replacement.has_component("pairwise_distances", "exact_space_distances"));
	assert(replacement.has_component("pairwise_distances", "distance_table_pairwise_distances"));
	assert(replacement.has_component_parameter("pairwise_distances", "distance_table_pairwise_distances", "mode",
											   "materialized"));

	bool rejected_missing_replacement = false;
	try {
		(void)plan.replaced_component("loss", "missing_loss", {"loss", "other_loss"});
	} catch (const mtrc::PipelineValidationError &) {
		rejected_missing_replacement = true;
	}
	assert(rejected_missing_replacement);

	const auto json = mtrc::modify::compose::pipeline_plan_to_json(plan);
	assert(json.at("name") == "demo_pipeline");
	assert(json.at("components").size() == plan.component_count());
	assert(json.at("components").at(1).at("parameters").at("mode") == "exact");
	assert(json.at("components").at(2).at("artifact").at("contributes") == true);

	const auto phate_plan = mtrc::modify::compose::native_phate_autoencoder_pipeline_plan(0.05, 1.0);
	assert(phate_plan.name() == "native_phate_autoencoder_pipeline");
	assert(phate_plan.component_count() == 11);
	assert(phate_plan.has_component("codec", "vector_record_codec"));
	assert(phate_plan.has_component("target_generator", "diffusion_potential_anchor_coordinates"));
	assert(phate_plan.has_component_parameter("loss", "reconstruction_mse_loss", "weight", "0.05"));
	assert(phate_plan.has_component_parameter("loss", "bottleneck_coordinate_mse_loss", "weight", "1"));
	const auto executable_phate =
		mtrc::modify::compose::resolve_native_phate_autoencoder_executable_hooks<double>(phate_plan);
	assert(executable_phate.executable());
	assert(executable_phate.codec == "vector_record_codec");
	assert(executable_phate.pairwise_distances == "exact_space_distances");
	assert(executable_phate.affinity_kernel == "gaussian_affinity_kernel");
	assert(executable_phate.diffusion_operator == "row_normalized_diffusion_operator");
	assert(executable_phate.reconstruction_weight == 0.05);
	assert(executable_phate.geometry_weight == 1.0);
	executable_phate.validate();

	const auto cached_distance_plan = mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(0.05, 1.0)
										  .use_distance_table_pairwise_distances()
										  .plan();
	assert(cached_distance_plan.has_component("pairwise_distances", "distance_table_pairwise_distances"));
	const auto cached_distance_hooks =
		mtrc::modify::compose::resolve_native_phate_autoencoder_executable_hooks<double>(cached_distance_plan);
	assert(cached_distance_hooks.executable());
	assert(cached_distance_hooks.pairwise_distances == "distance_table_pairwise_distances");
	assert(cached_distance_hooks.affinity_kernel == "gaussian_affinity_kernel");
	assert(cached_distance_hooks.diffusion_operator == "row_normalized_diffusion_operator");
	assert(cached_distance_hooks.reconstruction_weight == 0.05);
	assert(cached_distance_hooks.geometry_weight == 1.0);
	cached_distance_hooks.validate();

	const auto feature_codec_plan =
		mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(0.05, 1.0).use_feature_record_codec().plan();
	assert(feature_codec_plan.has_component("codec", "feature_record_codec"));
	const auto feature_codec_hooks =
		mtrc::modify::compose::resolve_native_phate_autoencoder_executable_hooks<double>(feature_codec_plan);
	assert(feature_codec_hooks.executable());
	assert(feature_codec_hooks.codec == "feature_record_codec");
	assert(feature_codec_hooks.pairwise_distances == "exact_space_distances");
	feature_codec_hooks.validate();

	const auto exponential_affinity_plan = mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(0.05, 1.0)
											   .use_exponential_affinity_kernel()
											   .plan();
	assert(exponential_affinity_plan.has_component("affinity_kernel", "exponential_affinity_kernel"));
	const auto exponential_affinity_hooks =
		mtrc::modify::compose::resolve_native_phate_autoencoder_executable_hooks<double>(exponential_affinity_plan);
	assert(exponential_affinity_hooks.executable());
	assert(exponential_affinity_hooks.pairwise_distances == "exact_space_distances");
	assert(exponential_affinity_hooks.affinity_kernel == "exponential_affinity_kernel");
	assert(exponential_affinity_hooks.diffusion_operator == "row_normalized_diffusion_operator");
	assert(exponential_affinity_hooks.reconstruction_weight == 0.05);
	assert(exponential_affinity_hooks.geometry_weight == 1.0);
	exponential_affinity_hooks.validate();

	const auto lazy_diffusion_plan = mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(0.05, 1.0)
										 .use_lazy_row_normalized_diffusion_operator()
										 .plan();
	assert(lazy_diffusion_plan.has_component("diffusion_operator", "lazy_row_normalized_diffusion_operator"));
	const auto lazy_diffusion_hooks =
		mtrc::modify::compose::resolve_native_phate_autoencoder_executable_hooks<double>(lazy_diffusion_plan);
	assert(lazy_diffusion_hooks.executable());
	assert(lazy_diffusion_hooks.pairwise_distances == "exact_space_distances");
	assert(lazy_diffusion_hooks.affinity_kernel == "gaussian_affinity_kernel");
	assert(lazy_diffusion_hooks.diffusion_operator == "lazy_row_normalized_diffusion_operator");
	assert(lazy_diffusion_hooks.reconstruction_weight == 0.05);
	assert(lazy_diffusion_hooks.geometry_weight == 1.0);
	lazy_diffusion_hooks.validate();

	const auto geometry_only_plan = mtrc::modify::compose::native_phate_autoencoder_pipeline_plan(0.0, 1.0);
	assert(geometry_only_plan.component_count() == 10);
	assert(!geometry_only_plan.has_component("loss", "reconstruction_mse_loss"));
	assert(geometry_only_plan.has_component("loss", "bottleneck_coordinate_mse_loss"));
	const auto geometry_only_hooks =
		mtrc::modify::compose::resolve_native_phate_autoencoder_executable_hooks<double>(geometry_only_plan);
	assert(geometry_only_hooks.executable());
	assert(geometry_only_hooks.reconstruction_weight == 0.0);
	assert(geometry_only_hooks.geometry_weight == 1.0);

	auto builder = mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(0.05, 1.0);
	builder.replace_pairwise_distances("distance_table_pairwise_distances", {{"mode", "materialized"}})
		.replace_affinity_kernel("adaptive_gaussian_affinity_kernel", {{"k", "5"}})
		.replace_target_generator("diffusion_potential_anchor_coordinates", {{"anchors", "landmarks"}})
		.replace_reconstruction_loss("reconstruction_mse_loss", 0.1, {{"normalization", "record"}})
		.replace_geometry_loss("bottleneck_coordinate_mse_loss", 2.0, {{"target", "phate"}});
	const auto explicit_plan = builder.plan();
	assert(explicit_plan.component_count() == 11);
	assert(explicit_plan.has_component("pairwise_distances", "distance_table_pairwise_distances"));
	assert(explicit_plan.has_component_parameter("pairwise_distances", "distance_table_pairwise_distances", "mode",
												 "materialized"));
	assert(explicit_plan.has_component("affinity_kernel", "adaptive_gaussian_affinity_kernel"));
	assert(explicit_plan.has_component_parameter("affinity_kernel", "adaptive_gaussian_affinity_kernel", "k", "5"));
	assert(explicit_plan.has_component_parameter("loss", "reconstruction_mse_loss", "weight", "0.1"));
	assert(explicit_plan.has_component_parameter("loss", "reconstruction_mse_loss", "normalization", "record"));
	assert(explicit_plan.has_component_parameter("loss", "bottleneck_coordinate_mse_loss", "weight", "2"));
	assert(explicit_plan.has_component_parameter("loss", "bottleneck_coordinate_mse_loss", "target", "phate"));

	const auto explicit_json = mtrc::modify::compose::pipeline_plan_to_json(explicit_plan);
	assert(explicit_json.at("components").at(1).at("name") == "vector_record_codec");
	assert(explicit_json.at("components").at(2).at("name") == "distance_table_pairwise_distances");
	assert(explicit_json.at("components").at(9).at("parameters").at("normalization") == "record");
	const auto explicit_hooks =
		mtrc::modify::compose::resolve_native_phate_autoencoder_executable_hooks<double>(explicit_plan);
	assert(!explicit_hooks.executable());
	assert(!explicit_hooks.unsupported_reasons.empty());
	bool rejected_metadata_only_plan = false;
	try {
		explicit_hooks.validate();
	} catch (const mtrc::PipelineValidationError &) {
		rejected_metadata_only_plan = true;
	}
	assert(rejected_metadata_only_plan);

	const auto no_loss_plan = mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(1.0, 1.0)
								  .without_reconstruction_loss()
								  .without_geometry_loss()
								  .plan();
	const auto no_loss_hooks =
		mtrc::modify::compose::resolve_native_phate_autoencoder_executable_hooks<double>(no_loss_plan);
	assert(!no_loss_hooks.executable());

	const auto replacement_points = mtrc::modify::compose::native_phate_autoencoder_replacement_points();
	assert(replacement_points.size() == 10);
	assert(replacement_points.front().role == "pairwise_distances");
	assert(replacement_points.front().default_name == "exact_space_distances");
	assert(replacement_points.front().controls_execution);
	assert(replacement_points[1].role == "codec");
	assert(replacement_points[1].default_name == "vector_record_codec");

	// tc3: the pipeline plan round-trips losslessly through json. This is the one
	// structured payload that the artifact loader reconstructs, so the inverse of
	// the forward serialization must reproduce names, parameters, and the
	// contributes-to-artifact flag exactly.
	const auto demo_roundtrip = mtrc::modify::compose::pipeline_plan_from_json(json);
	assert(mtrc::modify::compose::pipeline_plan_to_json(demo_roundtrip) == json);
	assert(demo_roundtrip.name() == plan.name());
	assert(demo_roundtrip.component_count() == plan.component_count());
	assert(demo_roundtrip.has_component_parameter("pairwise_distances", "exact_space_distances", "mode", "exact"));
	assert(demo_roundtrip.has_component("artifact", "native_mapping_artifact"));

	const auto explicit_roundtrip = mtrc::modify::compose::pipeline_plan_from_json(explicit_json);
	assert(mtrc::modify::compose::pipeline_plan_to_json(explicit_roundtrip) == explicit_json);
	assert(explicit_roundtrip.component_count() == explicit_plan.component_count());
	assert(explicit_roundtrip.has_component("pairwise_distances", "distance_table_pairwise_distances"));
	assert(explicit_roundtrip.has_component_parameter("pairwise_distances", "distance_table_pairwise_distances", "mode",
													  "materialized"));
	assert(explicit_roundtrip.has_component_parameter("loss", "reconstruction_mse_loss", "normalization", "record"));
	assert(explicit_roundtrip.has_component_parameter("loss", "bottleneck_coordinate_mse_loss", "target", "phate"));

	return 0;
}
