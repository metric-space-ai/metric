cmake_minimum_required(VERSION 3.10)

set(Boost_DEBUG on)  
find_package(Boost)


project(mapping_examples)


include_directories(
	${PROJECT_SOURCE_DIR}
)
	 
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
add_executable(SVM_example quantized_mappers/classification/SVM_example.cpp)
add_executable(C45_example quantized_mappers/classification/C45_example.cpp)
if(UNIX)
    target_compile_options(C45_example PRIVATE -pthread)
endif(UNIX)

# continious_mappers / clustering
add_executable(SOM_example continious_mappers/clustering/SOM_example.cpp)
add_executable(PCFA_example continious_mappers/clustering/PCFA_example.cpp)
add_executable(SOM_and_MNIST_example continious_mappers/clustering/SOM_and_MNIST_example.cpp)

# continious_mappers / classification
add_executable(ESN_example continious_mappers/classification/ESN_example.cpp)

# other
add_executable(auto_detect_metric_example auto_detect_metric_example.cpp)


if(UNIX)
	set_target_properties(KMedoids_example PROPERTIES CXX_STANDARD 17)
	set_target_properties(KMeans_example PROPERTIES CXX_STANDARD 17)
	set_target_properties(DBScan_example PROPERTIES CXX_STANDARD 17)
	set_target_properties(AffinityPropagation_example PROPERTIES CXX_STANDARD 17)
	set_target_properties(HierarchicalClustering_example PROPERTIES CXX_STANDARD 17)
	set_target_properties(SVM_example PROPERTIES CXX_STANDARD 17)
	set_target_properties(C45_example PROPERTIES CXX_STANDARD 17)
	set_target_properties(SOM_example PROPERTIES CXX_STANDARD 17)
	set_target_properties(PCFA_example PROPERTIES CXX_STANDARD 17)
	set_target_properties(SOM_and_MNIST_example PROPERTIES CXX_STANDARD 17)
	set_target_properties(ESN_example PROPERTIES CXX_STANDARD 17)
	set_target_properties(auto_detect_metric_example PROPERTIES CXX_STANDARD 17)
endif(UNIX)

find_package(LAPACK)


if(LAPACK_LIBRARIES)
	target_link_libraries(KMedoids_example PRIVATE ${LAPACK_LIBRARIES})	
	target_link_libraries(KMeans_example PRIVATE ${LAPACK_LIBRARIES})	
	target_link_libraries(DBScan_example PRIVATE ${LAPACK_LIBRARIES})	
	target_link_libraries(AffinityPropagation_example PRIVATE ${LAPACK_LIBRARIES})	
	target_link_libraries(HierarchicalClustering_example PRIVATE ${LAPACK_LIBRARIES})	
	target_link_libraries(SVM_example PRIVATE ${LAPACK_LIBRARIES})	
	target_link_libraries(C45_example PRIVATE ${LAPACK_LIBRARIES})	
	target_link_libraries(SOM_example PRIVATE ${LAPACK_LIBRARIES})	
	target_link_libraries(PCFA_example PRIVATE ${LAPACK_LIBRARIES})	
	target_link_libraries(SOM_and_MNIST_example PRIVATE ${LAPACK_LIBRARIES})	
	target_link_libraries(ESN_example PRIVATE ${LAPACK_LIBRARIES})
	target_link_libraries(auto_detect_metric_example PRIVATE ${LAPACK_LIBRARIES})
endif(LAPACK_LIBRARIES)

if(CMAKE_SYSTEM_NAME MATCHES Windows)

	target_link_directories(KMedoids_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(KMedoids_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(KMedoids_example PRIVATE ${Boost_LIBRARIES})

	target_link_directories(KMeans_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(KMeans_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(KMeans_example PRIVATE ${Boost_LIBRARIES})

	target_link_directories(DBScan_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(DBScan_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(DBScan_example PRIVATE ${Boost_LIBRARIES})

	target_link_directories(AffinityPropagation_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(AffinityPropagation_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(AffinityPropagation_example PRIVATE ${Boost_LIBRARIES})

	target_link_directories(HierarchicalClustering_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(HierarchicalClustering_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(HierarchicalClustering_example PRIVATE ${Boost_LIBRARIES})

	target_link_directories(SVM_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(SVM_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(SVM_example PRIVATE ${Boost_LIBRARIES})

	target_link_directories(C45_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(C45_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(C45_example PRIVATE ${Boost_LIBRARIES})

	target_link_directories(SOM_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(SOM_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(SOM_example PRIVATE ${Boost_LIBRARIES})

	target_link_directories(PCFA_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(PCFA_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(PCFA_example PRIVATE ${Boost_LIBRARIES})

	target_link_directories(SOM_and_MNIST_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(SOM_and_MNIST_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(SOM_and_MNIST_example PRIVATE ${Boost_LIBRARIES})

	target_link_directories(ESN_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(ESN_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(ESN_example PRIVATE ${Boost_LIBRARIES})

	target_link_directories(auto_detect_metric_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(auto_detect_metric_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(auto_detect_metric_example PRIVATE ${Boost_LIBRARIES})
endif()









