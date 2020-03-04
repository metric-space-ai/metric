cmake_minimum_required(VERSION 3.10)

project(ensemble_examples)

set(Boost_DEBUG on)  
find_package(Boost)

if(UNIX)
    find_package(Threads REQUIRED)
endif(UNIX)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

file(COPY "assets" DESTINATION ".")

include_directories(${Boost_INCLUDE_DIRS}) 

find_package(LAPACK)

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp) 

#Run through each source
foreach(exampleSrc ${EXAMPLE_SRCS})

	#Extract the filename without an extension (NAME_WE)
	get_filename_component(exampleName ${exampleSrc} NAME_WE)

	#Add compile target
	add_executable(${exampleName} ${exampleSrc})
 
	#link to Boost libraries AND your targets and dependencies
	if(CMAKE_SYSTEM_NAME MATCHES Windows)
		target_link_directories(${exampleName} PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
		target_link_libraries(${exampleName} PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	endif()
	target_link_libraries(${exampleName} ${Boost_LIBRARIES})
	target_link_libraries(${exampleName} ${LAPACK_LIBRARIES})
	set_target_properties(${exampleName} PROPERTIES CXX_STANDARD 17)	
	
	if(UNIX)
		set(THREADS_PREFER_PTHREAD_FLAG ON)
		set(CMAKE_CXX_FLAGS "-O3")

		target_link_libraries(${exampleName} Threads::Threads)

		target_compile_options(${exampleName} PRIVATE -pthread)
		set_target_properties(${exampleName} PROPERTIES LINK_FLAGS -pthread)
	endif(UNIX)

endforeach(exampleSrc)