cmake_minimum_required(VERSION 3.12)

set(Boost_DEBUG on)  
find_package(Boost)

project(space_examples)

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)


include_directories(${Boost_INCLUDE_DIRS})

# for Visual Studio
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)
 
#Run through each source
foreach(exampleSrc ${EXAMPLE_SRCS})

  #Extract the filename without an extension (NAME_WE)
  get_filename_component(exampleName ${exampleSrc} NAME_WE) 

  #Add compile target
  add_executable(${exampleName} ${exampleSrc})
 
  if(UNIX)
    target_compile_options(${exampleName} PRIVATE -pthread)
    set_target_properties(${exampleName} PROPERTIES LINK_FLAGS -pthread) 
  endif(UNIX)

  
if(CMAKE_SYSTEM_NAME MATCHES Windows)

	target_link_libraries(${exampleName} PRIVATE ${Boost_LIBRARIES})
endif()

endforeach(exampleSrc)
