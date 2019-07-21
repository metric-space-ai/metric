
include_directories(
    ${PROJECT_SOURCE_DIR}
    )

# for Visual Studio
#if(MSVC)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
#endif(MSVC)

add_executable(Crossfilter_example Crossfilter_example.cpp)

