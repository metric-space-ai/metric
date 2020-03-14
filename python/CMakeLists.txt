cmake_minimum_required(VERSION 3.10)
set(CMAKE_VERBOSE_MAKEFILE ON)

#set(CMAKE_BUILD_TYPE Debug)

#set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

project(metric_py DESCRIPTION "metric python binding")

SET(SOURCE_PATH ${CMAKE_SOURCE_DIR}/src)
SET(METRIC_SOURCE_PATH ${CMAKE_SOURCE_DIR}/..)

find_package(Python3 COMPONENTS Interpreter Development)

# rasbian has python3 while arch has python
find_package(Boost COMPONENTS python3)
If (Boost_FOUND)
else ()
find_package(Boost COMPONENTS python REQUIRED)
endif ()

find_package(BLAS)
find_package(LAPACK)
if(LAPACK_FOUND AND BLAS_FOUND)
   set(lapackblas_libraries ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
else ()
find_package(PkgConfig REQUIRED)
endif ()

set(COMMON_LIBRARIES
    ${Boost_LIBRARIES}
    ${Python3_LIBRARIES}
    ${lapackblas_libraries}
)

SET(HEADER_INCLUDE_PATH
    ${Boost_INCLUDE_DIR}
    ${Python3_INCLUDE_DIR}
    ${SOURCE_PATH}
    ${METRIC_SOURCE_PATH}
)

include_directories(${HEADER_INCLUDE_PATH})

add_subdirectory(src)