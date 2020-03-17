set (Boost_USE_STATIC_LIBS_OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

include_directories(${Boost_INCLUDE_DIR})
add_executable(dsv_tests dsv_tests.cpp)
target_link_libraries(dsv_tests ${Boost_LIBRARIES})
