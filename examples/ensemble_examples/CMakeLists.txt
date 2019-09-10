cmake_minimum_required(VERSION 3.12)

project(ensemble_examples)

set(Boost_DEBUG on)  
find_package(Boost)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

file(COPY "assets" DESTINATION ".")

include_directories(${Boost_INCLUDE_DIRS}) 

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

endforeach(exampleSrc)