#find_package (Boost COMPONENTS date_time REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}
#    ${Boost_INCLUDE_DIRS}
    )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -Wall -Wextra -g -std=c++14 -Ofast -pthread")
set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer -pthread")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -Wall -Wextra -std=c++14 -pthread")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -pthread")

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Werror")

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
#AUX_SOURCE_DIRECTORY(. TEST_SRCS)
#Run through each source
foreach(exampleSrc ${EXAMPLE_SRCS})
	message("test=${exampleSrc}")
  #Extract the filename without an extension (NAME_WE)
  get_filename_component(exampleName ${exampleSrc} NAME_WE)

  #Add compile target
  add_executable(${exampleName} ${exampleSrc})

  #link to Boost libraries AND your targets and dependencies
#  target_link_libraries(${benchName} ${Boost_LIBRARIES})

  #I like to move testing binaries into a testBin directory
  set_target_properties(${exampleName} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR})
  #	set_property(TARGET ${testName} PROPERTY CXX_STANDARD 17)

endforeach(exampleSrc)
