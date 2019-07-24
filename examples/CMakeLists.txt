cmake_minimum_required(VERSION 3.12)

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
)
	
# for Visual Studio
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)
	
file(COPY "db_creditinals.csv" DESTINATION ".") 

include_directories(${Boost_INCLUDE_DIRS})

set(EXAMPLE_NAME SensorExample)
add_executable(${EXAMPLE_NAME} SensorExample.cpp)

if(UNIX)
    target_compile_options(${EXAMPLE_NAME} PRIVATE -pthread)
    set_target_properties(${EXAMPLE_NAME} PROPERTIES LINK_FLAGS -pthread)

	set_target_properties(${EXAMPLE_NAME} PROPERTIES CXX_STANDARD 17)
endif(UNIX)

target_include_directories(${EXAMPLE_NAME} PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(${EXAMPLE_NAME} PRIVATE ${PostgreSQL_LIBRARIES})

target_link_libraries(${EXAMPLE_NAME} PRIVATE ${Boost_LIBRARIES})

# Copy PostgreSQL .dll files on bin folder if we are on Windows
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # It prints C:\Program Files\PostgreSQL\9.4\lib
  message (STATUS "paths are ${PostgreSQL_LIBRARY_DIRS}")
  # It prints  C:/Program Files/PostgreSQL/9.4/lib/libpq.lib
  message (STATUS "libraries are ${PostgreSQL_LIBRARIES}")
  add_custom_command (TARGET ${EXAMPLE_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PostgreSQL_LIBRARY_DIRS}/libpq.dll $<TARGET_FILE_DIR:${EXAMPLE_NAME}>)
  # I need to copy also dll's needed to libpq.dll
endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	target_link_directories(${EXAMPLE_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../mkl/lib/intel64_win)
	target_link_libraries(${EXAMPLE_NAME} PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
endif()

if(LAPACK_LIBRARIES)
    message("LAPACK exists")
	target_link_libraries(${EXAMPLE_NAME} PRIVATE ${LAPACK_LIBRARIES})
else()
    message("LAPACK does not exists")
endif(LAPACK_LIBRARIES)





