add_executable(metric_space_strings metric_space_strings.cpp)
target_link_libraries(metric_space_strings PRIVATE ${METRIC_TARGET_NAME})

add_executable(custom_metric_space custom_metric_space.cpp)
target_link_libraries(custom_metric_space PRIVATE ${METRIC_TARGET_NAME})

add_executable(explicit_space_representations explicit_space_representations.cpp)
target_link_libraries(explicit_space_representations PRIVATE ${METRIC_TARGET_NAME})

add_executable(space_user_workflow space_user_workflow.cpp)
target_link_libraries(space_user_workflow PRIVATE ${METRIC_TARGET_NAME})

add_executable(space_persistence space_persistence.cpp)
target_link_libraries(space_persistence PRIVATE ${METRIC_TARGET_NAME})

add_executable(record_csv_workflow record_csv_workflow.cpp)
target_link_libraries(record_csv_workflow PRIVATE ${METRIC_TARGET_NAME})
target_compile_definitions(record_csv_workflow
    PRIVATE METRIC_CORE_VECTOR_RECORDS_CSV="${CMAKE_CURRENT_SOURCE_DIR}/assets/vector_records.csv")

if (METRIC_BUILD_CORE_ENTROPY)
    add_executable(metric_space_entropy metric_space_entropy.cpp)
    target_link_libraries(metric_space_entropy PRIVATE ${METRIC_TARGET_NAME})
endif ()

add_executable(metric_space_correlation metric_space_correlation.cpp)
target_link_libraries(metric_space_correlation PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_space_intrinsic_dimension metric_space_intrinsic_dimension.cpp)
target_link_libraries(metric_space_intrinsic_dimension PRIVATE ${METRIC_TARGET_NAME})

add_executable(representative_selection_space representative_selection_space.cpp)
target_link_libraries(representative_selection_space PRIVATE ${METRIC_TARGET_NAME})

add_executable(time_series_twed_space time_series_twed_space.cpp)
target_link_libraries(time_series_twed_space PRIVATE ${METRIC_TARGET_NAME})

add_executable(histogram_emd_space histogram_emd_space.cpp)
target_link_libraries(histogram_emd_space PRIVATE ${METRIC_TARGET_NAME})

# Investigating an existing finite metric space with the mtrc::stats user API:
# profile + distribution, search, cluster diagnostics, k-NN outliers, and MGC significance.
add_executable(stats_space_profile stats_space_profile.cpp)
target_link_libraries(stats_space_profile PRIVATE ${METRIC_TARGET_NAME})

if (LAPACK_FOUND)
    get_property(_metric_core_example_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach (_target IN LISTS _metric_core_example_targets)
        target_link_libraries(${_target} PRIVATE ${LAPACK_LIBRARIES})
    endforeach ()
endif ()

if (BUILD_TESTING)
    add_test(NAME example_metric_space_strings COMMAND metric_space_strings)
    add_test(NAME example_custom_metric_space COMMAND custom_metric_space)
    add_test(NAME example_explicit_space_representations COMMAND explicit_space_representations)
    add_test(NAME example_space_user_workflow COMMAND space_user_workflow)
    add_test(NAME example_space_persistence COMMAND space_persistence)
    add_test(NAME example_record_csv_workflow COMMAND record_csv_workflow)
    if (METRIC_BUILD_CORE_ENTROPY)
        add_test(NAME example_metric_space_entropy COMMAND metric_space_entropy)
    endif ()
    add_test(NAME example_metric_space_correlation COMMAND metric_space_correlation)
    add_test(NAME example_metric_space_intrinsic_dimension COMMAND metric_space_intrinsic_dimension)
    add_test(NAME example_representative_selection_space COMMAND representative_selection_space)
    add_test(NAME example_time_series_twed_space COMMAND time_series_twed_space)
    add_test(NAME example_histogram_emd_space COMMAND histogram_emd_space)
    add_test(NAME example_stats_space_profile COMMAND stats_space_profile)
endif ()
