target_sources(unit_tests PRIVATE dnn_tests.cpp)
target_sources(unit_tests PRIVATE dnn_json_tests.cpp)
target_sources(unit_tests PRIVATE datasets_tests.cpp)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/empty ${test_bin_dir}/empty COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wrong-shape ${test_bin_dir}/wrong-shape COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image1.pgm ${test_bin_dir}/image1.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image2.pgm ${test_bin_dir}/image2.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image3.pgm ${test_bin_dir}/image3.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image.pgm ${test_bin_dir}/image.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/images-list ${test_bin_dir}/images-list COPYONLY)