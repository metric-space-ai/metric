find_package (Boost COMPONENTS date_time REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/3dparty/
    ${Boost_INCLUDE_DIRS}
    )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -Wall -Wextra -Werror -g -std=c++14 -Ofast")
set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -Wall -Wextra -Werror -std=c++14")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer")

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Werror")

link_directories(${PROJECT_SOURCE_DIR}/3dparty/benchmark/)
file(GLOB BENCH_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
#AUX_SOURCE_DIRECTORY(. TEST_SRCS)
#Run through each source
foreach(benchSrc ${BENCH_SRCS})
	message("test=${benchSrc}")
  #Extract the filename without an extension (NAME_WE)
  get_filename_component(benchName ${benchSrc} NAME_WE)

  #Add compile target
  add_executable(${benchName} ${benchSrc})

  if(benchName STREQUAL "thread")
    set(CROSS_FILTER_MULTI_THREAD "ON")
    if(NOT CROSS_FILTER_USE_THREAD_POOL)
      set(CROSS_FILTER_USE_THREAD_POOL "ON")
    endif(NOT CROSS_FILTER_USE_THREAD_POOL)
  endif(benchName STREQUAL "thread")

  if(CROSS_FILTER_MULTI_THREAD STREQUAL "ON")
    target_compile_definitions(${benchName} PRIVATE CROSS_FILTER_MULTI_THREAD)
    target_compile_options(${benchName} PRIVATE -pthread)
    set_target_properties(${benchName} PROPERTIES LINK_FLAGS -pthread)

    if(CROSS_FILTER_USE_THREAD_POOL STREQUAL "ON")
      target_compile_definitions(${benchName} PRIVATE CROSS_FILTER_USE_THREAD_POOL)

    endif(CROSS_FILTER_USE_THREAD_POOL STREQUAL "ON")

  else(CROSS_FILTER_MULTI_THREAD STREQUAL "ON")
    target_compile_definitions(${benchName} PRIVATE CROSS_FILTER_SINGE_THREAD)
  endif(CROSS_FILTER_MULTI_THREAD STREQUAL "ON")

  #link to Boost libraries AND your targets and dependencies
  target_link_libraries(${benchName} ${Boost_LIBRARIES})

  #I like to move testing binaries into a testBin directory
  set_target_properties(${benchName} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/benchBin)
  #	set_property(TARGET ${testName} PROPERTY CXX_STANDARD 17)

endforeach(benchSrc)
