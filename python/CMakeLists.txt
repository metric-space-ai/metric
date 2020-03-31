cmake_minimum_required(VERSION 3.10)
set(CMAKE_VERBOSE_MAKEFILE ON)

#set(CMAKE_BUILD_TYPE Debug)

#set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(PYBIND11_CPP_STANDARD -std=c++1z)

project(metric_py DESCRIPTION "metric python binding")

SET(SOURCE_PATH ${CMAKE_SOURCE_DIR}/src)
if(NOT DEFINED ENV{METRIC_SOURCE_PATH})
    SET(METRIC_SOURCE_PATH ${CMAKE_SOURCE_DIR}/..)
else ()
    SET(METRIC_SOURCE_PATH $ENV{METRIC_SOURCE_PATH})
endif()

find_package(Python3 COMPONENTS Interpreter Development)

find_package(Boost REQUIRED)

find_package(LAPACK REQUIRED)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
else ()
find_package(PkgConfig REQUIRED)
endif ()

add_library(base_python_module INTERFACE)
target_include_directories(base_python_module INTERFACE
    ${Boost_INCLUDE_DIR}
    ${SOURCE_PATH}
    ${METRIC_SOURCE_PATH}
)
target_link_libraries(base_python_module INTERFACE
    ${LAPACK_LIBRARIES}
)

add_subdirectory(pybind11)
add_subdirectory(src)