find_package(Boost)


include_directories(
    ${PROJECT_SOURCE_DIR}/../../3rdparty/jpeglib
    ) 
	 
include_directories(${Boost_INCLUDE_DIRS})  
	
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
	target_link_libraries(${exampleName} ${Boost_LIBRARIES} ${LAPACK_LIBRARIES})
	if(UNIX)
		target_link_libraries(${exampleName} ${JPEG_LIBRARIES})
	endif(UNIX)
	
endforeach(exampleSrc)

if(Boost_VERSION GREATER_EQUAL 107200)
    add_executable (earth_mover_distance_2_example earth_mover_distance_2_example.cpp)
    if(CMAKE_SYSTEM_NAME MATCHES Windows)
		# target_link_libraries(earth_mover_distance_2_example ${PROJECT_SOURCE_DIR}/../../3rdparty/jpeglib/Release/jpeg.lib)   
    else()
	if(UNIX)
	    target_link_libraries(earth_mover_distance_2_example ${JPEG_LIBRARIES})
	endif()
    endif()
endif()

