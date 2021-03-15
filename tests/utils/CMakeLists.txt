add_executable(dsv_tests dsv_tests.cpp)
target_link_libraries(dsv_tests Catch2::Catch2)
catch_discover_tests(dsv_tests)
