cmake_minimum_required(VERSION 3.10)
set(CMAKE_VERBOSE_MAKEFILE ON)

#set(CMAKE_BUILD_TYPE Debug)

#set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(PYBIND11_CPP_STANDARD -std=c++1z)

project(metric_py DESCRIPTION "metric python binding")

SET(SOURCE_PATH ${CMAKE_SOURCE_DIR}/src)
SET(METRIC_SOURCE_PATH ${CMAKE_SOURCE_DIR}/..)

find_package(Python3 COMPONENTS Interpreter Development)

set(Boost_DEBUG on)
find_package(Boost REQUIRED)

find_package(LAPACK REQUIRED)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
else ()
find_package(PkgConfig REQUIRED)
endif ()

set(COMMON_LIBRARIES
    #${Boost_LIBRARIES}
    ${LAPACK_LIBRARIES}
)

SET(HEADER_INCLUDE_PATH
    ${Boost_INCLUDE_DIR}
    ${SOURCE_PATH}
    ${METRIC_SOURCE_PATH}
)

include_directories(${HEADER_INCLUDE_PATH})

add_subdirectory(pybind11)
add_subdirectory(src)