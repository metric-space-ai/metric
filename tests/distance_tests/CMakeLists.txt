cmake_minimum_required(VERSION 3.10)
set(BOOST_TEST_LIB ${Boost_LIBRARIES})

enable_testing()

set(Boost_DEBUG on)  
find_package(Boost)

project(distance_tests)

include_directories(
	${PROJECT_SOURCE_DIR} 
	${PROJECT_SOURCE_DIR}/../..
    ${Boost_INCLUDE_DIRS}
	)	

# for Visual Studio
if(MSVC) 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")  
endif(MSVC)

set(BOOST_LINK_DIRS "${BOOST_ROOT}/stage/lib")

# file(COPY "assets" DESTINATION ".") 

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
		target_link_directories(${testName} PRIVATE ${BOOST_LINK_DIRS})
		target_link_libraries(${testName} ${Boost_LIBRARIES})
        target_link_libraries(${testName} ${BOOST_TEST_LIB})
		set_target_properties(${testName} PROPERTIES CXX_STANDARD 17)
	if(UNIX)
		set(CMAKE_CXX_FLAGS "-O3")

		target_link_libraries(${exampleName} Threads::Threads)

		target_compile_options(${exampleName} PRIVATE -pthread)
		set_target_properties(${exampleName} PROPERTIES LINK_FLAGS -pthread)
	endif(UNIX)
        #I like to move testing binaries into a testBin directory
#        set_target_properties(${testName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR})
        #Finally add it to test execution - 
        #Notice the WORKING_DIRECTORY and COMMAND
        add_test(NAME ${testName} COMMAND ${testName})

endforeach(testSrc)
