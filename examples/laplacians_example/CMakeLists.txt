add_executable(Laplacians_test Laplacians_test.cpp sparsification_test.cpp tests.cpp)
set_target_properties(Laplacians_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/examples)
target_link_libraries(Laplacians_test PRIVATE ${LAPACK_LIBRARIES})
