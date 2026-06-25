#include <cassert>
#include <iostream>

#include "metric/engine.hpp"

int main()
{
	auto builder = mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_builder(0.05, 1.0)
					   .use_distance_table_pairwise_distances()
					   .use_exponential_affinity_kernel()
					   .use_lazy_row_normalized_diffusion_operator();

	const auto plan = builder.plan();
	assert(plan.name() == "parametric_diffusion_coordinate_pipeline");
	assert(plan.has_component("codec", "vector_record_codec"));
	assert(plan.has_component("pairwise_distances", "distance_table_pairwise_distances"));
	assert(plan.has_component("affinity_kernel", "exponential_affinity_kernel"));
	assert(plan.has_component("diffusion_operator", "lazy_row_normalized_diffusion_operator"));
	assert(plan.has_component_parameter("loss", "reconstruction_mse_loss", "weight", "0.05"));

	auto metadata_builder = mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_builder(0.05, 1.0);
	metadata_builder.replace_affinity_kernel("adaptive_gaussian_affinity_kernel", {{"k", "5"}})
		.replace_target_generator("diffusion_potential_anchor_coordinates", {{"anchors", "landmarks"}});
	const auto metadata_plan = metadata_builder.plan();
	assert(metadata_plan.has_component("affinity_kernel", "adaptive_gaussian_affinity_kernel"));
	assert(metadata_plan.has_component_parameter("target_generator", "diffusion_potential_anchor_coordinates",
												 "anchors", "landmarks"));

	const auto replacement_points = mtrc::modify::compose::parametric_diffusion_coordinate_replacement_points();
	assert(replacement_points.size() == 10);
	const auto executable_default = mtrc::modify::compose::resolve_parametric_diffusion_coordinate_executable_hooks<double>(
		mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_plan(0.05, 1.0));
	assert(executable_default.executable());
	const auto executable_replaced = mtrc::modify::compose::resolve_parametric_diffusion_coordinate_executable_hooks<double>(plan);
	assert(executable_replaced.executable());
	assert(executable_replaced.codec == "vector_record_codec");
	assert(executable_replaced.pairwise_distances == "distance_table_pairwise_distances");
	assert(executable_replaced.affinity_kernel == "exponential_affinity_kernel");
	assert(executable_replaced.diffusion_operator == "lazy_row_normalized_diffusion_operator");
	const auto executable_metadata =
		mtrc::modify::compose::resolve_parametric_diffusion_coordinate_executable_hooks<double>(metadata_plan);
	assert(!executable_metadata.executable());

	std::cout << "diffusion coordinate pipeline builder plan = " << plan.name() << "\n";
	std::cout << "diffusion coordinate pipeline builder components = " << plan.component_count() << "\n";
	std::cout << "diffusion coordinate pipeline builder replacement points = " << replacement_points.size() << "\n";
	std::cout << "diffusion coordinate pipeline default executable = yes\n";
	std::cout << "diffusion coordinate pipeline distance-table executable = yes\n";
	std::cout << "diffusion coordinate pipeline metadata replacement executable = no\n";
	std::cout << "diffusion coordinate pipeline builder codec = vector_record_codec\n";
	std::cout << "diffusion coordinate pipeline builder distance provider = distance_table_pairwise_distances\n";
	std::cout << "diffusion coordinate pipeline builder affinity kernel = exponential_affinity_kernel\n";
	std::cout << "diffusion coordinate pipeline builder diffusion operator = lazy_row_normalized_diffusion_operator\n";

	return 0;
}
