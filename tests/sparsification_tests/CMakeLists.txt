add_executable(sparsification_tests sparsification_tests.cpp)
target_include_directories(sparsification_tests PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(sparsification_tests ${Boost_LIBRARIES})

add_test(NAME sparsification_tests
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
         COMMAND ${CMAKE_CURRENT_BINARY_DIR}/sparsification_tests )
