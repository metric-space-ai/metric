find_package(LAPACK)

add_executable(cramervon_mises_tests cramervon_mises_tests.cpp)
add_executable(entropy_vmixing_tests entropy_vmixing_tests.cpp)
add_executable(kohonen_distance_tests kohonen_distance_tests.cpp)
add_executable(kolmogorov_smirnov_tests kolmogorov_smirnov_tests.cpp)
add_executable(random_emd_tests random_emd_tests.cpp)
add_executable(reimannian_test reimannian_test.cpp)

target_link_libraries(cramervon_mises_tests PRIVATE Catch2::Catch2)
target_link_libraries(entropy_vmixing_tests PRIVATE Catch2::Catch2 ${LAPACK_LIBRARIES})
target_link_libraries(kohonen_distance_tests PRIVATE Catch2::Catch2 ${LAPACK_LIBRARIES})
target_link_libraries(kolmogorov_smirnov_tests PRIVATE Catch2::Catch2)
target_link_libraries(random_emd_tests PRIVATE Catch2::Catch2)
target_link_libraries(reimannian_test PRIVATE Catch2::Catch2 ${LAPACK_LIBRARIES})

catch_discover_tests(cramervon_mises_tests)
catch_discover_tests(entropy_vmixing_tests)
catch_discover_tests(kohonen_distance_tests)
catch_discover_tests(kolmogorov_smirnov_tests)
catch_discover_tests(random_emd_tests)
catch_discover_tests(reimannian_test)

#[[
cmake_minimum_required(VERSION 3.10)

enable_testing()

find_package(Boost)

project(distance_tests)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
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

	target_link_libraries(${testName} PRIVATE ${Boost_LIBRARIES} ${LAPACK_LIBRARIES})
	set_target_properties(${testName} PROPERTIES CXX_STANDARD 17)
 
	#link to Boost libraries AND your targets and dependencies 
	if(CMAKE_SYSTEM_NAME MATCHES Windows)
	    target_link_directories(${testName} PRIVATE ${BOOST_LINK_DIRS} ${Boost_LIBRARY_DIRS})
	
		target_link_directories(${testName} PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
		target_link_libraries(${testName} PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	endif()
	if(UNIX)
		target_compile_options(${testName} PRIVATE -DBOOST_TEST_DYN_LINK)
		target_link_libraries(${testName} PRIVATE Threads::Threads)
	endif(UNIX)
        #I like to move testing binaries into a testBin directory
#        set_target_properties(${testName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR})
        #Finally add it to test execution - 
        #Notice the WORKING_DIRECTORY and COMMAND
        add_test(NAME ${testName} COMMAND ${testName})

endforeach(testSrc)
]]
