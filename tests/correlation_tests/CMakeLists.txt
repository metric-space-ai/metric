add_executable(correlation_tests correlation_tests.cpp)
target_link_libraries(correlation_tests PRIVATE Catch2::Catch2)

catch_discover_tests(correlation_tests)

#target_link_libraries(correlation_tests ${Boost_LIBRARIES} ${LAPACK_LIBRARIES})

#FIXME: fix Connected_components_Test
#add_executable(Connected_components_Test Connected_Components_Test.cpp)
#target_include_directories(Connected_components_Test PRIVATE ${Boost_INCLUDE_DIRS})
#target_link_libraries(Connected_components_Test ${Boost_LIBRARIES})