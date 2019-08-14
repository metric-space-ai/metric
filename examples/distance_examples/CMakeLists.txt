cmake_minimum_required(VERSION 3.12)

set(Boost_DEBUG on)  
find_package(Boost)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../../3rdparty/jpeglib 
    )

include_directories(${Boost_INCLUDE_DIRS})
	
file(COPY "assets" DESTINATION ".")

add_executable(distance_example distance_example.cpp)
add_executable(StandartMetrics_example StandartMetrics_example.cpp)
add_executable(EMD_example EMD_example.cpp)
add_executable(TWED_example TWED_example.cpp)
add_executable(SSIM_example SSIM_example.cpp)
add_executable(EditDistance_example EditDistance_example.cpp)

add_executable(entropy_example entropy_example.cpp)
if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(entropy_example Threads::Threads)
endif(UNIX)
#set_target_properties(simple_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin") 

target_link_libraries(entropy_example ${Boost_LIBRARIES})
target_link_libraries(EMD_example ${Boost_LIBRARIES})

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	target_link_libraries(EMD_example ${PROJECT_SOURCE_DIR}/../../3rdparty/jpeglib/Release/jpeg.lib) 
endif()

