cmake_minimum_required(VERSION 3.10)
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()

project(cppcrossfilter)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/3dparty/)
include_directories(${PROJECT_SOURCE_DIR}/3dparty/nod-0.4.1/include)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    include(CodeCoverage)
    SET(CMAKE_CXX_FLAGS "-std=c++14 -Ofast")
    SET(CMAKE_C_FLAGS "-std=c++14 -Ofast")
    # SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -std=c++14 -Ofast")
    # SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -std=c++14 -Ofast")

    setup_target_for_coverage(NAME ${PROJECT_NAME}_coverage EXECUTABLE ctest )

endif() #CMAKE_BUILD_TYPE STREQUAL "Coverage"

if (SIGNALS_TYPE STREQUAL "nod")
	add_definitions(-DUSE_NOD_SIGNALS)
endif()

add_subdirectory(benchmark)
enable_testing()
add_subdirectory(test)

