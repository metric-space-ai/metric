cmake_minimum_required(VERSION 3.9)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

project(metric_py DESCRIPTION "metric python binding")

SET(SRC src/metric_py.cpp)

add_library(metric_py SHARED ${SRC})
set_target_properties(metric_py PROPERTIES PREFIX "")
set_target_properties(metric_py PROPERTIES OUTPUT_NAME "metric")
set_target_properties(metric_py PROPERTIES SOVERSION 1)

# rasbian has python3 while arch has python
find_package(Boost COMPONENTS python3)
If (Boost_FOUND)
else ()
find_package(Boost COMPONENTS python REQUIRED)
endif ()

find_package(PkgConfig REQUIRED)
pkg_search_module(PYTHON3 REQUIRED python3)
set_target_properties(metric_py PROPERTIES CXX_STANDARD 17)
target_include_directories(metric_py PRIVATE ${PYTHON3_INCLUDE_DIRS} src ..)

target_link_libraries(metric_py ${Boost_LIBRARIES} ${PYTHON3_LIBRARIES})
set(CMAKE_VERBOSE_MAKEFILE ON)

