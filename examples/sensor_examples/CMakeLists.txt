find_package(PostgreSQL REQUIRED)

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
	
	
#file(COPY "db_creditinals.csv" DESTINATION ".") 

set(EXAMPLE_NAME SensorExample)
add_executable(${EXAMPLE_NAME} SensorExample.cpp)
if(UNIX)
    target_compile_options(${EXAMPLE_NAME} PRIVATE -pthread)
    set_target_properties(${EXAMPLE_NAME} PROPERTIES LINK_FLAGS -pthread)
endif(UNIX)

target_include_directories(${EXAMPLE_NAME} PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(${EXAMPLE_NAME} PRIVATE ${PostgreSQL_LIBRARIES})
set_target_properties(${EXAMPLE_NAME} PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

# Copy PostgreSQL .dll files on bin folder if we are on Windows
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # It prints C:\Program Files\PostgreSQL\9.4\lib
  message (STATUS "paths are ${PostgreSQL_LIBRARY_DIRS}")
  # It prints  C:/Program Files/PostgreSQL/9.4/lib/libpq.lib
  message (STATUS "libraries are ${PostgreSQL_LIBRARIES}")
  add_custom_command (TARGET ${EXAMPLE_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PostgreSQL_LIBRARY_DIRS}/libpq.dll $<TARGET_FILE_DIR:${EXAMPLE_NAME}>)
  # I need to copy also dll's needed to libpq.dll
endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")





