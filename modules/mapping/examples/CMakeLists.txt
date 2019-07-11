
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

# quantized_mappers / clustering
add_executable(KMedoids_example quantized_mappers/clustering/KMedoids_example.cpp)
add_executable(KMeans_example quantized_mappers/clustering/KMeans_example.cpp)
add_executable(DBScan_example quantized_mappers/clustering/DBScan_example.cpp)
add_executable(AffinityPropagation_example quantized_mappers/clustering/AffinityPropagation_example.cpp)

#FIXME HierarchicalClustering_example don't compile
#add_executable(HierarchicalClustering_example quantized_mappers/clustering/HierarchicalClustering_example.cpp) 

# quantized_mappers / constraint

# quantized_mappers / classification
add_executable(Classification_example quantized_mappers/classification/Classification_example.cpp)
add_executable(SVM_example quantized_mappers/classification/SVM_example.cpp)
add_executable(C45_example quantized_mappers/classification/C45_example.cpp)
add_executable(Boosting_example quantized_mappers/classification/Boosting_example.cpp)
add_executable(Bagging_example quantized_mappers/classification/Bagging_example.cpp)
add_executable(CWA_demo quantized_mappers/classification/CWA_demo.cpp)
add_executable(MetricDesicionTree_example quantized_mappers/classification/MetricDesicionTree_example.cpp)


if(LAPACK_LIBRARIES)
	# continious_mappers / clustering
	add_executable(SOM_example continious_mappers/clustering/SOM_example.cpp)
	target_link_libraries(SOM_example PRIVATE ${LAPACK_LIBRARIES})
endif(LAPACK_LIBRARIES)

# continious_mappers / classification






