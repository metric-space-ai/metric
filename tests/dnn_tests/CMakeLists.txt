add_executable(dnn_tests dnn_tests.cpp)
target_link_libraries(dnn_tests PRIVATE Catch2::Catch2)

add_executable(dnn_json_tests dnn_json_tests.cpp)
target_link_libraries(dnn_json_tests PRIVATE Catch2::Catch2)

add_executable(datasets_tests datasets_tests.cpp)
target_link_libraries(datasets_tests PRIVATE Catch2::Catch2)

catch_discover_tests(dnn_tests)
catch_discover_tests(dnn_json_tests)
catch_discover_tests(datasets_tests)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/empty ${CMAKE_CURRENT_BINARY_DIR}/empty COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wrong-shape ${CMAKE_CURRENT_BINARY_DIR}/wrong-shape COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image1.pgm ${CMAKE_CURRENT_BINARY_DIR}/image1.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image2.pgm ${CMAKE_CURRENT_BINARY_DIR}/image2.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image3.pgm ${CMAKE_CURRENT_BINARY_DIR}/image3.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image.pgm ${CMAKE_CURRENT_BINARY_DIR}/image.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/images-list ${CMAKE_CURRENT_BINARY_DIR}/images-list COPYONLY)