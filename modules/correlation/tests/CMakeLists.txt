
add_executable(unit-tests unit-tests.cpp)
target_include_directories(unit-tests PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(unit-tests ${Boost_LIBRARIES})

#FIXME: fix Connected_components_Test
#add_executable(Connected_components_Test Connected_Components_Test.cpp)
#target_include_directories(Connected_components_Test PRIVATE ${Boost_INCLUDE_DIRS})
#target_link_libraries(Connected_components_Test ${Boost_LIBRARIES})

