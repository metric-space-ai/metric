file(COPY "assets" DESTINATION ".")

# for Visual Studio
#if(MSVC)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
#endif(MSVC)

add_executable(Correlation_example Correlation_example.cpp)
add_executable(main main.cpp)





