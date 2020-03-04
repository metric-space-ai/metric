cmake_minimum_required(VERSION 3.10)
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()
if (POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

project(panda_metric)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if(UNIX) 
#    append(CMAKE_CXX_FLAGS 
#endif(UNIX)
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

find_package(LAPACK) 
if(NOT LAPACK_FOUND)
	set(BLA_VENDOR Intel10_64lp_seq)
	message("Generic LAPACK not Found, trying Intel MKL")
	if(NOT BLAS_ROOT)
	     if(WIN32)
		     set(BLAS_ROOT "C:/Program\ Files\ (x86)/IntelSWTools/compilers_and_libraries/windows/mkl/lib/intel64")
	     endif()
	endif()
	find_package(LAPACK QUIET)
endif()


if(MSVC)
add_definitions("/w")
endif()
find_package(Boost REQUIRED unit_test_framework)
include_directories(
    ${Boost_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}
)

enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)
