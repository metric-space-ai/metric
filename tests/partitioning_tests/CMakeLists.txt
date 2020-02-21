add_executable(partitioning_tests partitioning_tests.cpp)
target_include_directories(partitioning_tests PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(partitioning_tests ${Boost_LIBRARIES})

add_test(NAME partitioning_tests
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
         COMMAND ${CMAKE_CURRENT_BINARY_DIR}/partitioning_tests )
