find_package(Threads)

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

file(COPY "assets" DESTINATION ".")

#Run through each source
foreach(exampleSrc ${EXAMPLE_SRCS})

	#Extract the filename without an extension (NAME_WE)
	get_filename_component(exampleName ${exampleSrc} NAME_WE)

	#Add compile target
	add_executable(${exampleName} ${exampleSrc}) 
 
	target_link_libraries(${exampleName} ${METRIC_TARGET_NAME})

	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
	    target_link_libraries(${exampleName} stdc++fs)
	endif()
endforeach(exampleSrc)