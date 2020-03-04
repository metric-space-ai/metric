
include_directories( ${PROJECT_SOURCE_DIR} )
add_subdirectory(correlation_tests)
add_subdirectory(dnn_tests)
add_subdirectory(ensembles_tests)
add_subdirectory(mapping_tests)
# add_subdirectory(space_tests)             # broken(class metric::Node<int, distance<int> >’ has no member named ‘data’)
# add_subdirectory(sparsification_tests)    # broken
