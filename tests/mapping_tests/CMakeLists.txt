target_sources(unit_tests PRIVATE kohonen_outliers_clustering_tests.cpp)
target_sources(unit_tests PRIVATE mapping_tests.cpp)
target_sources(unit_tests PRIVATE dspcc_tests.cpp)

target_link_libraries(unit_tests PRIVATE ${LAPACK_LIBRARIES})