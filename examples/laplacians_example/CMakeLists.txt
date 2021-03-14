add_executable(Laplacians_test Laplacians_test.cpp sparsification_test.cpp tests.cpp)
target_link_libraries(Laplacians_test PRIVATE ${LAPACK_LIBRARIES})