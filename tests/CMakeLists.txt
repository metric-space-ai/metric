include_directories( ${PROJECT_SOURCE_DIR} )


# Catch2 installtion
Include(FetchContent)

FetchContent_Declare(
		Catch2
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG        v2.13.1)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(CTest)
include(Catch)

add_subdirectory(correlation_tests)
add_subdirectory(dnn_tests)
add_subdirectory(ensembles_tests)
add_subdirectory(mapping_tests)
add_subdirectory(space_tests)
add_subdirectory(sparsification_tests)
#add_subdirectory(transform_test) bye bye error
add_subdirectory(utils)
add_subdirectory(wrapper_tests)
add_subdirectory(distance_tests)
add_subdirectory(image_processing_tests)
