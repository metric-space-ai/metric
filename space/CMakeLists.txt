cmake_minimum_required(VERSION 3.10)
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()

project(metric_search)
include_directories(${PROJECT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(examples)
enable_testing()
add_subdirectory(tests)

