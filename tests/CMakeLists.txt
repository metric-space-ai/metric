add_executable(unit_tests unit-tests.cpp)

set(test_bin_dir ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(correlation_tests)
add_subdirectory(dnn_tests)
add_subdirectory(ensembles_tests)
add_subdirectory(mapping_tests)
add_subdirectory(space_tests)
add_subdirectory(sparsification_tests)
#add_subdirectory(transform_test) bye bye error
add_subdirectory(utils)
add_subdirectory(wrapper_tests)
add_subdirectory(distance_tests)
add_subdirectory(image_processing_tests)


target_link_libraries(unit_tests PRIVATE Catch2::Catch2)

catch_discover_tests(unit_tests)
