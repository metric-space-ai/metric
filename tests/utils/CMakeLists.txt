add_executable(dsv_tests dsv_tests.cpp)
add_executable(ups_tests ups_tests.cpp)

target_link_libraries(dsv_tests Catch2::Catch2)
target_link_libraries(ups_tests Catch2::Catch2)

catch_discover_tests(dsv_tests)
catch_discover_tests(ups_tests)
