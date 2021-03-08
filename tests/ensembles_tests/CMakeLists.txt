add_executable(ensemble_tests ensemble_tests.cpp)

target_link_libraries(ensemble_tests PRIVATE Catch2::Catch2)

catch_discover_tests(ensemble_tests)
