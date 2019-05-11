#find_package (Boost COMPONENTS date_time REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/metric_distance
    ${PROJECT_SOURCE_DIR}/metric_clustering
    )
#set (GCC_WARNING "-Wall -Wpedantic -Wextra -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wmissing-format-attribute -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Weffc++ -Wno-unused-local-typedefs")
set(GCC_WARNING "-Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Wno-unused-local-typedefs")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer ${GCC_WARNING} -g -std=c++17 -O3 -pthread")
set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer -pthread -fsanitize=address")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer ${GCC_WARNING} -std=c++17 -pthread -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -pthread -fsanitize=address")




add_executable(simple_example simple_example.cpp)
#set_target_properties(simple_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin")

