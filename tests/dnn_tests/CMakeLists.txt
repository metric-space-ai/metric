# see https://cmake.org/cmake/help/latest/module/FindBoost.html
set (Boost_USE_STATIC_LIBS_OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

find_package(BLAS)

include_directories(${Boost_INCLUDE_DIR})

Include(FetchContent)

# Catch2 installtion
FetchContent_Declare(
		Catch2
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG        v2.13.1)

FetchContent_MakeAvailable(Catch2)


add_executable(dnn_tests dnn_tests.cpp)
target_link_libraries(dnn_tests PRIVATE Catch2::Catch2)

add_executable(dnn_json_tests dnn_json_tests.cpp)
target_link_libraries(dnn_json_tests ${Boost_LIBRARIES})

add_executable(datasets_test datasets_test.cpp)
target_link_libraries(datasets_test ${Boost_LIBRARIES})

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)

include(CTest)
include(Catch)
catch_discover_tests(dnn_tests)

add_test(NAME dnn_json_test COMMAND dnn_json_tests)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/empty ${CMAKE_CURRENT_BINARY_DIR}/empty COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wrong-shape ${CMAKE_CURRENT_BINARY_DIR}/wrong-shape COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image1.pgm ${CMAKE_CURRENT_BINARY_DIR}/image1.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image2.pgm ${CMAKE_CURRENT_BINARY_DIR}/image2.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image3.pgm ${CMAKE_CURRENT_BINARY_DIR}/image3.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/image.pgm ${CMAKE_CURRENT_BINARY_DIR}/image.pgm COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/images-list ${CMAKE_CURRENT_BINARY_DIR}/images-list COPYONLY)
