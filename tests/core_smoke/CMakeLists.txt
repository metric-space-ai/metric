# The smoke gate proves its invariants with assert(). Release defines NDEBUG,
# which would compile every assert out and let the whole gate pass vacuously.
# Keep assertions live in every configuration so the gate is protective under the
# `release` preset and Release CI legs too. (-UNDEBUG is processed after the
# config's -DNDEBUG, so it wins; in Debug it is a harmless no-op.)
if (MSVC)
    add_compile_options(/UNDEBUG)
else ()
    add_compile_options(-UNDEBUG)
endif ()

add_executable(record_layer_smoke record_layer_smoke.cpp)
target_link_libraries(record_layer_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_core_smoke metric_core_smoke.cpp)
target_link_libraries(metric_core_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_mgc_smoke metric_mgc_smoke.cpp)
target_link_libraries(metric_mgc_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_mgc_properties_smoke metric_mgc_properties_smoke.cpp)
target_link_libraries(metric_mgc_properties_smoke PRIVATE ${METRIC_TARGET_NAME})

if (METRIC_BUILD_CORE_ENTROPY)
    add_executable(metric_entropy_smoke metric_entropy_smoke.cpp)
    target_link_libraries(metric_entropy_smoke PRIVATE ${METRIC_TARGET_NAME})
    add_executable(metric_entropy_properties_smoke metric_entropy_properties_smoke.cpp)
    target_link_libraries(metric_entropy_properties_smoke PRIVATE ${METRIC_TARGET_NAME})
    add_executable(metric_entropy_estimator_smoke metric_entropy_estimator_smoke.cpp)
    target_link_libraries(metric_entropy_estimator_smoke PRIVATE ${METRIC_TARGET_NAME})
    add_executable(engine_statistics_operators_smoke engine_statistics_operators_smoke.cpp)
    target_link_libraries(engine_statistics_operators_smoke PRIVATE ${METRIC_TARGET_NAME})
    add_executable(engine_pcfa_mapping_smoke engine_pcfa_mapping_smoke.cpp)
    target_link_libraries(engine_pcfa_mapping_smoke PRIVATE ${METRIC_TARGET_NAME})
    add_executable(engine_reduce_intent_smoke engine_reduce_intent_smoke.cpp)
    target_link_libraries(engine_reduce_intent_smoke PRIVATE ${METRIC_TARGET_NAME})
    add_executable(engine_embed_intent_smoke engine_embed_intent_smoke.cpp)
    target_link_libraries(engine_embed_intent_smoke PRIVATE ${METRIC_TARGET_NAME})
endif ()

add_executable(metric_contracts_smoke metric_contracts_smoke.cpp)
target_link_libraries(metric_contracts_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_mixed_records_smoke metric_mixed_records_smoke.cpp)
target_link_libraries(metric_mixed_records_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_twed_smoke metric_twed_smoke.cpp)
target_link_libraries(metric_twed_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_wasserstein_smoke metric_wasserstein_smoke.cpp)
target_link_libraries(metric_wasserstein_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_distribution_quarantine_smoke metric_distribution_quarantine_smoke.cpp)
target_link_libraries(metric_distribution_quarantine_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_catalog_additions_smoke metric_catalog_additions_smoke.cpp)
target_link_libraries(metric_catalog_additions_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_standardized_smoke metric_standardized_smoke.cpp)
target_link_libraries(metric_standardized_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_discovery_smoke metric_discovery_smoke.cpp)
target_link_libraries(metric_discovery_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_discovery_report_smoke metric_discovery_report_smoke.cpp)
target_link_libraries(metric_discovery_report_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(space_consistency_smoke space_consistency_smoke.cpp)
target_link_libraries(space_consistency_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_core_smoke engine_core_smoke.cpp)
target_link_libraries(engine_core_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_compat_smoke engine_compat_smoke.cpp)
target_link_libraries(engine_compat_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_representations_smoke engine_representations_smoke.cpp)
target_link_libraries(engine_representations_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_distance_matrix_id_smoke engine_distance_matrix_id_smoke.cpp)
target_link_libraries(engine_distance_matrix_id_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_distance_matrix_snapshot_smoke engine_distance_matrix_snapshot_smoke.cpp)
target_link_libraries(engine_distance_matrix_snapshot_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_representation_parity_smoke engine_representation_parity_smoke.cpp)
target_link_libraries(engine_representation_parity_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_nearest_operators_smoke engine_nearest_operators_smoke.cpp)
target_link_libraries(engine_nearest_operators_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_neighbor_oracle_smoke engine_neighbor_oracle_smoke.cpp)
target_link_libraries(engine_neighbor_oracle_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(storage_search_edge_parity_smoke storage_search_edge_parity_smoke.cpp)
target_link_libraries(storage_search_edge_parity_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_sparsify_operator_smoke engine_sparsify_operator_smoke.cpp)
target_link_libraries(engine_sparsify_operator_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_clustering_operators_smoke engine_clustering_operators_smoke.cpp)
target_link_libraries(engine_clustering_operators_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_dbscan_operator_smoke engine_dbscan_operator_smoke.cpp)
target_link_libraries(engine_dbscan_operator_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(structural_analysis_clustering_fixtures_smoke structural_analysis_clustering_fixtures_smoke.cpp)
target_link_libraries(structural_analysis_clustering_fixtures_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(stats_production_readiness_smoke stats_production_readiness_smoke.cpp)
target_link_libraries(stats_production_readiness_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_intent_strategy_smoke engine_intent_strategy_smoke.cpp)
target_link_libraries(engine_intent_strategy_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_mapping_model_smoke engine_mapping_model_smoke.cpp)
target_link_libraries(engine_mapping_model_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_map_intent_smoke engine_map_intent_smoke.cpp)
target_link_libraries(engine_map_intent_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_koc_mapping_smoke engine_koc_mapping_smoke.cpp)
target_link_libraries(engine_koc_mapping_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_denoise_intent_smoke engine_denoise_intent_smoke.cpp)
target_link_libraries(engine_denoise_intent_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(modify_level2_components_smoke modify_level2_components_smoke.cpp)
target_link_libraries(modify_level2_components_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(modify_represent_smoke modify_represent_smoke.cpp)
target_link_libraries(modify_represent_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(modify_expand_synthesis_smoke modify_expand_synthesis_smoke.cpp)
target_link_libraries(modify_expand_synthesis_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(modify_metric_status_smoke modify_metric_status_smoke.cpp)
target_link_libraries(modify_metric_status_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_compare_intent_smoke engine_compare_intent_smoke.cpp)
target_link_libraries(engine_compare_intent_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_cross_space_dependency_smoke engine_cross_space_dependency_smoke.cpp)
target_link_libraries(engine_cross_space_dependency_smoke PRIVATE ${METRIC_TARGET_NAME})
target_include_directories(engine_cross_space_dependency_smoke
    PRIVATE ${CMAKE_SOURCE_DIR}/examples/engine)

add_executable(engine_describe_intent_smoke engine_describe_intent_smoke.cpp)
target_link_libraries(engine_describe_intent_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_representatives_intent_smoke engine_representatives_intent_smoke.cpp)
target_link_libraries(engine_representatives_intent_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_compress_intent_smoke engine_compress_intent_smoke.cpp)
target_link_libraries(engine_compress_intent_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_outliers_intent_smoke engine_outliers_intent_smoke.cpp)
target_link_libraries(engine_outliers_intent_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_runtime_policy_smoke engine_runtime_policy_smoke.cpp)
target_link_libraries(engine_runtime_policy_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_representation_swap_smoke engine_representation_swap_smoke.cpp)
target_link_libraries(engine_representation_swap_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(benchmark_report_smoke benchmark_report_smoke.cpp)
target_link_libraries(benchmark_report_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_dynamic_matrix_smoke metric_numeric_dynamic_matrix_smoke.cpp)
target_link_libraries(metric_numeric_dynamic_matrix_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_elementwise_maps_smoke metric_numeric_elementwise_maps_smoke.cpp)
target_link_libraries(metric_numeric_elementwise_maps_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_expression_adaptors_smoke metric_numeric_expression_adaptors_smoke.cpp)
target_link_libraries(metric_numeric_expression_adaptors_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_fixed_vector_storage_smoke metric_numeric_fixed_vector_storage_smoke.cpp)
target_link_libraries(metric_numeric_fixed_vector_storage_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_functional_map_smoke metric_numeric_functional_map_smoke.cpp)
target_link_libraries(metric_numeric_functional_map_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_custom_matrix_smoke metric_numeric_custom_matrix_smoke.cpp)
target_link_libraries(metric_numeric_custom_matrix_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_diagonal_identity_smoke metric_numeric_diagonal_identity_smoke.cpp)
target_link_libraries(metric_numeric_diagonal_identity_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_diagonal_view_smoke metric_numeric_diagonal_view_smoke.cpp)
target_link_libraries(metric_numeric_diagonal_view_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_determinant_pd_smoke metric_numeric_determinant_pd_smoke.cpp)
target_link_libraries(metric_numeric_determinant_pd_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_inverse_llh_smoke metric_numeric_inverse_llh_smoke.cpp)
target_link_libraries(metric_numeric_inverse_llh_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_lifecycle_evaluate_smoke metric_numeric_lifecycle_evaluate_smoke.cpp)
target_link_libraries(metric_numeric_lifecycle_evaluate_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_matrix_reductions_smoke metric_numeric_matrix_reductions_smoke.cpp)
target_link_libraries(metric_numeric_matrix_reductions_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_matrix_statistics_smoke metric_numeric_matrix_statistics_smoke.cpp)
target_link_libraries(metric_numeric_matrix_statistics_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_matrix_primitives_smoke metric_numeric_matrix_primitives_smoke.cpp)
target_link_libraries(metric_numeric_matrix_primitives_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_graph_shortest_paths_smoke metric_numeric_graph_shortest_paths_smoke.cpp)
target_link_libraries(metric_numeric_graph_shortest_paths_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_solve_laplacian_smoke metric_solve_laplacian_smoke.cpp)
target_link_libraries(metric_solve_laplacian_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_minmax_smoke metric_numeric_minmax_smoke.cpp)
target_link_libraries(metric_numeric_minmax_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_norm_extrema_smoke metric_numeric_norm_extrema_smoke.cpp)
target_link_libraries(metric_numeric_norm_extrema_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_random_smoke metric_numeric_random_smoke.cpp)
target_link_libraries(metric_numeric_random_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_solve_eigen_smoke metric_numeric_solve_eigen_smoke.cpp)
target_link_libraries(metric_numeric_solve_eigen_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_serialization_smoke metric_numeric_serialization_smoke.cpp)
target_link_libraries(metric_numeric_serialization_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_sparse_matrix_smoke metric_numeric_sparse_matrix_smoke.cpp)
target_link_libraries(metric_numeric_sparse_matrix_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_svd_rank_smoke metric_numeric_svd_rank_smoke.cpp)
target_link_libraries(metric_numeric_svd_rank_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_sparse_vector_smoke metric_numeric_sparse_vector_smoke.cpp)
target_link_libraries(metric_numeric_sparse_vector_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_symmetric_matrix_smoke metric_numeric_symmetric_matrix_smoke.cpp)
target_link_libraries(metric_numeric_symmetric_matrix_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_transcendental_maps_smoke metric_numeric_transcendental_maps_smoke.cpp)
target_link_libraries(metric_numeric_transcendental_maps_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_vector_ops_smoke metric_numeric_vector_ops_smoke.cpp)
target_link_libraries(metric_numeric_vector_ops_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_views_smoke metric_numeric_views_smoke.cpp)
target_link_libraries(metric_numeric_views_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_graph_connected_components_smoke metric_graph_connected_components_smoke.cpp)
target_link_libraries(metric_graph_connected_components_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_triangular_adaptors_smoke metric_numeric_triangular_adaptors_smoke.cpp)
target_link_libraries(metric_numeric_triangular_adaptors_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_static_hybrid_matrix_smoke metric_numeric_static_hybrid_matrix_smoke.cpp)
target_link_libraries(metric_numeric_static_hybrid_matrix_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_selection_views_smoke metric_numeric_selection_views_smoke.cpp)
target_link_libraries(metric_numeric_selection_views_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_sparse_mutation_smoke metric_numeric_sparse_mutation_smoke.cpp)
target_link_libraries(metric_numeric_sparse_mutation_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_special_containers_smoke metric_numeric_special_containers_smoke.cpp)
target_link_libraries(metric_numeric_special_containers_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_numeric_facades_smoke metric_numeric_facades_smoke.cpp)
target_link_libraries(metric_numeric_facades_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(pipeline_plan_smoke pipeline_plan_smoke.cpp)
target_link_libraries(pipeline_plan_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_dnn_baseline_smoke native_dnn_baseline_smoke.cpp)
target_link_libraries(native_dnn_baseline_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_dnn_training_hooks_smoke native_dnn_training_hooks_smoke.cpp)
target_link_libraries(native_dnn_training_hooks_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_dnn_dataset_codec_smoke native_dnn_dataset_codec_smoke.cpp)
target_link_libraries(native_dnn_dataset_codec_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_dnn_composite_loss_smoke native_dnn_composite_loss_smoke.cpp)
target_link_libraries(native_dnn_composite_loss_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_dnn_loss_gradient_check_smoke native_dnn_loss_gradient_check_smoke.cpp)
target_link_libraries(native_dnn_loss_gradient_check_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_dnn_shape_guard_smoke native_dnn_shape_guard_smoke.cpp)
target_link_libraries(native_dnn_shape_guard_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_dnn_conv2d_parameters_smoke native_dnn_conv2d_parameters_smoke.cpp)
target_link_libraries(native_dnn_conv2d_parameters_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_dnn_conv2d_backprop_smoke native_dnn_conv2d_backprop_smoke.cpp)
target_link_libraries(native_dnn_conv2d_backprop_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_dnn_autoencoder_mapping_smoke native_dnn_autoencoder_mapping_smoke.cpp)
target_link_libraries(native_dnn_autoencoder_mapping_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_phate_geometry_targets_smoke native_phate_geometry_targets_smoke.cpp)
target_link_libraries(native_phate_geometry_targets_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(mapping_neighbor_preservation_smoke mapping_neighbor_preservation_smoke.cpp)
target_link_libraries(mapping_neighbor_preservation_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(mapping_out_of_sample_stability_smoke mapping_out_of_sample_stability_smoke.cpp)
target_link_libraries(mapping_out_of_sample_stability_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_dnn_phate_autoencoder_smoke native_dnn_phate_autoencoder_smoke.cpp)
target_link_libraries(native_dnn_phate_autoencoder_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_phate_feature_codec_smoke native_phate_feature_codec_smoke.cpp)
target_link_libraries(native_phate_feature_codec_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_phate_autoencoder_artifact_smoke native_phate_autoencoder_artifact_smoke.cpp)
target_link_libraries(native_phate_autoencoder_artifact_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_space_mapping_pipeline_smoke metric_space_mapping_pipeline_smoke.cpp)
target_link_libraries(metric_space_mapping_pipeline_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(native_dnn_autoencoder_artifact_smoke native_dnn_autoencoder_artifact_smoke.cpp)
target_link_libraries(native_dnn_autoencoder_artifact_smoke PRIVATE ${METRIC_TARGET_NAME})

if (LAPACK_FOUND)
    get_property(_metric_core_smoke_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach (_target IN LISTS _metric_core_smoke_targets)
        target_link_libraries(${_target} PRIVATE ${LAPACK_LIBRARIES})
    endforeach ()
endif ()

# These core-smoke targets call LAPACK directly (dsyevd / dpotrf / dpotrs /
# dgeev / dgesdd / dgesv / dgetrf / dgetri). Without a linked LAPACK they cannot
# link, so when LAPACK is unavailable they are excluded from the default build
# here and their tests are DISABLED at the end of this file (the rest of the core
# gate stays buildable LAPACK-free). LAPACK is a documented OPTIONAL backend, not
# a hard core dependency.
set(_metric_core_lapack_required_targets
    engine_representation_parity_smoke
    metric_numeric_determinant_pd_smoke
    metric_numeric_inverse_llh_smoke
    metric_numeric_solve_eigen_smoke
    metric_numeric_svd_rank_smoke
    metric_solve_laplacian_smoke)
if (NOT LAPACK_FOUND)
    set_target_properties(${_metric_core_lapack_required_targets} PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif ()

add_executable(metric_numeric_production_smoke metric_numeric_production_smoke.cpp)
target_link_libraries(metric_numeric_production_smoke PRIVATE ${METRIC_TARGET_NAME})

add_test(NAME record_layer_smoke COMMAND record_layer_smoke)
add_test(NAME metric_core_smoke COMMAND metric_core_smoke)
add_test(NAME metric_mgc_smoke COMMAND metric_mgc_smoke)
add_test(NAME metric_mgc_properties_smoke COMMAND metric_mgc_properties_smoke)
if (METRIC_BUILD_CORE_ENTROPY)
    add_test(NAME metric_entropy_smoke COMMAND metric_entropy_smoke)
    add_test(NAME metric_entropy_properties_smoke COMMAND metric_entropy_properties_smoke)
    add_test(NAME metric_entropy_estimator_smoke COMMAND metric_entropy_estimator_smoke)
    add_test(NAME engine_statistics_operators_smoke COMMAND engine_statistics_operators_smoke)
    add_test(NAME engine_pcfa_mapping_smoke COMMAND engine_pcfa_mapping_smoke)
    add_test(NAME engine_reduce_intent_smoke COMMAND engine_reduce_intent_smoke)
    add_test(NAME engine_embed_intent_smoke COMMAND engine_embed_intent_smoke)
endif ()
add_test(NAME metric_contracts_smoke COMMAND metric_contracts_smoke)
add_test(NAME metric_mixed_records_smoke COMMAND metric_mixed_records_smoke)
add_test(NAME metric_twed_smoke COMMAND metric_twed_smoke)
add_test(NAME metric_wasserstein_smoke COMMAND metric_wasserstein_smoke)
add_test(NAME metric_distribution_quarantine_smoke COMMAND metric_distribution_quarantine_smoke)
add_test(NAME metric_catalog_additions_smoke COMMAND metric_catalog_additions_smoke)
add_test(NAME metric_standardized_smoke COMMAND metric_standardized_smoke)
add_test(NAME metric_discovery_smoke COMMAND metric_discovery_smoke)
add_test(NAME metric_discovery_report_smoke COMMAND metric_discovery_report_smoke)
add_test(NAME space_consistency_smoke COMMAND space_consistency_smoke)
add_test(NAME engine_core_smoke COMMAND engine_core_smoke)
add_test(NAME engine_compat_smoke COMMAND engine_compat_smoke)
add_test(NAME engine_representations_smoke COMMAND engine_representations_smoke)
add_test(NAME engine_distance_matrix_id_smoke COMMAND engine_distance_matrix_id_smoke)
add_test(NAME engine_distance_matrix_snapshot_smoke COMMAND engine_distance_matrix_snapshot_smoke)
add_test(NAME engine_representation_parity_smoke COMMAND engine_representation_parity_smoke)
add_test(NAME engine_nearest_operators_smoke COMMAND engine_nearest_operators_smoke)
add_test(NAME engine_neighbor_oracle_smoke COMMAND engine_neighbor_oracle_smoke)
add_test(NAME storage_search_edge_parity_smoke COMMAND storage_search_edge_parity_smoke)
add_test(NAME engine_sparsify_operator_smoke COMMAND engine_sparsify_operator_smoke)
add_test(NAME engine_clustering_operators_smoke COMMAND engine_clustering_operators_smoke)
add_test(NAME engine_dbscan_operator_smoke COMMAND engine_dbscan_operator_smoke)
add_test(NAME structural_analysis_clustering_fixtures_smoke COMMAND structural_analysis_clustering_fixtures_smoke)
add_test(NAME stats_production_readiness_smoke COMMAND stats_production_readiness_smoke)
add_test(NAME engine_intent_strategy_smoke COMMAND engine_intent_strategy_smoke)
add_test(NAME engine_mapping_model_smoke COMMAND engine_mapping_model_smoke)
add_test(NAME engine_map_intent_smoke COMMAND engine_map_intent_smoke)
add_test(NAME engine_koc_mapping_smoke COMMAND engine_koc_mapping_smoke)
add_test(NAME engine_denoise_intent_smoke COMMAND engine_denoise_intent_smoke)
add_test(NAME modify_level2_components_smoke COMMAND modify_level2_components_smoke)
add_test(NAME modify_represent_smoke COMMAND modify_represent_smoke)
add_test(NAME modify_expand_synthesis_smoke COMMAND modify_expand_synthesis_smoke)
add_test(NAME modify_metric_status_smoke COMMAND modify_metric_status_smoke)
add_test(NAME engine_compare_intent_smoke COMMAND engine_compare_intent_smoke)
add_test(NAME engine_cross_space_dependency_smoke COMMAND engine_cross_space_dependency_smoke)
add_test(NAME engine_describe_intent_smoke COMMAND engine_describe_intent_smoke)
add_test(NAME engine_representatives_intent_smoke COMMAND engine_representatives_intent_smoke)
add_test(NAME engine_compress_intent_smoke COMMAND engine_compress_intent_smoke)
add_test(NAME engine_outliers_intent_smoke COMMAND engine_outliers_intent_smoke)
add_test(NAME engine_runtime_policy_smoke COMMAND engine_runtime_policy_smoke)
add_test(NAME engine_representation_swap_smoke COMMAND engine_representation_swap_smoke)
add_test(NAME benchmark_report_smoke COMMAND benchmark_report_smoke)
add_test(NAME metric_numeric_custom_matrix_smoke COMMAND metric_numeric_custom_matrix_smoke)
add_test(NAME metric_numeric_diagonal_identity_smoke COMMAND metric_numeric_diagonal_identity_smoke)
add_test(NAME metric_numeric_diagonal_view_smoke COMMAND metric_numeric_diagonal_view_smoke)
add_test(NAME metric_numeric_determinant_pd_smoke COMMAND metric_numeric_determinant_pd_smoke)
add_test(NAME metric_numeric_dynamic_matrix_smoke COMMAND metric_numeric_dynamic_matrix_smoke)
add_test(NAME metric_numeric_elementwise_maps_smoke COMMAND metric_numeric_elementwise_maps_smoke)
add_test(NAME metric_numeric_expression_adaptors_smoke COMMAND metric_numeric_expression_adaptors_smoke)
add_test(NAME metric_numeric_fixed_vector_storage_smoke COMMAND metric_numeric_fixed_vector_storage_smoke)
add_test(NAME metric_numeric_functional_map_smoke COMMAND metric_numeric_functional_map_smoke)
add_test(NAME metric_numeric_inverse_llh_smoke COMMAND metric_numeric_inverse_llh_smoke)
add_test(NAME metric_numeric_lifecycle_evaluate_smoke COMMAND metric_numeric_lifecycle_evaluate_smoke)
add_test(NAME metric_numeric_matrix_reductions_smoke COMMAND metric_numeric_matrix_reductions_smoke)
add_test(NAME metric_numeric_matrix_statistics_smoke COMMAND metric_numeric_matrix_statistics_smoke)
add_test(NAME metric_numeric_matrix_primitives_smoke COMMAND metric_numeric_matrix_primitives_smoke)
add_test(NAME metric_numeric_graph_shortest_paths_smoke COMMAND metric_numeric_graph_shortest_paths_smoke)
add_test(NAME metric_solve_laplacian_smoke COMMAND metric_solve_laplacian_smoke)
add_test(NAME metric_numeric_minmax_smoke COMMAND metric_numeric_minmax_smoke)
add_test(NAME metric_numeric_norm_extrema_smoke COMMAND metric_numeric_norm_extrema_smoke)
add_test(NAME metric_numeric_random_smoke COMMAND metric_numeric_random_smoke)
add_test(NAME metric_numeric_solve_eigen_smoke COMMAND metric_numeric_solve_eigen_smoke)
add_test(NAME metric_numeric_serialization_smoke COMMAND metric_numeric_serialization_smoke)
add_test(
    NAME metric_numeric_header_only_compile_smoke
    COMMAND ${CMAKE_CXX_COMPILER}
        ${CMAKE_CXX17_STANDARD_COMPILE_OPTION}
        -DMETRIC_NUMERIC_BLAS_MODE=0
        -I${CMAKE_SOURCE_DIR}
        -c
        ${CMAKE_CURRENT_SOURCE_DIR}/metric_numeric_header_only_compile_smoke.cpp
        -o
        ${CMAKE_CURRENT_BINARY_DIR}/metric_numeric_header_only_compile_smoke.o
)
add_test(NAME metric_numeric_sparse_matrix_smoke COMMAND metric_numeric_sparse_matrix_smoke)
add_test(NAME metric_numeric_production_smoke COMMAND metric_numeric_production_smoke)
add_test(NAME metric_numeric_svd_rank_smoke COMMAND metric_numeric_svd_rank_smoke)
add_test(NAME metric_numeric_sparse_vector_smoke COMMAND metric_numeric_sparse_vector_smoke)
add_test(NAME metric_numeric_symmetric_matrix_smoke COMMAND metric_numeric_symmetric_matrix_smoke)
add_test(NAME metric_numeric_transcendental_maps_smoke COMMAND metric_numeric_transcendental_maps_smoke)
add_test(NAME metric_numeric_vector_ops_smoke COMMAND metric_numeric_vector_ops_smoke)
add_test(NAME metric_numeric_views_smoke COMMAND metric_numeric_views_smoke)
add_test(NAME metric_graph_connected_components_smoke COMMAND metric_graph_connected_components_smoke)
add_test(NAME metric_numeric_triangular_adaptors_smoke COMMAND metric_numeric_triangular_adaptors_smoke)
add_test(NAME metric_numeric_static_hybrid_matrix_smoke COMMAND metric_numeric_static_hybrid_matrix_smoke)
add_test(NAME metric_numeric_selection_views_smoke COMMAND metric_numeric_selection_views_smoke)
add_test(NAME metric_numeric_sparse_mutation_smoke COMMAND metric_numeric_sparse_mutation_smoke)
add_test(NAME metric_numeric_special_containers_smoke COMMAND metric_numeric_special_containers_smoke)
add_test(NAME metric_numeric_facades_smoke COMMAND metric_numeric_facades_smoke)
add_test(
    NAME metric_numeric_facades_standalone_compile_smoke
    COMMAND ${CMAKE_COMMAND}
        -DMETRIC_CXX=${CMAKE_CXX_COMPILER}
        -DMETRIC_CXX17_FLAG=${CMAKE_CXX17_STANDARD_COMPILE_OPTION}
        -DMETRIC_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DMETRIC_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/numeric_facade_standalone_compile.cmake
)
set_tests_properties(
    metric_numeric_custom_matrix_smoke
    metric_numeric_diagonal_identity_smoke
    metric_numeric_diagonal_view_smoke
    metric_numeric_determinant_pd_smoke
    metric_numeric_dynamic_matrix_smoke
    metric_numeric_elementwise_maps_smoke
    metric_numeric_expression_adaptors_smoke
    metric_numeric_fixed_vector_storage_smoke
    metric_numeric_functional_map_smoke
    metric_numeric_inverse_llh_smoke
    metric_numeric_lifecycle_evaluate_smoke
    metric_numeric_matrix_reductions_smoke
    metric_numeric_matrix_statistics_smoke
    metric_numeric_matrix_primitives_smoke
    metric_numeric_graph_shortest_paths_smoke
    metric_numeric_minmax_smoke
    metric_numeric_norm_extrema_smoke
    metric_numeric_header_only_compile_smoke
    metric_numeric_production_smoke
    metric_numeric_random_smoke
    metric_numeric_serialization_smoke
    metric_numeric_solve_eigen_smoke
    metric_numeric_sparse_matrix_smoke
    metric_numeric_sparse_vector_smoke
    metric_numeric_svd_rank_smoke
    metric_numeric_symmetric_matrix_smoke
    metric_numeric_transcendental_maps_smoke
    metric_numeric_vector_ops_smoke
    metric_numeric_views_smoke
    metric_graph_connected_components_smoke
    metric_numeric_triangular_adaptors_smoke
    metric_numeric_static_hybrid_matrix_smoke
    metric_numeric_selection_views_smoke
    metric_numeric_sparse_mutation_smoke
    metric_numeric_special_containers_smoke
    metric_numeric_facades_smoke
    metric_numeric_facades_standalone_compile_smoke
    PROPERTIES LABELS metric_numeric_core
)
add_test(NAME pipeline_plan_smoke COMMAND pipeline_plan_smoke)
add_test(NAME native_dnn_baseline_smoke COMMAND native_dnn_baseline_smoke)
add_test(NAME native_dnn_training_hooks_smoke COMMAND native_dnn_training_hooks_smoke)
add_test(NAME native_dnn_dataset_codec_smoke COMMAND native_dnn_dataset_codec_smoke)
add_test(NAME native_dnn_composite_loss_smoke COMMAND native_dnn_composite_loss_smoke)
add_test(NAME native_dnn_loss_gradient_check_smoke COMMAND native_dnn_loss_gradient_check_smoke)
add_test(NAME native_dnn_shape_guard_smoke COMMAND native_dnn_shape_guard_smoke)
add_test(NAME native_dnn_conv2d_parameters_smoke COMMAND native_dnn_conv2d_parameters_smoke)
add_test(NAME native_dnn_conv2d_backprop_smoke COMMAND native_dnn_conv2d_backprop_smoke)
add_test(NAME native_dnn_autoencoder_mapping_smoke COMMAND native_dnn_autoencoder_mapping_smoke)
add_test(NAME native_phate_geometry_targets_smoke COMMAND native_phate_geometry_targets_smoke)
add_test(NAME mapping_neighbor_preservation_smoke COMMAND mapping_neighbor_preservation_smoke)
add_test(NAME mapping_out_of_sample_stability_smoke COMMAND mapping_out_of_sample_stability_smoke)
add_test(NAME native_dnn_phate_autoencoder_smoke COMMAND native_dnn_phate_autoencoder_smoke)
add_test(NAME native_phate_feature_codec_smoke COMMAND native_phate_feature_codec_smoke)
add_test(NAME native_phate_autoencoder_artifact_smoke COMMAND native_phate_autoencoder_artifact_smoke)
add_test(NAME native_dnn_autoencoder_artifact_smoke COMMAND native_dnn_autoencoder_artifact_smoke)
add_test(NAME metric_space_mapping_pipeline_smoke COMMAND metric_space_mapping_pipeline_smoke)

set_tests_properties(
    modify_level2_components_smoke
    PROPERTIES LABELS metric_modify_components
)

set_tests_properties(
    metric_solve_laplacian_smoke
    PROPERTIES LABELS metric_solve_laplacian
)

set_tests_properties(
    engine_distance_matrix_id_smoke
    PROPERTIES LABELS metric_distance_matrix_id
)

set_tests_properties(
    engine_distance_matrix_snapshot_smoke
    PROPERTIES LABELS metric_distance_matrix_snapshot
)

set_tests_properties(
    engine_neighbor_oracle_smoke
    PROPERTIES LABELS metric_neighbor_oracle
)

set_tests_properties(
    storage_search_edge_parity_smoke
    PROPERTIES LABELS metric_storage_search_parity
)

set_tests_properties(
    pipeline_plan_smoke
    native_phate_geometry_targets_smoke
    native_dnn_phate_autoencoder_smoke
    native_phate_feature_codec_smoke
    native_phate_autoencoder_artifact_smoke
    metric_space_mapping_pipeline_smoke
    PROPERTIES LABELS metric_phate_pipeline
)

set_tests_properties(
    benchmark_report_smoke
    PROPERTIES LABELS metric_benchmark_report
)

# Level-1 stats production-readiness fixtures (search range parity / NaN-safe ordering,
# MGC dependence-testing edge cases, structural-analysis determinism).
add_executable(stats_search_range_parity_smoke stats_search_range_parity_smoke.cpp)
target_link_libraries(stats_search_range_parity_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(stats_correlate_dependence_fixtures_smoke stats_correlate_dependence_fixtures_smoke.cpp)
target_link_libraries(stats_correlate_dependence_fixtures_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(stats_structural_determinism_smoke stats_structural_determinism_smoke.cpp)
target_link_libraries(stats_structural_determinism_smoke PRIVATE ${METRIC_TARGET_NAME})

if (LAPACK_FOUND)
    target_link_libraries(stats_search_range_parity_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(stats_correlate_dependence_fixtures_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(stats_structural_determinism_smoke PRIVATE ${LAPACK_LIBRARIES})
endif ()

add_test(NAME stats_search_range_parity_smoke COMMAND stats_search_range_parity_smoke)
add_test(NAME stats_correlate_dependence_fixtures_smoke COMMAND stats_correlate_dependence_fixtures_smoke)
add_test(NAME stats_structural_determinism_smoke COMMAND stats_structural_determinism_smoke)

# Level-1 space production-readiness fixtures (legacy Matrix/Tree/KNNGraph correctness regressions,
# lineage-preserving split/merge/select of finite metric spaces).
add_executable(space_legacy_reps_smoke space_legacy_reps_smoke.cpp)
target_link_libraries(space_legacy_reps_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(space_partition_lineage_smoke space_partition_lineage_smoke.cpp)
target_link_libraries(space_partition_lineage_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(space_search_parity_smoke space_search_parity_smoke.cpp)
target_link_libraries(space_search_parity_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(user_workflow_api_smoke user_workflow_api_smoke.cpp)
target_link_libraries(user_workflow_api_smoke PRIVATE ${METRIC_TARGET_NAME})

# User-facing mtrc::space API: builder/records/distances/query, stale cache recovery, lineage workflow.
add_executable(space_user_api_smoke space_user_api_smoke.cpp)
target_link_libraries(space_user_api_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(space_cache_recovery_smoke space_cache_recovery_smoke.cpp)
target_link_libraries(space_cache_recovery_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(space_lineage_workflow_smoke space_lineage_workflow_smoke.cpp)
target_link_libraries(space_lineage_workflow_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(space_persistence_smoke space_persistence_smoke.cpp)
target_link_libraries(space_persistence_smoke PRIVATE ${METRIC_TARGET_NAME})

if (LAPACK_FOUND)
    target_link_libraries(space_legacy_reps_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(space_partition_lineage_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(space_search_parity_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(user_workflow_api_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(space_user_api_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(space_cache_recovery_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(space_lineage_workflow_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(space_persistence_smoke PRIVATE ${LAPACK_LIBRARIES})
endif ()

add_test(NAME space_legacy_reps_smoke COMMAND space_legacy_reps_smoke)
add_test(NAME space_partition_lineage_smoke COMMAND space_partition_lineage_smoke)
add_test(NAME space_search_parity_smoke COMMAND space_search_parity_smoke)
add_test(NAME user_workflow_api_smoke COMMAND user_workflow_api_smoke)
add_test(NAME space_user_api_smoke COMMAND space_user_api_smoke)
add_test(NAME space_cache_recovery_smoke COMMAND space_cache_recovery_smoke)
add_test(NAME space_lineage_workflow_smoke COMMAND space_lineage_workflow_smoke)
add_test(NAME space_persistence_smoke COMMAND space_persistence_smoke)

# Runtime metric-law coverage for the AIRM Riemannian distance. It instantiates
# RiemannianDistance::operator(), which calls the LAPACK generalized symmetric
# eigensolver (sygv), so the target is only built when LAPACK is available.
# metric_discovery_smoke covers the declared law via static_assert regardless.
if (LAPACK_FOUND)
    add_executable(metric_riemannian_law_smoke metric_riemannian_law_smoke.cpp)
    target_link_libraries(metric_riemannian_law_smoke PRIVATE ${METRIC_TARGET_NAME} ${LAPACK_LIBRARIES})
    add_test(NAME metric_riemannian_law_smoke COMMAND metric_riemannian_law_smoke)
endif ()

# User-facing mtrc::stats API: cohesive profile + distance distribution, batch search and
# sample lineage, MGC significance/permutation + RecordId alignment, cluster validity
# diagnostics, and k-NN outlier scoring.
add_executable(stats_user_api_properties_smoke stats_user_api_properties_smoke.cpp)
target_link_libraries(stats_user_api_properties_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(stats_user_api_search_sample_smoke stats_user_api_search_sample_smoke.cpp)
target_link_libraries(stats_user_api_search_sample_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(stats_user_api_correlate_smoke stats_user_api_correlate_smoke.cpp)
target_link_libraries(stats_user_api_correlate_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(stats_user_api_structural_smoke stats_user_api_structural_smoke.cpp)
target_link_libraries(stats_user_api_structural_smoke PRIVATE ${METRIC_TARGET_NAME})

add_executable(stats_space_diagnosis_smoke stats_space_diagnosis_smoke.cpp)
target_link_libraries(stats_space_diagnosis_smoke PRIVATE ${METRIC_TARGET_NAME})

if (LAPACK_FOUND)
    target_link_libraries(stats_user_api_properties_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(stats_user_api_search_sample_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(stats_user_api_correlate_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(stats_user_api_structural_smoke PRIVATE ${LAPACK_LIBRARIES})
    target_link_libraries(stats_space_diagnosis_smoke PRIVATE ${LAPACK_LIBRARIES})
endif ()

add_test(NAME stats_user_api_properties_smoke COMMAND stats_user_api_properties_smoke)
add_test(NAME stats_user_api_search_sample_smoke COMMAND stats_user_api_search_sample_smoke)
add_test(NAME stats_user_api_correlate_smoke COMMAND stats_user_api_correlate_smoke)
add_test(NAME stats_user_api_structural_smoke COMMAND stats_user_api_structural_smoke)
add_test(NAME stats_space_diagnosis_smoke COMMAND stats_space_diagnosis_smoke)

# Entropy result-status coverage uses the LAPACK-backed kpN estimator, so it is gated like
# the other entropy core targets.
if (METRIC_BUILD_CORE_ENTROPY)
    add_executable(stats_entropy_status_smoke stats_entropy_status_smoke.cpp)
    target_link_libraries(stats_entropy_status_smoke PRIVATE ${METRIC_TARGET_NAME})
    if (LAPACK_FOUND)
        target_link_libraries(stats_entropy_status_smoke PRIVATE ${LAPACK_LIBRARIES})
    endif ()
    add_test(NAME stats_entropy_status_smoke COMMAND stats_entropy_status_smoke)
endif ()

# The LAPACK-requiring tests stay registered (so the LABELS set_tests_properties
# calls above resolve) but are DISABLED when LAPACK is absent; their binaries are
# EXCLUDE_FROM_ALL (see above), so ctest skips them and the no-LAPACK core build
# and gate stay green. With LAPACK present, nothing here changes.
if (NOT LAPACK_FOUND)
    set_tests_properties(${_metric_core_lapack_required_targets} PROPERTIES DISABLED TRUE)
endif ()
