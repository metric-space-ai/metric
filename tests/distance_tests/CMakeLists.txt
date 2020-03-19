set(BOOST_TEST_LIB ${Boost_LIBRARIES})


if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif(UNIX)

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
	endif()
        target_link_libraries(${testName} ${BOOST_TEST_LIB})
	if(UNIX)
	    target_link_libraries(${testName} Threads::Threads)
	    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
	    set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
	endif(UNIX)
        add_test(NAME ${testName} 
                 COMMAND ${testName} )
endforeach(testSrc)
