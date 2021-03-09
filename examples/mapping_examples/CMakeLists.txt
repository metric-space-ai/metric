find_package(Threads REQUIRED)


file(COPY "assets" DESTINATION ".")


# quantized_mappers / clustering
add_executable(KMedoids_example quantized_mappers/clustering/KMedoids_example.cpp)
add_executable(KMeans_example quantized_mappers/clustering/KMeans_example.cpp)
add_executable(DBScan_example quantized_mappers/clustering/DBScan_example.cpp)
add_executable(AffinityPropagation_example quantized_mappers/clustering/AffinityPropagation_example.cpp)
add_executable(HierarchicalClustering_example quantized_mappers/clustering/HierarchicalClustering_example.cpp) 

# quantized_mappers / constraint

# quantized_mappers / classification
add_executable(SVM_example quantized_mappers/classification/SVM_example.cpp)
add_executable(C45_example quantized_mappers/classification/C45_example.cpp)

# continious_mappers / clustering
add_executable(SOM_example continious_mappers/clustering/SOM_example.cpp)
add_executable(SOM_2_example continious_mappers/clustering/SOM_2_example.cpp)
add_executable(SOM_3_example continious_mappers/clustering/SOM_3_example.cpp)
add_executable(PCFA_example continious_mappers/clustering/PCFA_example.cpp)
add_executable(SOM_and_MNIST_example continious_mappers/clustering/SOM_and_MNIST_example.cpp)

# continious_mappers / classification
add_executable(ESN_example continious_mappers/classification/ESN_example.cpp)

# other
add_executable(auto_detect_metric_example auto_detect_metric_example.cpp)
add_executable(kohonen_outliers_clustering_example kohonen_outliers_clustering_example.cpp)
add_executable(KOC_MNIST_example KOC_MNIST_example.cpp)
add_executable(reverse_diffusion_example reverse_diffusion_example.cpp)


target_link_libraries(KMedoids_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(KMeans_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(DBScan_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(AffinityPropagation_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(HierarchicalClustering_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(SVM_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(C45_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(SOM_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(SOM_2_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(SOM_3_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(PCFA_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(SOM_and_MNIST_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(ESN_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(auto_detect_metric_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(kohonen_outliers_clustering_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(KOC_MNIST_example PRIVATE ${LAPACK_LIBRARIES})
target_link_libraries(reverse_diffusion_example PRIVATE ${LAPACK_LIBRARIES})


target_link_libraries(C45_example PRIVATE Threads::Threads)
target_link_libraries(KOC_MNIST_example PRIVATE Threads::Threads)
target_link_libraries(kohonen_outliers_clustering_example PRIVATE Threads::Threads)

