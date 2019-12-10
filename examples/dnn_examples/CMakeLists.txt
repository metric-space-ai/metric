cmake_minimum_required(VERSION 3.13)
project(dnn_example)

add_executable(dnn_example dnn_example.cpp)
add_executable(dnn_example_pgm dnn_example_pgm.cpp)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data.cereal ${CMAKE_CURRENT_BINARY_DIR}/data.cereal COPYONLY)

