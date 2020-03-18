cmake_minimum_required(VERSION 3.10)

project(distance_examples)

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

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp) 

#Run through each source
foreach(exampleSrc ${EXAMPLE_SRCS})

	#Extract the filename without an extension (NAME_WE)
	get_filename_component(exampleName ${exampleSrc} NAME_WE)

	#Add compile target
	add_executable(${exampleName} ${exampleSrc})
 
	#link to Boost libraries AND your targets and dependencies 
	target_link_libraries(${exampleName} ${Boost_LIBRARIES})
	set_target_properties(${exampleName} PROPERTIES CXX_STANDARD 17)

#  target_link_libraries(${exampleName} ${LAPACK_LIBRARIES})
	if(UNIX) 
		set(THREADS_PREFER_PTHREAD_FLAG ON)
		set(CMAKE_CXX_FLAGS "-O3")

		target_link_libraries(${exampleName} Threads::Threads)

		target_compile_options(${exampleName} PRIVATE -pthread)
		set_target_properties(${exampleName} PROPERTIES LINK_FLAGS -pthread) 
	endif(UNIX) 

endforeach(exampleSrc)

if(UNIX)
    FIND_PACKAGE(JPEG REQUIRED)
	target_link_libraries(earth_mover_distance_example ${JPEG_LIBRARY})
	target_link_libraries(earth_mover_distance_2_example ${JPEG_LIBRARY})
endif(UNIX)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	target_link_libraries(earth_mover_distance_example ${PROJECT_SOURCE_DIR}/../../3rdparty/jpeglib/Release/jpeg.lib) 
	target_link_libraries(earth_mover_distance_2_example ${PROJECT_SOURCE_DIR}/../../3rdparty/jpeglib/Release/jpeg.lib)
endif()