cmake_minimum_required(VERSION 3.12)

project(SensorExample)

find_package(PostgreSQL REQUIRED)
set(Boost_DEBUG on) 
 
find_package(Boost)

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
    ${PROJECT_SOURCE_DIR}/..
    ${PROJECT_SOURCE_DIR}/../modules
    )
	

#set (GCC_WARNING "-Wall -Wpedantic -Wextra -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wmissing-format-attribute -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Weffc++ -Wno-unused-local-typedefs")
#set (GCC_WARNING "-Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Wno-unused-local-typedefs")
set (GCC_WARNING "-Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer ${GCC_WARNING} -g -std=c++17 -O3 -pthread")
set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer -pthread")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer ${GCC_WARNING} -std=c++17 -pthread")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -pthread")


# for Visual Studio
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} SensorExample.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${PostgreSQL_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${PostgreSQL_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})

# Copy PostgreSQL .dll files on bin folder if we are on Windows
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # It prints C:\Program Files\PostgreSQL\9.4\lib
  message (STATUS "paths are ${PostgreSQL_LIBRARY_DIRS}")
  # It prints  C:/Program Files/PostgreSQL/9.4/lib/libpq.lib
  message (STATUS "libraries are ${PostgreSQL_LIBRARIES}")
  add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PostgreSQL_LIBRARY_DIRS}/libpq.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>)
  # I need to copy also dll's needed to libpq.dll
endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")





