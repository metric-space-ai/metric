find_package(LAPACK)

target_sources(unit_tests PRIVATE cramervon_mises_tests.cpp)
target_sources(unit_tests PRIVATE entropy_vmixing_tests.cpp)
target_sources(unit_tests PRIVATE kohonen_distance_tests.cpp)
target_sources(unit_tests PRIVATE kolmogorov_smirnov_tests.cpp)
target_sources(unit_tests PRIVATE random_emd_tests.cpp)
target_sources(unit_tests PRIVATE reimannian_test.cpp)

target_link_libraries(unit_tests PRIVATE ${LAPACK_LIBRARIES})