cmake_minimum_required(VERSION 3.10)
project(dnn_example)

find_package(BLAS)

add_executable(dnn_example dnn_example.cpp)
# broken -  error: ‘Softmax’ was not declared in this scope; did you mean ‘fmax’?
# add_executable(dnn_example_classification_fc dnn_example_classification.cpp)
add_executable(dnn_example_pgm dnn_example_pgm.cpp)

target_link_libraries(dnn_example_pgm ${BLAS_LIBRARIES})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data.cereal ${CMAKE_CURRENT_BINARY_DIR}/data.cereal COPYONLY)

