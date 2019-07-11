set(BOOST_TEST_LIB ${Boost_LIBRARIES})
message(${BOOST_TEST_LIB})
find_package (Boost COMPONENTS serialization system REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    )
message("Boost_test_lib=${BOOST_TEST_LIB}")
message("Boost_ser=${Boost_LIBRARIES}")
AUX_SOURCE_DIRECTORY(. TEST_SRCS)
#Run through each source
foreach(testSrc ${TEST_SRCS})

        #Extract the filename without an extension (NAME_WE)
        get_filename_component(testName ${testSrc} NAME_WE)

        #Add compile target
        add_executable(${testName} ${testSrc})

        #link to Boost libraries AND your targets and dependencies
	if(WIN32)
		target_compile_options(${testName} PRIVATE /D BOOST_ALL_NO_LIB=1)
#		target_compile_options(${testName} PRIVATE /DBOOST_USE_WINDOWS_H)
	endif()
        target_link_libraries(${testName} ${Boost_LIBRARIES})
        target_link_libraries(${testName} ${BOOST_TEST_LIB})
        #I like to move testing binaries into a testBin directory
#        set_target_properties(${testName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR})
        #Finally add it to test execution - 
        #Notice the WORKING_DIRECTORY and COMMAND
        add_test(NAME ${testName} 
                 COMMAND ${testName} )
endforeach(testSrc)
