cmake_minimum_required(VERSION 3.9)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-DMETRIC_DISABLE_INLINE_CPP -g")
set(CMAKE_CXX_FLAGS_RELEASE "-DMETRIC_DISABLE_INLINE_CPP -O3")

project(metric_py DESCRIPTION "metric python binding")

SET(SRC 
	src/metric_py.cpp
    src/numpy_array_converter.cpp
    src/numpy_scalar_converter.cpp
    src/metric_types.cpp
    src/metric_MGC.cpp
    src/metric_Edit.cpp
    src/metric_Entropy.cpp
    src/metric_VOI_kl.cpp
    src/metric_VOI_normalized.cpp
    src/metric_EMD.cpp
	src/metric_EMD_details.cpp
)

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

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
else ()
find_package(PkgConfig REQUIRED)
pkg_search_module(PYTHON3 REQUIRED python3)
endif ()
set_target_properties(metric_py PROPERTIES CXX_STANDARD 17)
target_include_directories(metric_py PRIVATE ${PYTHON3_INCLUDE_DIRS} src ..)

target_link_libraries(metric_py ${Boost_LIBRARIES} ${PYTHON3_LIBRARIES})
set(CMAKE_VERBOSE_MAKEFILE ON)

