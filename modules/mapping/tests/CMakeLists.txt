cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

project(main)

enable_testing()

find_package(Boost REQUIRED unit_test_framework)



add_executable(main main.cpp )
add_executable(unit-tests unit-tests.cpp)

target_include_directories(unit-tests PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(unit-tests ${Boost_LIBRARIES})

add_test(NAME unit-tests COMMAND unit-tests)

