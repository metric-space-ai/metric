find_package (Boost COMPONENTS date_time REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/3dparty/
    ${Boost_INCLUDE_DIRS}
    )



file(GLOB BENCH_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

#Run through each source
foreach(benchSrc ${BENCH_SRCS})
  #Extract the filename without an extension (NAME_WE)
  get_filename_component(benchName ${benchSrc} NAME_WE)

  #Add compile target
  add_executable(${benchName} ${benchSrc})

  if(UNIX)
    target_compile_definitions(${benchName} PRIVATE CROSS_FILTER_MULTI_THREAD)
    target_compile_options(${benchName} PRIVATE -pthread)
    set_target_properties(${benchName} PROPERTIES LINK_FLAGS -pthread)
  endif(UNIX)

  #link to Boost libraries AND your targets and dependencies
  target_link_libraries(${benchName} ${Boost_LIBRARIES})

endforeach(benchSrc)
