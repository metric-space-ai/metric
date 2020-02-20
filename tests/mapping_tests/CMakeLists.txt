cmake_minimum_required(VERSION 3.10)

enable_testing()

set(Boost_DEBUG on)  
find_package(Boost)
find_package(LAPACK)

project(mapping_tests)

include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/../..)	

# for Visual Studio
if(MSVC) 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

set(BOOST_LINK_DIRS "${BOOST_ROOT}/stage/lib")

# mapping tests

add_executable(mapping_tests mapping_tests.cpp)

target_include_directories(mapping_tests PRIVATE ${Boost_INCLUDE_DIRS})
target_link_directories(mapping_tests PRIVATE ${BOOST_LINK_DIRS})
target_link_libraries(mapping_tests ${Boost_LIBRARIES})

if(UNIX)
	set_target_properties(mapping_tests PROPERTIES CXX_STANDARD 17)
endif(UNIX)

add_test(NAME mapping_tests COMMAND mapping_tests)

