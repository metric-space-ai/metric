find_package (Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address -Wall -Wextra -Werror -g -std=c++14 -Ofast")
set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address -g -std=c++14 -Ofast")


#file(GLOB TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*.cpp)
AUX_SOURCE_DIRECTORY(. TEST_SRCS)
#Run through each source
foreach(testSrc ${TEST_SRCS})
	message("test=${testSrc}")
        #Extract the filename without an extension (NAME_WE)
        get_filename_component(testName ${testSrc} NAME_WE)

        #Add compile target
        add_executable(${testName} ${testSrc})

        if(CROSS_FILTER_MULTI_THREAD STREQUAL "ON")
          target_compile_definitions(${testName} PUBLIC CROSS_FILTER_MULTI_THREAD)
          target_compile_options(${testName} PRIVATE -pthread)
          set_target_properties(${testName} PROPERTIES LINK_FLAGS -pthread)

          if(CROSS_FILTER_USE_THREAD_POOL STREQUAL "ON")
            target_compile_definitions(${testName} PUBLIC CROSS_FILTER_USE_THREAD_POOL)
          endif(CROSS_FILTER_USE_THREAD_POOL STREQUAL "ON")

        else(CROSS_FILTER_MULTI_THREAD STREQUAL "ON")
          target_compile_definitions(${testName} PUBLIC CROSS_FILTER_SINGE_THREAD)
        endif(CROSS_FILTER_MULTI_THREAD STREQUAL "ON")

        #link to Boost libraries AND your targets and dependencies
        target_link_libraries(${testName} ${Boost_LIBRARIES})

        #I like to move testing binaries into a testBin directory
        set_target_properties(${testName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/testBin)
          #	set_property(TARGET ${testName} PROPERTY CXX_STANDARD 17)
        #Finally add it to test execution - 
        #Notice the WORKING_DIRECTORY and COMMAND
        add_test(NAME ${testName} 
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testBin 
                 COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/testBin/${testName} )
endforeach(testSrc)
