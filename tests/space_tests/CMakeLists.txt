cmake_minimum_required(VERSION 3.10)

enable_testing()

set(BOOST_TEST_LIB ${Boost_LIBRARIES})
# message(${BOOST_TEST_LIB})
find_package (Boost COMPONENTS serialization system REQUIRED)
find_package(LAPACK)

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
		#target_compile_options(${testName} PRIVATE /D BOOST_ALL_NO_LIB=1)
#		target_compile_options(${testName} PRIVATE /DBOOST_USE_WINDOWS_H)
	endif()
		target_include_directories(${testName} PRIVATE ${Boost_INCLUDE_DIRS})
		target_link_directories(${testName} PRIVATE ${BOOST_LINK_DIRS})
        target_link_libraries(${testName} PRIVATE ${Boost_LIBRARIES})
        target_link_libraries(${testName} PRIVATE ${BOOST_TEST_LIB})

	if(LAPACK_LIBRARIES)
		target_link_libraries(${testName} PRIVATE ${LAPACK_LIBRARIES})
	endif(LAPACK_LIBRARIES)
 
	if(CMAKE_SYSTEM_NAME MATCHES Windows)
		target_link_directories(${testName} PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
		target_link_libraries(${testName} PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
		target_link_libraries(${testName} PRIVATE ${Boost_LIBRARIES})
	endif()

	if(UNIX)
	    target_link_libraries(${testName} Threads::Threads)
	set_target_properties(mapping_tests PROPERTIES CXX_STANDARD 17)
	endif(UNIX)

        #I like to move testing binaries into a testBin directory 
#        set_target_properties(${testName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR})
        #Finally add it to test execution - 
        #Notice the WORKING_DIRECTORY and COMMAND
        add_test(NAME ${testName} 
                 COMMAND ${testName} )
endforeach(testSrc)
