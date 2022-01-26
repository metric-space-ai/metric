cmake_minimum_required(VERSION 3.14)

#if (POLICY CMP0025)
#  cmake_policy(SET CMP0025 NEW)
#endif ()
#if (POLICY CMP0074)
#    cmake_policy(SET CMP0074 NEW)
#endif()
#cmake_policy(SET CMP0076 NEW)

project(panda_metric)

# CCache
# set(METRIC_USE_CCACHE ON)

find_program(CCACHE_PATH ccache)
if(CCACHE_PATH)
	message(STATUS "CCache found in ${CCACHE_PATH}")
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif(CCACHE_PATH)


option(BUILD_TESTS "build tests" ON)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES ".*64$")
        message("Build architecture: x64")
        set(CMAKE_GENERATOR_PLATFORM x64)
    else()
        message("Build architecture: x86")
        set(CMAKE_GENERATOR_PLATFORM Win32)
    endif()
endif()


# Dependencies
include(FetchContent)

# LAPACK
find_package(LAPACK REQUIRED)

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


# Create metric target
add_library(metric INTERFACE)

target_compile_features(metric INTERFACE cxx_std_17)

# Include paths
target_include_directories(metric INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Link libraries
target_link_libraries(metric INTERFACE ${LAPACK_LIBRARIES})
target_link_libraries(metric INTERFACE nlohmann_json::nlohmann_json)
target_link_libraries(metric INTERFACE blaze::blaze)


# Catch2 installation
FetchContent_Declare(
		Catch2
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG        v2.13.7)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)


enable_testing()
include(CTest)
include(Catch)


add_subdirectory(tests)
add_subdirectory(benchmarks)
add_subdirectory(examples)


# Install rules
include(GNUInstallDirs)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/metric TYPE INCLUDE
        PATTERN "*.md" EXCLUDE)
