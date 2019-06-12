cmake_minimum_required(VERSION 3.14)

project(CrossfilterExample)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES ".*64$")
        message("Build architecture: x64")
        set(CMAKE_GENERATOR_PLATFORM x64)
    else()
        message("Build architecture: x86")
        set(CMAKE_GENERATOR_PLATFORM Win32)
    endif()
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}
    )
#set (GCC_WARNING "-Wall -Wpedantic -Wextra -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wmissing-format-attribute -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Weffc++ -Wno-unused-local-typedefs")
#set (GCC_WARNING "-Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Wno-unused-local-typedefs")

#set (CMAKE_C_COMPILER /usr/bin/gcc-8)
#set (CMAKE_CXX_COMPILER /usr/bin/g++-8)
set (GCC_WARNING "-Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer ${GCC_WARNING} -g -std=c++17 -O3 -pthread")
set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer -pthread")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer ${GCC_WARNING} -std=c++17 -pthread")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -pthread")

# for Visual Studio
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

add_executable(${PROJECT_NAME} Crossfilter_example.cpp)

