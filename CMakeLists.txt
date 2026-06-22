cmake_minimum_required(VERSION 3.19)

#if (POLICY CMP0025)
#  cmake_policy(SET CMP0025 NEW)
#endif ()
#if (POLICY CMP0074)
#    cmake_policy(SET CMP0074 NEW)
#endif()
#cmake_policy(SET CMP0076 NEW)

project(metric VERSION 0.3.4 LANGUAGES CXX)

# CCache
# set(METRIC_USE_CCACHE ON)

find_program(CCACHE_PATH ccache)
if (CCACHE_PATH)
    message(STATUS "CCache found in ${CCACHE_PATH}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif (CCACHE_PATH)


set(_metric_build_tests_default OFF)
if (DEFINED BUILD_TESTS AND BUILD_TESTS)
    message(DEPRECATION "BUILD_TESTS is deprecated; use METRIC_BUILD_TESTS instead.")
    set(_metric_build_tests_default ${BUILD_TESTS})
endif ()

option(METRIC_BUILD_TESTS "Build the full C++ test suite" ${_metric_build_tests_default})
option(METRIC_BUILD_CORE_TESTS "Build the minimal core smoke tests" ON)
option(METRIC_BUILD_CORE_EXAMPLES "Build the minimal core examples" ON)
option(METRIC_BUILD_EXAMPLES "Build C++ examples" OFF)
option(METRIC_BUILD_BENCHMARKS "Build C++ benchmarks" OFF)
option(METRIC_BUILD_PYTHON "Build Python bindings from the top-level CMake project" OFF)
option(METRIC_BUILD_LAPACK_CORE "Build LAPACK-dependent core smoke targets when LAPACK is available" ON)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
    if (CMAKE_SYSTEM_PROCESSOR MATCHES ".*64$")
        message("Build architecture: x64")
        set(CMAKE_GENERATOR_PLATFORM x64)
    else ()
        message("Build architecture: x86")
        set(CMAKE_GENERATOR_PLATFORM Win32)
    endif ()
endif ()


include(FetchContent)
include(GNUInstallDirs)

# LAPACK
find_package(LAPACK QUIET)
set(METRIC_BUILD_CORE_ENTROPY OFF)
if (METRIC_BUILD_LAPACK_CORE AND LAPACK_FOUND)
    set(METRIC_BUILD_CORE_ENTROPY ON)
elseif (METRIC_BUILD_LAPACK_CORE)
    message(STATUS "LAPACK not found; skipping LAPACK-dependent entropy core targets")
endif ()

set(METRIC_TARGET_NAME ${PROJECT_NAME})

# Create metric target
add_library(${METRIC_TARGET_NAME} INTERFACE)

target_compile_features(${METRIC_TARGET_NAME} INTERFACE cxx_std_17)

# Include paths
target_include_directories(${METRIC_TARGET_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Catch2 installation
if (METRIC_BUILD_TESTS OR METRIC_BUILD_BENCHMARKS)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v2.13.7)

    FetchContent_MakeAvailable(Catch2)

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)

    include(Catch)
endif ()

if (METRIC_BUILD_CORE_TESTS OR METRIC_BUILD_TESTS)
    enable_testing()
    include(CTest)
endif ()

if (METRIC_BUILD_CORE_TESTS)
    message(STATUS "Core smoke tests enabled")
    add_subdirectory(tests/core_smoke)
endif ()

if (METRIC_BUILD_CORE_EXAMPLES)
    message(STATUS "Core examples enabled")
    add_subdirectory(examples/core)
    add_subdirectory(examples/engine)
    add_subdirectory(examples/templates)
endif ()

if (METRIC_BUILD_TESTS)
    message(STATUS "Unit tests enabled")
    add_subdirectory(tests)
endif ()

if (METRIC_BUILD_BENCHMARKS)
    message(STATUS "Benchmarks enabled")
    add_subdirectory(benchmarks)
endif ()

if (METRIC_BUILD_EXAMPLES)
    message(STATUS "Examples enabled")
    add_subdirectory(examples)
endif ()

if (METRIC_BUILD_PYTHON)
    message(STATUS "Python bindings enabled")
    add_subdirectory(python)
endif ()

# Install rules

# Create package targets file
install(TARGETS ${METRIC_TARGET_NAME} EXPORT metric-target)
install(EXPORT metric-target
        FILE ${METRIC_TARGET_NAME}-targets.cmake
        NAMESPACE ${METRIC_TARGET_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

include(CMakePackageConfigHelpers)

# Create package config file
configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${METRIC_TARGET_NAME}-config.cmake.in"
        "${METRIC_TARGET_NAME}-config.cmake"
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
)
# Create package version file
write_basic_package_version_file(${METRIC_TARGET_NAME}-config-version.cmake COMPATIBILITY ExactVersion)

# Install cmake files
install(FILES "${PROJECT_BINARY_DIR}/${METRIC_TARGET_NAME}-config.cmake"
        "${PROJECT_BINARY_DIR}/${METRIC_TARGET_NAME}-config-version.cmake"
        DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

# Install headers
install(DIRECTORY ${PROJECT_SOURCE_DIR}/metric TYPE INCLUDE
        PATTERN "*.md" EXCLUDE
        PATTERN "3rdparty" EXCLUDE)

# Ship the numeric-core provenance/ownership note alongside the installed headers so the
# retained historical-source license (LICENSE.BSD-3-Clause, installed by the rule above) does
# not appear in the installed tree without its METRIC-owned framing.
install(FILES ${PROJECT_SOURCE_DIR}/metric/numeric/README.md
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/metric/numeric)
