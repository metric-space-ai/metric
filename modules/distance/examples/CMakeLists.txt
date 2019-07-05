#find_package (Boost COMPONENTS date_time REQUIRED)

set(Boost_DEBUG on) 
 
find_package(Boost)
 
if(Boost_FOUND) 
    message(STATUS "BOOST found!")
else()
    message(STATUS "Fail to find BOOST!")
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../..
    ${PROJECT_SOURCE_DIR}/metric_distance
    ${PROJECT_SOURCE_DIR}/metric_clustering
    )
#set (GCC_WARNING "-Wall -Wpedantic -Wextra -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wmissing-format-attribute -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Weffc++ -Wno-unused-local-typedefs")
#set (GCC_WARNING "-Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Wno-unused-local-typedefs")
set (GCC_WARNING "-Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer ${GCC_WARNING} -g -std=c++17 -O3 -pthread")
set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer -pthread")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer ${GCC_WARNING} -std=c++17 -pthread")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -pthread")

include_directories(${Boost_INCLUDE_DIRS})

add_executable(simple_example simple_example.cpp)
add_executable(StandartMetrics_example StandartMetrics_example.cpp)
add_executable(EMD_example EMD_example.cpp)
add_executable(TWED_example TWED_example.cpp)
add_executable(SSIM_example SSIM_example.cpp)
add_executable(EditDistance_example EditDistance_example.cpp)
add_executable(entropy_example entropy_example.cpp)
#set_target_properties(simple_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin")

target_link_libraries(entropy_example ${Boost_LIBRARIES})

