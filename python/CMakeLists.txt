cmake_minimum_required(VERSION 3.9)

set(CMAKE_BUILD_TYPE Debug)

#set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-DMETRIC_DISABLE_INLINE_CPP -g")
set(CMAKE_CXX_FLAGS_RELEASE "-DMETRIC_DISABLE_INLINE_CPP -O3")

project(metric_py DESCRIPTION "metric python binding")

SET(SRC 
	src/metric_py.cpp
    src/metric_converters.cpp
    src/metric_types.cpp
	src/metric_KOC.cpp
	src/metric_sorensen.cpp
	src/metric_Edit.cpp
	src/metric_EMD.cpp
	src/metric_Entropy.cpp
	src/metric_MGC.cpp
	src/metric_standards.cpp
	src/metric_SSIM.cpp
	src/metric_TWED.cpp
    src/metric_matrix.cpp
#    src/metric_kohonen_distance.cpp
#    src/metric_dbscan.cpp
    src/metric_DSPCC.cpp
    src/metric_autoencoder.cpp
#    src/metric_wavelet.cpp
#    src/metric_ESN.cpp
    src/metric_affprop.cpp
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

find_package(BLAS)
find_package(LAPACK)
if(LAPACK_FOUND AND BLAS_FOUND)
   set(lapackblas_libraries ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
else ()
find_package(PkgConfig REQUIRED)
# pkg_search_module(PYTHON3 REQUIRED python36)
endif ()
set_target_properties(metric_py PROPERTIES CXX_STANDARD 17)
target_include_directories(metric_py PRIVATE ${PYTHON3_INCLUDE_DIRS} src ..)

target_link_libraries(metric_py ${Boost_LIBRARIES} ${PYTHON3_LIBRARIES} ${lapackblas_libraries})
set(CMAKE_VERBOSE_MAKEFILE ON)

