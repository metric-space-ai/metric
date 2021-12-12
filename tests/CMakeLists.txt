add_executable(unit_tests unit-tests.cpp)
target_link_libraries(unit_tests PRIVATE metric)

set(test_bin_dir ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(correlation_tests)
add_subdirectory(dnn_tests)
add_subdirectory(ensembles_tests)
add_subdirectory(mapping_tests)
add_subdirectory(space_tests)
add_subdirectory(sparsification_tests)
add_subdirectory(transform_test)
add_subdirectory(utils)
add_subdirectory(wrapper_tests)
add_subdirectory(distance_tests)
add_subdirectory(image_processing_tests)


target_link_libraries(unit_tests PRIVATE Catch2::Catch2)

# Code coverage
option(CODE_COVERAGE "Enable coverage report (only for debug build type)" OFF)

if (CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  if (CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_options(unit_tests PRIVATE --coverage)
    target_link_options(unit_tests PRIVATE --coverage)
    message(STATUS "Turn on code coverage")
  else (CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(WARNING "Code coverage only works with Debug build type. Skipping")
  endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif (CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")


# Discover tests
catch_discover_tests(unit_tests)
