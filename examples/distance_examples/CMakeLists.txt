file(COPY "assets" DESTINATION ".")

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp) 

#Run through each source
foreach(exampleSrc ${EXAMPLE_SRCS})

	#Extract the filename without an extension (NAME_WE)
	get_filename_component(exampleName ${exampleSrc} NAME_WE)
	if( ${exampleName} STREQUAL "earth_mover_distance_2_example")
	    continue()
	endif()
	#Add compile target
	add_executable(${exampleName} ${exampleSrc}) 
 
	#link to Boost libraries AND your targets and dependencies 
	target_link_libraries(${exampleName} metric)

endforeach(exampleSrc)