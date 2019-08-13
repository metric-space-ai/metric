
include_directories(
		${PROJECT_SOURCE_DIR}
	)

#file(COPY "assets" DESTINATION ".")


include_directories(${Boost_INCLUDE_DIRS})

# quantized_mappers / clustering
add_executable(KMedoids_example quantized_mappers/clustering/KMedoids_example.cpp)
target_link_libraries(KMedoids_example PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(KMedoids_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

add_executable(KMeans_example quantized_mappers/clustering/KMeans_example.cpp)
target_link_libraries(KMeans_example PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(KMeans_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

add_executable(DBScan_example quantized_mappers/clustering/DBScan_example.cpp)
target_link_libraries(DBScan_example PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(DBScan_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

add_executable(AffinityPropagation_example quantized_mappers/clustering/AffinityPropagation_example.cpp)
target_link_libraries(AffinityPropagation_example PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(AffinityPropagation_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

add_executable(HierarchicalClustering_example quantized_mappers/clustering/HierarchicalClustering_example.cpp) 
target_link_libraries(HierarchicalClustering_example PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(HierarchicalClustering_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")


# quantized_mappers / classification
add_executable(Classification_example quantized_mappers/classification/Classification_example.cpp)
target_link_libraries(Classification_example PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(Classification_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

add_executable(SVM_example quantized_mappers/classification/SVM_example.cpp)
target_link_libraries(SVM_example PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(SVM_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

add_executable(C45_example quantized_mappers/classification/C45_example.cpp)
set_target_properties(C45_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")
if(UNIX)
    target_compile_options(C45_example PRIVATE -pthread)
endif(UNIX)

add_executable(Bagging_example quantized_mappers/classification/Bagging_example.cpp)
target_link_libraries(Bagging_example PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(Bagging_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

# continious_mappers / clustering
add_executable(SOM_example continious_mappers/clustering/SOM_example.cpp)
target_link_libraries(SOM_example PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(SOM_example PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

add_executable(main_encoder main_encoder.cpp)
target_link_libraries(main_encoder PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(main_encoder PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

add_executable(main_esn main_esn.cpp)
target_link_libraries(main_esn PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(main_esn PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")

target_link_libraries(main_esn PRIVATE ${LAPACK_LIBRARIES})

# continious_mappers / classification








