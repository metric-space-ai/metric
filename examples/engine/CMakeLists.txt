add_executable(engine_strings_edit_space strings_edit_space.cpp)
target_link_libraries(engine_strings_edit_space PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_process_curves_space process_curves_space.cpp)
target_link_libraries(engine_process_curves_space PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_vector_space_special_case vector_space_special_case.cpp)
target_link_libraries(engine_vector_space_special_case PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_histogram_transport_space histogram_transport_space.cpp)
target_link_libraries(engine_histogram_transport_space PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_cross_space_mgc cross_space_mgc.cpp)
target_link_libraries(engine_cross_space_mgc PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_representation_swap representation_swap.cpp)
target_link_libraries(engine_representation_swap PRIVATE ${METRIC_TARGET_NAME})

add_executable(engine_phate_autoencoder_map phate_autoencoder_map.cpp)
target_link_libraries(engine_phate_autoencoder_map PRIVATE ${METRIC_TARGET_NAME})

if (BUILD_TESTING)
    add_test(NAME example_engine_strings_edit_space COMMAND engine_strings_edit_space)
    add_test(NAME example_engine_process_curves_space COMMAND engine_process_curves_space)
    add_test(NAME example_engine_vector_space_special_case COMMAND engine_vector_space_special_case)
    add_test(NAME example_engine_histogram_transport_space COMMAND engine_histogram_transport_space)
    add_test(NAME example_engine_cross_space_mgc COMMAND engine_cross_space_mgc)
    add_test(NAME example_engine_representation_swap COMMAND engine_representation_swap)
    add_test(NAME example_engine_phate_autoencoder_map COMMAND engine_phate_autoencoder_map)
endif ()
