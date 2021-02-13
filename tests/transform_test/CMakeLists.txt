add_executable(dwt_tests dwt_tests.cpp)
target_link_libraries(dwt_tests Catch2::Catch2)
catch_discover_tests(dwt_tests)

#[[
include_directories(${Boost_INCLUDE_DIR})
add_executable(hog_tests hog_tests.cpp)
target_link_libraries(hog_tests ${Boost_LIBRARIES})


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/astronaut.pgm ${CMAKE_CURRENT_BINARY_DIR}/astronaut.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/images-list ${CMAKE_CURRENT_BINARY_DIR}/images-list COPYONLY)

add_test(NAME hog_test
		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/hog_tests
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
]]
