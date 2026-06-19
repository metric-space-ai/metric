add_executable(metric_space_strings metric_space_strings.cpp)
target_link_libraries(metric_space_strings PRIVATE ${METRIC_TARGET_NAME})

add_executable(custom_metric_space custom_metric_space.cpp)
target_link_libraries(custom_metric_space PRIVATE ${METRIC_TARGET_NAME})

add_executable(explicit_space_representations explicit_space_representations.cpp)
target_link_libraries(explicit_space_representations PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_space_entropy metric_space_entropy.cpp)
target_link_libraries(metric_space_entropy PRIVATE ${METRIC_TARGET_NAME})

add_executable(metric_space_correlation metric_space_correlation.cpp)
target_link_libraries(metric_space_correlation PRIVATE ${METRIC_TARGET_NAME})

add_executable(time_series_twed_space time_series_twed_space.cpp)
target_link_libraries(time_series_twed_space PRIVATE ${METRIC_TARGET_NAME})

add_executable(histogram_emd_space histogram_emd_space.cpp)
target_link_libraries(histogram_emd_space PRIVATE ${METRIC_TARGET_NAME})

if (BUILD_TESTING)
    add_test(NAME example_metric_space_strings COMMAND metric_space_strings)
    add_test(NAME example_custom_metric_space COMMAND custom_metric_space)
    add_test(NAME example_explicit_space_representations COMMAND explicit_space_representations)
    add_test(NAME example_metric_space_entropy COMMAND metric_space_entropy)
    add_test(NAME example_metric_space_correlation COMMAND metric_space_correlation)
    add_test(NAME example_time_series_twed_space COMMAND time_series_twed_space)
    add_test(NAME example_histogram_emd_space COMMAND histogram_emd_space)
endif ()
