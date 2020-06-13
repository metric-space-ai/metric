# see https://cmake.org/cmake/help/latest/module/FindBoost.html
set (Boost_USE_STATIC_LIBS_OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

include_directories(${Boost_INCLUDE_DIR})

add_executable(convolution_tests convolution_tests.cpp)
target_link_libraries(convolution_tests ${Boost_LIBRARIES})

add_test(NAME convolution_test COMMAND convolution_tests)
