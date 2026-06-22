cmake_minimum_required(VERSION 3.15)

if (POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif ()
if (POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()
if (POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif ()
if (POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif ()

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
project(metric_py DESCRIPTION "metric python binding" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(FetchContent)
option(METRIC_PYTHON_BUILD_FULL "Build legacy mapping and correlation Python bindings that require Boost" OFF)
option(METRIC_PYTHON_FETCH_DEPS "Fetch missing C++ header dependencies for Python bindings" ON)
option(METRIC_PYTHON_USE_BLAS "Link Python bindings to BLAS when available" ON)

# Keep paths relative to this file so the Python build works both standalone and
# when included by the top-level METRIC project.
SET(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(NOT DEFINED ENV{METRIC_SOURCE_PATH})
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/metric_cpp/metric")
        SET(METRIC_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/metric_cpp)
    else ()
        SET(METRIC_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
    endif ()
else ()
    SET(METRIC_SOURCE_PATH $ENV{METRIC_SOURCE_PATH})
endif()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.18")
    find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
else ()
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
endif ()
if (METRIC_PYTHON_USE_BLAS)
    find_package(BLAS QUIET)
endif ()

find_package(nlohmann_json QUIET)
if (NOT nlohmann_json_FOUND AND METRIC_PYTHON_FETCH_DEPS)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE)
    FetchContent_GetProperties(nlohmann_json)
    if (NOT nlohmann_json_POPULATED)
        FetchContent_Populate(nlohmann_json)
    endif ()
    set(METRIC_PYTHON_NLOHMANN_JSON_INCLUDE_DIR ${nlohmann_json_SOURCE_DIR}/include)
endif ()

find_package(cereal QUIET)
if (NOT cereal_FOUND AND METRIC_PYTHON_FETCH_DEPS)
    FetchContent_Declare(
        cereal
        GIT_REPOSITORY https://github.com/USCiLab/cereal.git
        GIT_TAG v1.3.2
        GIT_SHALLOW TRUE)
    FetchContent_GetProperties(cereal)
    if (NOT cereal_POPULATED)
        FetchContent_Populate(cereal)
    endif ()
    set(METRIC_PYTHON_CEREAL_INCLUDE_DIR ${cereal_SOURCE_DIR}/include)
endif ()

if (METRIC_PYTHON_BUILD_FULL)
    find_package(Boost REQUIRED)
endif ()

add_library(base_python_module INTERFACE)
target_include_directories(base_python_module INTERFACE
    ${SOURCE_PATH}
    ${METRIC_SOURCE_PATH}
)
if (METRIC_PYTHON_NLOHMANN_JSON_INCLUDE_DIR)
    target_include_directories(base_python_module INTERFACE ${METRIC_PYTHON_NLOHMANN_JSON_INCLUDE_DIR})
endif ()
if (TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(base_python_module INTERFACE nlohmann_json::nlohmann_json)
endif ()
if (METRIC_PYTHON_CEREAL_INCLUDE_DIR)
    target_include_directories(base_python_module INTERFACE ${METRIC_PYTHON_CEREAL_INCLUDE_DIR})
endif ()
if (TARGET cereal::cereal)
    target_link_libraries(base_python_module INTERFACE cereal::cereal)
endif ()
if (Boost_FOUND)
    target_include_directories(base_python_module INTERFACE ${Boost_INCLUDE_DIR})
endif ()

if (BLAS_FOUND)
    target_link_libraries(base_python_module INTERFACE ${BLAS_LIBRARIES})
endif ()

if (MSVC)
    target_compile_options(base_python_module INTERFACE
        /wd"4267" /wd"4305" /wd"4244" /wd"4996"
    )
endif ()

find_package(pybind11 CONFIG QUIET)
if (NOT pybind11_FOUND)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v3.0.1
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(pybind11)
endif ()
add_subdirectory(src)
