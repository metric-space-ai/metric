set(_metric_template_targets
    template_time_series_condition_monitoring
    template_histogram_transport
    template_mixed_records
    template_cross_space_dependency
    template_representative_reduction
    template_derived_coordinate_space)

add_executable(template_time_series_condition_monitoring time_series_condition_monitoring.cpp)
add_executable(template_histogram_transport histogram_transport.cpp)
add_executable(template_mixed_records mixed_records.cpp)
add_executable(template_cross_space_dependency cross_space_dependency.cpp)
add_executable(template_representative_reduction representative_reduction.cpp)
add_executable(template_derived_coordinate_space derived_coordinate_space.cpp)

foreach (_target IN LISTS _metric_template_targets)
    target_link_libraries(${_target} PRIVATE ${METRIC_TARGET_NAME})
    target_compile_options(${_target} PRIVATE -UNDEBUG)
endforeach ()

if (LAPACK_FOUND)
    foreach (_target IN LISTS _metric_template_targets)
        target_link_libraries(${_target} PRIVATE ${LAPACK_LIBRARIES})
    endforeach ()
endif ()

if (BUILD_TESTING)
    add_test(NAME template_time_series_condition_monitoring COMMAND template_time_series_condition_monitoring)
    add_test(NAME template_histogram_transport COMMAND template_histogram_transport)
    add_test(NAME template_mixed_records COMMAND template_mixed_records)
    add_test(NAME template_cross_space_dependency COMMAND template_cross_space_dependency)
    add_test(NAME template_representative_reduction COMMAND template_representative_reduction)
    add_test(NAME template_derived_coordinate_space COMMAND template_derived_coordinate_space)
endif ()
