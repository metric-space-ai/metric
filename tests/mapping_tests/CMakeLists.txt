enable_testing()

add_executable(mapping_tests mapping_tests.cpp)

target_include_directories(mapping_tests PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(mapping_tests ${Boost_LIBRARIES})

add_test(NAME mapping_tests COMMAND mapping_tests)

