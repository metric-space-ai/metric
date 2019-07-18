

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

#Run through each source
foreach(exampleSrc ${EXAMPLE_SRCS})

  #Extract the filename without an extension (NAME_WE)
  get_filename_component(exampleName ${exampleSrc} NAME_WE)

  #Add compile target
  add_executable(${exampleName} ${exampleSrc})
 
  #link to Boost libraries AND your targets and dependencies

#  target_link_libraries(${exampleName} ${LAPACK_LIBRARIES})
  if(UNIX)
    target_compile_options(${exampleName} PRIVATE -pthread)
    set_target_properties(${exampleName} PROPERTIES LINK_FLAGS -pthread)
  endif(UNIX)

endforeach(exampleSrc)
