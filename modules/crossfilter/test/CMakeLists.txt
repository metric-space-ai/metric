
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    )


AUX_SOURCE_DIRECTORY(. TEST_SRCS)
#Run through each source
foreach(testSrc ${TEST_SRCS})
        #Extract the filename without an extension (NAME_WE)
        get_filename_component(testName ${testSrc} NAME_WE)

        #Add compile target
        add_executable(${testName} ${testSrc})

        if(UNIX)
        	target_compile_options(${testName} PRIVATE -pthread)
		set_target_properties(${testName} PROPERTIES LINK_FLAGS -pthread)
	endif(UNIX)
	if(MSVC)
		target_compile_options(${testName} PRIVATE /bigobj)
	endif()
        #link to Boost libraries AND your targets and dependencies
        target_link_libraries(${testName} ${Boost_LIBRARIES})

        #Finally add it to test execution - 
        #Notice the WORKING_DIRECTORY and COMMAND
        add_test(NAME ${testName} 
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
                 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${testName} )
endforeach(testSrc)
