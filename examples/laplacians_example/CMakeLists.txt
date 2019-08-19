add_executable(Laplacians_example Laplacians_test.cpp sparsification_test.cpp tests.cpp)
set_target_properties(Laplacians_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/examples)




