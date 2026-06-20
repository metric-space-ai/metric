cmake_minimum_required(VERSION 3.19)

#if (POLICY CMP0025)
#  cmake_policy(SET CMP0025 NEW)
#endif ()
#if (POLICY CMP0074)
#    cmake_policy(SET CMP0074 NEW)
#endif()
#cmake_policy(SET CMP0076 NEW)

project(panda_metric VERSION 0.3.3 LANGUAGES CXX)

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
option(METRIC_FETCH_DEPS "Fetch missing C++ dependencies with CMake FetchContent" OFF)
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


# Dependencies
include(FetchContent)
include(GNUInstallDirs)

set(METRIC_INSTALL_BUNDLED_NLOHMANN_JSON OFF)
set(METRIC_INSTALL_BUNDLED_BLAZE OFF)
set(METRIC_INSTALL_BUNDLED_CEREAL OFF)

# LAPACK
find_package(LAPACK QUIET)
set(METRIC_BUILD_CORE_ENTROPY OFF)
if (METRIC_BUILD_LAPACK_CORE AND LAPACK_FOUND)
    set(METRIC_BUILD_CORE_ENTROPY ON)
elseif (METRIC_BUILD_LAPACK_CORE)
    message(STATUS "LAPACK not found; skipping LAPACK-dependent entropy core targets")
endif ()

# Json
find_package(nlohmann_json QUIET)
if (NOT nlohmann_json_FOUND)
    if (METRIC_FETCH_DEPS)
        FetchContent_Declare(
                nlohmann_json
                GIT_REPOSITORY https://github.com/nlohmann/json.git
                GIT_TAG v3.11.3
                GIT_SHALLOW TRUE)
        FetchContent_GetProperties(nlohmann_json)
        if (NOT nlohmann_json_POPULATED)
            if (POLICY CMP0169)
                cmake_policy(PUSH)
                cmake_policy(SET CMP0169 OLD)
            endif ()
            FetchContent_Populate(nlohmann_json)
            if (POLICY CMP0169)
                cmake_policy(POP)
            endif ()
        endif ()
        set(METRIC_NLOHMANN_JSON_INCLUDE_DIR ${nlohmann_json_SOURCE_DIR}/include)
        set(METRIC_INSTALL_BUNDLED_NLOHMANN_JSON ON)
    else ()
        message(FATAL_ERROR "nlohmann_json was not found. Install it with your package manager or configure with -DMETRIC_FETCH_DEPS=ON.")
    endif ()
endif ()
if (NOT METRIC_NLOHMANN_JSON_INCLUDE_DIR AND NOT TARGET nlohmann_json::nlohmann_json)
    message(FATAL_ERROR "nlohmann_json was found, but target nlohmann_json::nlohmann_json is unavailable.")
endif ()

# Blaze
find_package(blaze 3.8 QUIET)
if (NOT blaze_FOUND)
    if (METRIC_FETCH_DEPS)
        FetchContent_Declare(
                blaze
                GIT_REPOSITORY https://bitbucket.org/blaze-lib/blaze.git
                GIT_TAG v3.8
                GIT_SHALLOW TRUE)
        FetchContent_GetProperties(blaze)
        if (NOT blaze_POPULATED)
            if (POLICY CMP0169)
                cmake_policy(PUSH)
                cmake_policy(SET CMP0169 OLD)
            endif ()
            FetchContent_Populate(blaze)
            if (POLICY CMP0169)
                cmake_policy(POP)
            endif ()
        endif ()
    else ()
        message(FATAL_ERROR "blaze 3.8 was not found. Install it with your package manager or configure with -DMETRIC_FETCH_DEPS=ON.")
    endif ()
endif ()
set(METRIC_BLAZE_TARGET blaze::blaze)
if (NOT TARGET ${METRIC_BLAZE_TARGET})
    if (DEFINED blaze_SOURCE_DIR)
        add_library(metric_blaze INTERFACE)
        target_include_directories(metric_blaze INTERFACE ${blaze_SOURCE_DIR})
        set(METRIC_BLAZE_INCLUDE_DIR ${blaze_SOURCE_DIR})
        set(METRIC_INSTALL_BUNDLED_BLAZE ON)
        set(METRIC_BLAZE_TARGET metric_blaze)
    elseif (DEFINED BLAZE_INCLUDE_DIRS)
        add_library(metric_blaze INTERFACE)
        target_include_directories(metric_blaze INTERFACE ${BLAZE_INCLUDE_DIRS})
        set(METRIC_BLAZE_TARGET metric_blaze)
    else ()
        message(FATAL_ERROR "blaze was found, but target blaze::blaze is unavailable.")
    endif ()
endif ()

find_package(cereal QUIET)
if (NOT cereal_FOUND)
    if (METRIC_FETCH_DEPS)
        FetchContent_Declare(
                cereal
                GIT_REPOSITORY https://github.com/USCiLab/cereal.git
                GIT_TAG v1.3.2
                GIT_SHALLOW TRUE)
        FetchContent_GetProperties(cereal)
        if (NOT cereal_POPULATED)
            if (POLICY CMP0169)
                cmake_policy(PUSH)
                cmake_policy(SET CMP0169 OLD)
            endif ()
            FetchContent_Populate(cereal)
            if (POLICY CMP0169)
                cmake_policy(POP)
            endif ()
        endif ()
        set(METRIC_CEREAL_INCLUDE_DIR ${cereal_SOURCE_DIR}/include)
        set(METRIC_INSTALL_BUNDLED_CEREAL ON)
    else ()
        message(FATAL_ERROR "cereal was not found. Install it with your package manager or configure with -DMETRIC_FETCH_DEPS=ON.")
    endif ()
endif ()
set(METRIC_CEREAL_TARGET cereal::cereal)
if (NOT METRIC_CEREAL_INCLUDE_DIR AND NOT TARGET ${METRIC_CEREAL_TARGET})
    if (TARGET cereal)
        set(METRIC_CEREAL_TARGET cereal)
    else ()
        message(FATAL_ERROR "cereal was found, but target cereal::cereal is unavailable.")
    endif ()
endif ()

set(METRIC_TARGET_NAME ${PROJECT_NAME})

# Create metric target
add_library(${METRIC_TARGET_NAME} INTERFACE)

target_compile_features(${METRIC_TARGET_NAME} INTERFACE cxx_std_17)

# Include paths
target_include_directories(${METRIC_TARGET_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Link libraries
if (LAPACK_FOUND)
    target_link_libraries(${METRIC_TARGET_NAME} INTERFACE ${LAPACK_LIBRARIES})
endif()
if (METRIC_NLOHMANN_JSON_INCLUDE_DIR)
    target_include_directories(${METRIC_TARGET_NAME} INTERFACE
            $<BUILD_INTERFACE:${METRIC_NLOHMANN_JSON_INCLUDE_DIR}>)
else ()
    target_link_libraries(${METRIC_TARGET_NAME} INTERFACE
            $<BUILD_INTERFACE:nlohmann_json::nlohmann_json>)
endif ()
if (NOT METRIC_INSTALL_BUNDLED_NLOHMANN_JSON)
    target_link_libraries(${METRIC_TARGET_NAME} INTERFACE
            $<INSTALL_INTERFACE:nlohmann_json::nlohmann_json>)
endif ()
target_link_libraries(${METRIC_TARGET_NAME} INTERFACE
        $<BUILD_INTERFACE:${METRIC_BLAZE_TARGET}>)
if (NOT METRIC_INSTALL_BUNDLED_BLAZE)
    target_link_libraries(${METRIC_TARGET_NAME} INTERFACE
            $<INSTALL_INTERFACE:blaze::blaze>)
endif ()
if (METRIC_CEREAL_INCLUDE_DIR)
    target_include_directories(${METRIC_TARGET_NAME} INTERFACE
            $<BUILD_INTERFACE:${METRIC_CEREAL_INCLUDE_DIR}>)
else ()
    target_link_libraries(${METRIC_TARGET_NAME} INTERFACE
            $<BUILD_INTERFACE:${METRIC_CEREAL_TARGET}>)
endif ()
if (NOT METRIC_INSTALL_BUNDLED_CEREAL)
    target_link_libraries(${METRIC_TARGET_NAME} INTERFACE
            $<INSTALL_INTERFACE:cereal::cereal>)
endif ()


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
        PATTERN "*.md" EXCLUDE)
if (METRIC_INSTALL_BUNDLED_NLOHMANN_JSON)
    install(DIRECTORY "${METRIC_NLOHMANN_JSON_INCLUDE_DIR}/nlohmann"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif ()
if (METRIC_INSTALL_BUNDLED_BLAZE)
    install(DIRECTORY "${METRIC_BLAZE_INCLUDE_DIR}/blaze"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif ()
if (METRIC_INSTALL_BUNDLED_CEREAL)
    install(DIRECTORY "${METRIC_CEREAL_INCLUDE_DIR}/cereal"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif ()
