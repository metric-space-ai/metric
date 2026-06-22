find_package(Threads REQUIRED)

file(COPY "assets" DESTINATION ".")

# ---------------------------------------------------------------------------
# Dependency-free examples (always built when METRIC_BUILD_EXAMPLES=ON).
# These use only the header-only METRIC core + the C++ standard library
# (KOC_MNIST/kohonen additionally use POSIX <dirent.h> for directory scanning).
# ---------------------------------------------------------------------------
set(MAPPING_EXAMPLES
    quantized_mappers/clustering/KMedoids_example.cpp
    quantized_mappers/clustering/KMeans_example.cpp
    quantized_mappers/clustering/DBScan_example.cpp
    quantized_mappers/clustering/AffinityPropagation_example.cpp
    quantized_mappers/clustering/HierarchicalClustering_example.cpp
    continuous_mappers/clustering/SOM_3_example.cpp
    continuous_mappers/clustering/PCFA_example.cpp
    continuous_mappers/clustering/DSPCC_example.cpp
    continuous_mappers/classification/ESN_example.cpp
    auto_detect_metric_example.cpp
    kohonen_outliers_clustering_example.cpp
    KOC_MNIST_example.cpp
    reverse_diffusion_example.cpp)

foreach (exampleSrc ${MAPPING_EXAMPLES})
    get_filename_component(exampleName ${exampleSrc} NAME_WE)
    add_executable(${exampleName} ${exampleSrc})
    target_link_libraries(${exampleName} PRIVATE ${METRIC_TARGET_NAME})
endforeach ()

target_link_libraries(KOC_MNIST_example PRIVATE Threads::Threads)
target_link_libraries(kohonen_outliers_clustering_example PRIVATE Threads::Threads)

# ---------------------------------------------------------------------------
# nlohmann/json examples (external, example-only dependency; opt-in).
# SOM_example loads assets/data.json; SOM_2_example loads assets/toy_dataset_*.json.
# ---------------------------------------------------------------------------
if (METRIC_EXAMPLES_WITH_JSON)
    add_executable(SOM_example continuous_mappers/clustering/SOM_example.cpp)
    add_executable(SOM_2_example continuous_mappers/clustering/SOM_2_example.cpp)
    target_link_libraries(SOM_example PRIVATE ${METRIC_TARGET_NAME} ${METRIC_EXAMPLES_JSON_LINK})
    target_link_libraries(SOM_2_example PRIVATE ${METRIC_TARGET_NAME} ${METRIC_EXAMPLES_JSON_LINK})
else ()
    message(STATUS "mapping_examples: skipping SOM_example, SOM_2_example (need nlohmann/json; set METRIC_EXAMPLES_WITH_JSON=ON)")
endif ()

# ---------------------------------------------------------------------------
# MNIST example (external IDX dataset via bundled example-only MIT reader assets/mnist/; opt-in).
# ---------------------------------------------------------------------------
if (METRIC_EXAMPLES_WITH_MNIST)
    add_executable(SOM_and_MNIST_example continuous_mappers/clustering/SOM_and_MNIST_example.cpp)
    target_link_libraries(SOM_and_MNIST_example PRIVATE ${METRIC_TARGET_NAME})
else ()
    message(STATUS "mapping_examples: skipping SOM_and_MNIST_example (needs an external MNIST dataset; set METRIC_EXAMPLES_WITH_MNIST=ON)")
endif ()
