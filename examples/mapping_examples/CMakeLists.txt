cmake_minimum_required(VERSION 3.12)

set(Boost_DEBUG on)  
find_package(Boost)

project(MappingExamples)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	include_directories(
		${PROJECT_SOURCE_DIR}
#		${PROJECT_SOURCE_DIR}/assets
#		${PROJECT_SOURCE_DIR}/../../..
#		${PROJECT_SOURCE_DIR}/../../../3rdparty
#		${PROJECT_SOURCE_DIR}/../../../mkl/include
	)
else()	
	include_directories(
		${PROJECT_SOURCE_DIR}
#		${PROJECT_SOURCE_DIR}/assets
#		${PROJECT_SOURCE_DIR}/../../..
#		${PROJECT_SOURCE_DIR}/../../../3rdparty
	)
endif()
	
file(COPY "assets" DESTINATION ".")

include_directories(${Boost_INCLUDE_DIRS})
	
# for Visual Studio
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)


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
if(UNIX)
    target_compile_options(C45_example PRIVATE -pthread)
endif(UNIX)
add_executable(Boosting_example quantized_mappers/classification/Boosting_example.cpp)
add_executable(Bagging_example quantized_mappers/classification/Bagging_example.cpp)
add_executable(CWA_demo quantized_mappers/classification/CWA_demo.cpp)
add_executable(DesicionTree_example quantized_mappers/classification/DesicionTree_example.cpp)


if(LAPACK_LIBRARIES)
	# continious_mappers / clustering
	add_executable(SOM_example continious_mappers/clustering/SOM_example.cpp)
	target_link_libraries(SOM_example PRIVATE ${LAPACK_LIBRARIES})
	
	add_executable(main_encoder main_encoder.cpp)
	target_link_libraries(main_encoder PRIVATE ${LAPACK_LIBRARIES})
	
	add_executable(main_esn main_esn.cpp)
	target_link_libraries(main_esn PRIVATE ${LAPACK_LIBRARIES})
endif(LAPACK_LIBRARIES)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	# continious_mappers / clustering
	add_executable(SOM_example continious_mappers/clustering/SOM_example.cpp)
	target_link_directories(SOM_example PRIVATE ${PROJECT_SOURCE_DIR}/mkl/lib/intel64_win)
	target_link_libraries(SOM_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(SOM_example PRIVATE ${Boost_LIBRARIES})
endif()

# continious_mappers / classification









