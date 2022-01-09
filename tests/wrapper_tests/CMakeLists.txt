find_package(BLAS)

target_sources(unit_tests PRIVATE lapack_tests.cpp)