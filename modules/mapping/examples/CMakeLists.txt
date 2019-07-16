cmake_minimum_required(VERSION 3.14)

set(Boost_DEBUG on) 
 
find_package(Boost)
 
if(Boost_FOUND) 
    message(STATUS "BOOST found!")
else()
    message(STATUS "Fail to find BOOST!")
endif()


project(MappingExamples)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES ".*64$")
        message("Build architecture: x64")
        set(CMAKE_GENERATOR_PLATFORM x64)
    else()
        message("Build architecture: x86")
        set(CMAKE_GENERATOR_PLATFORM Win32)
    endif()
endif()

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	include_directories(
		${PROJECT_SOURCE_DIR}
		${PROJECT_SOURCE_DIR}/assets
		${PROJECT_SOURCE_DIR}/../../..
		${PROJECT_SOURCE_DIR}/../../../3rdparty
		${PROJECT_SOURCE_DIR}/../../../mkl/include
	)
else()	
	include_directories(
		${PROJECT_SOURCE_DIR}
		${PROJECT_SOURCE_DIR}/assets
		${PROJECT_SOURCE_DIR}/../../..
		${PROJECT_SOURCE_DIR}/../../../3rdparty
	)
endif()
	
file(COPY "assets" DESTINATION ".")

#set (GCC_WARNING "-Wall -Wpedantic -Wextra -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wmissing-format-attribute -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Weffc++ -Wno-unused-local-typedefs")
#set (GCC_WARNING "-Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wvla -Wsuggest-attribute=const -Wuninitialized -Winit-self -Wdouble-promotion -Wstrict-aliasing -Wno-unused-local-typedefs")
set (GCC_WARNING "-Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer ${GCC_WARNING} -g -std=c++17 -O3 -pthread")
set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer -pthread")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer ${GCC_WARNING} -std=c++17 -pthread")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -pthread")


# for Visual Studio
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)


#find_package(blaze CONFIG REQUIRED)
#find_package(blaze CONFIG REQUIRED)
#if( blaze_FOUND )
#    message("Blaze was found")
#   #add_library( blaze_target INTERFACE )
#   target_link_libraries(main PRIVATE blaze::blaze)
#   #target_link_libraries( blaze_target INTERFACE blaze::blaze )
#else()
#    message("Blaze was not found!")
#endif()

include_directories(${Boost_INCLUDE_DIRS})

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

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	target_link_directories(SOM_example PRIVATE ${PROJECT_SOURCE_DIR}/../../../mkl/lib/intel64_win)
	target_link_libraries(SOM_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
endif()

# continious_mappers / classification






