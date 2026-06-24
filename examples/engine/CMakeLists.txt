# The hero examples double as reproducible evidence / regression checks via
# assert(). Release defines NDEBUG, which would compile those asserts out and let
# the binaries (and the published benchmark report generated from a Release build)
# return 0 / emit green numbers even when an invariant is violated. Keep the
# evidence checks live in every configuration. (-UNDEBUG is processed after the
# config's -DNDEBUG, so it wins; in Debug it is a harmless no-op.)
if (MSVC)
    add_compile_options(/UNDEBUG)
else ()
    add_compile_options(-UNDEBUG)
endif ()

add_executable(engine_strings_edit_space strings_edit_space.cpp)
target_link_libraries(engine_strings_edit_space PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_process_curves_space process_curves_space.cpp)
target_link_libraries(engine_process_curves_space PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_process_curve_external_gallery process_curve_external_gallery.cpp)
target_link_libraries(engine_process_curve_external_gallery PRIVATE ${METRIC_TARGET_NAME})
target_compile_definitions(engine_process_curve_external_gallery
    PRIVATE
        METRIC_PROCESS_CURVE_POWER_DEMAND_CSV="${CMAKE_CURRENT_SOURCE_DIR}/assets/process_curve_power_demand_gallery.csv"
        METRIC_PROCESS_CURVE_INTERNAL_BLEEDING_CSV="${CMAKE_CURRENT_SOURCE_DIR}/assets/process_curve_internal_bleeding_gallery.csv"
)

add_executable(engine_mnist_phate_autoencoder_smoke mnist_phate_autoencoder_smoke.cpp)
target_link_libraries(engine_mnist_phate_autoencoder_smoke PRIVATE ${METRIC_TARGET_NAME})
target_compile_definitions(engine_mnist_phate_autoencoder_smoke
    PRIVATE
        METRIC_MNIST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../mapping_examples/assets/mnist"
)

add_executable(engine_mnist_phate_autoencoder_gallery mnist_phate_autoencoder_gallery.cpp)
target_link_libraries(engine_mnist_phate_autoencoder_gallery PRIVATE ${METRIC_TARGET_NAME})
target_compile_definitions(engine_mnist_phate_autoencoder_gallery
    PRIVATE
        METRIC_MNIST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../mapping_examples/assets/mnist"
)

add_executable(engine_mnist_grae10_integrity mnist_grae10_integrity.cpp)
target_link_libraries(engine_mnist_grae10_integrity PRIVATE ${METRIC_TARGET_NAME})
target_compile_definitions(engine_mnist_grae10_integrity
    PRIVATE
        METRIC_MNIST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../mapping_examples/assets/mnist"
        METRIC_GRAE10_DATA_JSON="${CMAKE_SOURCE_DIR}/visual/examples/grae10-metric-engine/grae10-data.json"
)

add_executable(engine_vector_space_special_case vector_space_special_case.cpp)
target_link_libraries(engine_vector_space_special_case PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_histogram_transport_space histogram_transport_space.cpp)
target_link_libraries(engine_histogram_transport_space PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_distribution_image_recoding distribution_image_recoding.cpp)
target_link_libraries(engine_distribution_image_recoding PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_mixed_structured_records mixed_structured_records.cpp)
target_link_libraries(engine_mixed_structured_records PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_cross_space_mgc cross_space_mgc.cpp)
target_link_libraries(engine_cross_space_mgc PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_cross_space_dependency cross_space_dependency.cpp)
target_link_libraries(engine_cross_space_dependency PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_representation_swap representation_swap.cpp)
target_link_libraries(engine_representation_swap PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_modify_space_components modify_space_components.cpp)
target_link_libraries(engine_modify_space_components PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_phate_autoencoder_map phate_autoencoder_map.cpp)
target_link_libraries(engine_phate_autoencoder_map PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_process_curve_phate_map process_curve_phate_map.cpp)
target_link_libraries(engine_process_curve_phate_map PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_process_curve_phate_gallery process_curve_phate_gallery.cpp)
target_link_libraries(engine_process_curve_phate_gallery PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_phate_pipeline_builder phate_pipeline_builder.cpp)
target_link_libraries(engine_phate_pipeline_builder PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_metric_space_mapping_pipeline metric_space_mapping_pipeline.cpp)
target_link_libraries(engine_metric_space_mapping_pipeline PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_benchmark_report benchmark_report.cpp)
target_link_libraries(engine_benchmark_report PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_hero_application_benchmarks hero_application_benchmarks.cpp)
target_link_libraries(engine_hero_application_benchmarks PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_condition_monitoring condition_monitoring.cpp)
target_link_libraries(engine_condition_monitoring PRIVATE ${METRIC_TARGET_NAME})
# Entropy is a LAPACK-backed space property (numeric-core determinant / EP path). Mirror the
# repo's core-entropy gate: demonstrate the entropy property only where LAPACK is
# available; otherwise the example degrades to its structure-based space properties.
if (METRIC_BUILD_CORE_ENTROPY)
    target_compile_definitions(engine_condition_monitoring PRIVATE METRIC_HERO_WITH_ENTROPY)
endif ()

add_executable(engine_finite_metric_dynamics finite_metric_dynamics.cpp)
target_link_libraries(engine_finite_metric_dynamics PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_mixed_finite_records mixed_finite_records.cpp)
target_link_libraries(engine_mixed_finite_records PRIVATE ${METRIC_TARGET_NAME})

if (LAPACK_FOUND)
    get_property(_metric_engine_example_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach (_target IN LISTS _metric_engine_example_targets)
        target_link_libraries(${_target} PRIVATE ${LAPACK_LIBRARIES})
    endforeach ()
endif ()

if (BUILD_TESTING)
    add_test(NAME example_engine_strings_edit_space COMMAND engine_strings_edit_space)
    add_test(NAME example_engine_process_curves_space COMMAND engine_process_curves_space)
    add_test(NAME example_engine_process_curve_external_gallery COMMAND engine_process_curve_external_gallery)
    add_test(NAME example_engine_mnist_phate_autoencoder_smoke COMMAND engine_mnist_phate_autoencoder_smoke --per-class 6 --epochs 4)
    add_test(NAME example_engine_mnist_phate_autoencoder_gallery COMMAND engine_mnist_phate_autoencoder_gallery --per-class 3 --epochs 2)
    add_test(NAME example_engine_mnist_grae10_integrity COMMAND engine_mnist_grae10_integrity)
    add_test(NAME example_engine_vector_space_special_case COMMAND engine_vector_space_special_case)
    add_test(NAME example_engine_histogram_transport_space COMMAND engine_histogram_transport_space)
    add_test(NAME example_engine_distribution_image_recoding COMMAND engine_distribution_image_recoding)
    add_test(NAME example_engine_mixed_structured_records COMMAND engine_mixed_structured_records)
    add_test(NAME example_engine_cross_space_mgc COMMAND engine_cross_space_mgc)
    add_test(NAME example_engine_cross_space_dependency COMMAND engine_cross_space_dependency)
    add_test(NAME example_engine_representation_swap COMMAND engine_representation_swap)
    add_test(NAME example_engine_modify_space_components COMMAND engine_modify_space_components)
    add_test(NAME example_engine_phate_autoencoder_map COMMAND engine_phate_autoencoder_map)
    add_test(NAME example_engine_process_curve_phate_map COMMAND engine_process_curve_phate_map)
    add_test(NAME example_engine_process_curve_phate_gallery COMMAND engine_process_curve_phate_gallery)
    add_test(NAME example_engine_phate_pipeline_builder COMMAND engine_phate_pipeline_builder)
    add_test(NAME example_engine_metric_space_mapping_pipeline COMMAND engine_metric_space_mapping_pipeline)
    add_test(NAME example_engine_benchmark_report COMMAND engine_benchmark_report)
    add_test(NAME example_engine_hero_application_benchmarks COMMAND engine_hero_application_benchmarks)
    add_test(NAME example_engine_condition_monitoring COMMAND engine_condition_monitoring)
    add_test(NAME example_engine_finite_metric_dynamics COMMAND engine_finite_metric_dynamics)
    add_test(NAME example_engine_mixed_finite_records COMMAND engine_mixed_finite_records)

    set_tests_properties(
        example_engine_strings_edit_space
        example_engine_process_curves_space
        example_engine_process_curve_external_gallery
        example_engine_mnist_grae10_integrity
        example_engine_histogram_transport_space
        example_engine_distribution_image_recoding
        example_engine_mixed_structured_records
        example_engine_cross_space_mgc
        example_engine_cross_space_dependency
        example_engine_modify_space_components
        example_engine_condition_monitoring
        example_engine_finite_metric_dynamics
        example_engine_mixed_finite_records
        PROPERTIES LABELS metric_application_evidence
    )

    set_tests_properties(
        example_engine_mnist_phate_autoencoder_smoke
        example_engine_mnist_phate_autoencoder_gallery
        PROPERTIES LABELS "metric_phate_pipeline;metric_mnist;metric_smoke"
    )

    set_tests_properties(
        example_engine_mnist_grae10_integrity
        PROPERTIES LABELS "metric_application_evidence;metric_mnist;metric_visual_integrity"
    )

    set_tests_properties(
        example_engine_phate_autoencoder_map
        example_engine_process_curve_phate_map
        example_engine_process_curve_phate_gallery
        example_engine_metric_space_mapping_pipeline
        PROPERTIES LABELS "metric_application_evidence;metric_phate_pipeline"
    )

    set_tests_properties(
        example_engine_phate_pipeline_builder
        PROPERTIES LABELS metric_phate_pipeline
    )

    set_tests_properties(
        example_engine_benchmark_report
        example_engine_hero_application_benchmarks
        PROPERTIES LABELS "metric_application_evidence;metric_benchmark_report"
    )
endif ()
