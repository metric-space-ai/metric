find_package(BLAS)

add_executable(lapack_tests lapack_tests.cpp)
target_link_libraries(lapack_tests Catch2::Catch2 ${LAPACK_LIBRARIES})

catch_discover_tests(lapack_tests)
