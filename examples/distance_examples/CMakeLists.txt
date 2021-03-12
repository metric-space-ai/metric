include_directories(
    ${PROJECT_SOURCE_DIR}/../../3rdparty/jpeglib
    ) 
	 
file(COPY "assets" DESTINATION ".")

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp) 
find_package(JPEG)

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
	target_link_libraries(${exampleName} ${LAPACK_LIBRARIES})
	if(UNIX)
		target_link_libraries(${exampleName} ${JPEG_LIBRARIES})
	endif(UNIX)
	
endforeach(exampleSrc)