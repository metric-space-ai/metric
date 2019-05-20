cmake_minimum_required(VERSION 3.14)
#find_package (Boost COMPONENTS date_time REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/assets
    )
	
file(COPY "assets" DESTINATION ".")

#set (GCC_WARNING "-Wall -Wpedantic -Wextra -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wmissing-format-attribute -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Weffc++ -Wno-unused-local-typedefs")
#set (GCC_WARNING "-Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Wno-unused-local-typedefs")
set (GCC_WARNING "-Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer ${GCC_WARNING} -g -std=c++17 -O3 -pthread")
set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer -pthread -fsanitize=address")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer ${GCC_WARNING} -std=c++17 -pthread -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -pthread -fsanitize=address")

# for Visual Studio
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

# quantized_mappers / clustering
add_executable(KMedoids_example quantized_mappers/clustering/KMedoids_example.cpp)
add_executable(KMeans_example quantized_mappers/clustering/KMeans_example.cpp)
add_executable(DBScan_example quantized_mappers/clustering/DBScan_example.cpp)
add_executable(AffinityPropagation_example quantized_mappers/clustering/AffinityPropagation_example.cpp)
add_executable(HierarchicalClustering_example quantized_mappers/clustering/HierarchicalClustering_example.cpp)

# quantized_mappers / constraint

# quantized_mappers / classification
add_executable(Classification_example quantized_mappers/classification/Classification_example.cpp)
add_executable(SVM_example quantized_mappers/classification/SVM_example.cpp)
add_executable(C45_example quantized_mappers/classification/C45_example.cpp)
add_executable(Boosting_example quantized_mappers/classification/Boosting_example.cpp)
add_executable(Bagging_example quantized_mappers/classification/Bagging_example.cpp)
add_executable(CWA_demo quantized_mappers/classification/CWA_demo.cpp)
add_executable(MetricDesicionTree_example quantized_mappers/classification/MetricDesicionTree_example.cpp)

# continious_mappers / clustering
add_executable(SOM_example continious_mappers/clustering/SOM_example.cpp)

# continious_mappers / classification






