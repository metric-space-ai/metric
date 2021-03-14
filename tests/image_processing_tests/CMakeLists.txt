add_executable(convolution_tests convolution_tests.cpp)
target_link_libraries(convolution_tests Catch2::Catch2)

catch_discover_tests(convolution_tests)

