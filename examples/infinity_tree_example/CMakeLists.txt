cmake_minimum_required(VERSION 3.12)

project(infinity_tree_example_project)

set(Boost_DEBUG on)  
find_package(Boost)

if(UNIX)
    find_package(Threads REQUIRED)
endif(UNIX)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../../3rdparty/jpeglib 
    )

include_directories(${Boost_INCLUDE_DIRS})
	
file(COPY "assets" DESTINATION ".")

add_executable(infinity_tree_example infinity_tree_example.cpp)
 
target_link_libraries(infinity_tree_example ${Boost_LIBRARIES})
set_target_properties(infinity_tree_example PROPERTIES CXX_STANDARD 17)

if(UNIX)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	set(CMAKE_CXX_FLAGS "-O3")

	target_link_libraries(infinity_tree_example Threads::Threads)

	target_compile_options(infinity_tree_example PRIVATE -pthread)
	set_target_properties(infinity_tree_example PROPERTIES LINK_FLAGS -pthread)
	target_link_libraries(infinity_tree_example /usr/local/lib/libjpeg.a) 
endif(UNIX)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	target_link_libraries(infinity_tree_example ${PROJECT_SOURCE_DIR}/../../3rdparty/jpeglib/Release/jpeg.lib) 
endif()


