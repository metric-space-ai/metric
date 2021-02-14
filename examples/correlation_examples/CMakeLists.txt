cmake_minimum_required(VERSION 3.10)

project(correlation_examples)

include_directories(
    ${PROJECT_SOURCE_DIR}
    )

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

file(COPY "assets" DESTINATION ".")

#Run through each source
foreach(exampleSrc ${EXAMPLE_SRCS})

	#Extract the filename without an extension (NAME_WE)
	get_filename_component(exampleName ${exampleSrc} NAME_WE)

	#Add compile target
	add_executable(${exampleName} ${exampleSrc})
 
	target_link_libraries(${exampleName} ${LAPACK_LIBRARIES})
	set_target_properties(${exampleName} PROPERTIES CXX_STANDARD 17)

endforeach(exampleSrc)