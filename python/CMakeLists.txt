cmake_minimum_required(VERSION 3.10)

if (POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif ()
if (POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

# TODO
if(CMAKE_SYSTEM_NAME MATCHES Windows)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES ".*64$")
        message("Build architecture: x64")
        set(CMAKE_GENERATOR_PLATFORM x64)
    else()
        message("Build architecture: x86")
        set(CMAKE_GENERATOR_PLATFORM Win32)
    endif()
endif()

# TODO: check compiler, not OS
if (WIN32)
    set(PYBIND11_CPP_STANDARD /std:c++17)
else ()
    set(PYBIND11_CPP_STANDARD -std=c++1z)
endif ()

project(metric_py DESCRIPTION "metric python binding")

# we need this workaround as wheel copies package directory to temp folder and metric/modules is not one level up anymore
SET(SOURCE_PATH ${CMAKE_SOURCE_DIR}/src)
if(NOT DEFINED ENV{METRIC_SOURCE_PATH})
    SET(METRIC_SOURCE_PATH ${CMAKE_SOURCE_DIR}/..)
else ()
    SET(METRIC_SOURCE_PATH $ENV{METRIC_SOURCE_PATH})
endif()

find_package(Python3 COMPONENTS Interpreter Development)
find_package(Boost REQUIRED)
find_package(BLAS REQUIRED)

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
    ${BLAS_LIBRARIES}
)

if (MSVC)
    target_compile_options(base_python_module INTERFACE
        /wd"4267" /wd"4305" /wd"4244" /wd"4996"
    )
endif ()

add_subdirectory(pybind11)
add_subdirectory(src)
