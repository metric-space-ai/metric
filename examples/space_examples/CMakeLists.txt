cmake_minimum_required(VERSION 3.10)

project(space_examples)

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)


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
  
 
  set_target_properties(${exampleName} PROPERTIES CXX_STANDARD 17)
 
  if(UNIX)
    target_compile_options(${exampleName} PRIVATE -pthread)
    target_link_libraries(${exampleName} ${LAPACK_LIBRARIES})
    set_target_properties(${exampleName} PROPERTIES LINK_FLAGS -pthread) 
  endif(UNIX)

endforeach(exampleSrc)
