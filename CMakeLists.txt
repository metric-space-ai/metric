cmake_minimum_required(VERSION 3.14)

#if (POLICY CMP0025)
#  cmake_policy(SET CMP0025 NEW)
#endif ()
#if (POLICY CMP0074)
#    cmake_policy(SET CMP0074 NEW)
#endif()
#cmake_policy(SET CMP0076 NEW)

project(panda_metric VERSION 0.2.0)

# CCache
# set(METRIC_USE_CCACHE ON)

find_program(CCACHE_PATH ccache)
if (CCACHE_PATH)
    message(STATUS "CCache found in ${CCACHE_PATH}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif (CCACHE_PATH)


option(BUILD_TESTS "build tests" ON)

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

# LAPACK
find_package(LAPACK)

# Json
#FetchContent_Declare(json
#        GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
#        GIT_TAG v3.7.3)
#FetchContent_GetProperties(json)
#if(NOT json_POPULATED)
#    FetchContent_Populate(json)
#    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
#endif()
find_package(nlohmann_json REQUIRED)

# Blaze
#FetchContent_Declare(blaze
#        GIT_REPOSITORY https://bitbucket.org/blaze-lib/blaze.git
#        GIT_TAG v3.8)
#FetchContent_MakeAvailable(blaze)
#list(APPEND CMAKE_MODULE_PATH ${blaze_SOURCE_DIR}/cmake)
#include(Blaze_Import)
#Blaze_Import(DEFAULT_INITIALIZATION)
find_package(blaze 3.8 REQUIRED)

find_package(cereal REQUIRED)

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
target_link_libraries(${METRIC_TARGET_NAME} INTERFACE nlohmann_json::nlohmann_json)
target_link_libraries(${METRIC_TARGET_NAME} INTERFACE blaze::blaze)
target_link_libraries(${METRIC_TARGET_NAME} INTERFACE cereal:cereal)


# Catch2 installation
if (METRIC_BUILD_TESTS OR METRIC_BUILD_BENCHMARKS)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v2.13.7)

    FetchContent_MakeAvailable(Catch2)

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)

    enable_testing()
    include(CTest)
    include(Catch)
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


# Install rules
include(GNUInstallDirs)

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
