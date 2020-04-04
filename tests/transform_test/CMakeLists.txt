set (Boost_USE_STATIC_LIBS_OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)


include_directories(${Boost_INCLUDE_DIR})
add_executable(hog_tests hog_tests.cpp)
target_link_libraries(hog_tests ${Boost_LIBRARIES})


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/astronaut.pgm ${CMAKE_CURRENT_BINARY_DIR}/astronaut.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/images-list ${CMAKE_CURRENT_BINARY_DIR}/images-list COPYONLY)

#add_test(NAME hog_test
#		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/hog_tests
#		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
