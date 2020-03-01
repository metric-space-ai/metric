set (Boost_USE_STATIC_LIBS_OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)


include_directories(${Boost_INCLUDE_DIR})
add_executable(hog_tests hog_tests.cpp)
target_link_libraries(hog_tests ${Boost_LIBRARIES})


#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image.pgm ${CMAKE_CURRENT_BINARY_DIR}/image.pgm COPYONLY)
