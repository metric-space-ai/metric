
include_directories(
    ${PROJECT_SOURCE_DIR}
    )

include_directories(${Boost_INCLUDE_DIRS})

add_executable(distance_example distance_example.cpp)
set_target_properties(distance_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")
add_executable(StandartMetrics_example StandartMetrics_example.cpp)
set_target_properties(StandartMetrics_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")
add_executable(EMD_example EMD_example.cpp)
set_target_properties(EMD_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")
add_executable(TWED_example TWED_example.cpp)
set_target_properties(TWED_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")
add_executable(SSIM_example SSIM_example.cpp)
set_target_properties(SSIM_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")
add_executable(EditDistance_example EditDistance_example.cpp)
set_target_properties(EditDistance_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

add_executable(entropy_example entropy_example.cpp)
set_target_properties(entropy_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")
if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(entropy_example Threads::Threads)
endif(UNIX)
target_link_libraries(entropy_example ${Boost_LIBRARIES})

