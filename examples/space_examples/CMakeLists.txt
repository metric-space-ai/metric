find_package(Threads REQUIRED)

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

#Run through each source
foreach(exampleSrc ${EXAMPLE_SRCS})

  #Extract the filename without an extension (NAME_WE)
  get_filename_component(exampleName ${exampleSrc} NAME_WE) 

  #Add compile target
  add_executable(${exampleName} ${exampleSrc}) 
  
  target_link_libraries(${exampleName} metric Threads::Threads)
endforeach(exampleSrc)
