cmake_minimum_required(VERSION 3.13)

if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()
if (POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

project(panda_metric)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# CCache
#set(METRIC_USE_CCACHE ON)

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


find_package(LAPACK REQUIRED)


add_library(metric INTERFACE)
target_include_directories(metric INTERFACE ${PROJECT_SOURCE_DIR})
target_include_directories(metric INTERFACE ${PROJECT_SOURCE_DIR}/3rdparty)
target_link_libraries(metric INTERFACE LAPACK::LAPACK)


# Catch2 installation
Include(FetchContent)

FetchContent_Declare(
		Catch2
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG        v2.13.4)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)


enable_testing()
include(CTest)
include(Catch)


add_subdirectory(tests)
add_subdirectory(benchmarks)
add_subdirectory(examples)
