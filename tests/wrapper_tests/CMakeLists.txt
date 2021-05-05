find_package(BLAS)

target_sources(unit_tests PRIVATE lapack_tests.cpp)
target_link_libraries(unit_tests PRIVATE ${LAPACK_LIBRARIES})