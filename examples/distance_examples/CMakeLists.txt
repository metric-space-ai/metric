file(COPY "assets" DESTINATION ".")

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# Examples that require external, example-only tooling the METRIC core does NOT need.
# They are handled explicitly below and skipped unless opted in (see examples/CMakeLists.txt):
#   - earth_mover_distance_2_example: Boost.GIL + libjpeg (JPEG pixel loading only)
#   - entropy_mnist[_riemannian]_example: a legacy MNIST "data.cereal" archive that the
#     dependency-free core no longer reads (Datasets::getMnist throws at runtime)
set(DISTANCE_OPT_IN_EXAMPLES
    earth_mover_distance_2_example
    entropy_mnist_example
    entropy_mnist_riemannian_example)

foreach (exampleSrc ${EXAMPLE_SRCS})
    get_filename_component(exampleName ${exampleSrc} NAME_WE)
    if (exampleName IN_LIST DISTANCE_OPT_IN_EXAMPLES)
        continue()
    endif ()
    add_executable(${exampleName} ${exampleSrc})
    target_link_libraries(${exampleName} ${METRIC_TARGET_NAME})
endforeach ()

# Boost.GIL EMD demo (opt-in). Boost.GIL is header-only; JPEG I/O needs libjpeg.
if (METRIC_EXAMPLES_WITH_BOOST_GIL)
    add_executable(earth_mover_distance_2_example earth_mover_distance_2_example.cpp)
    target_link_libraries(earth_mover_distance_2_example ${METRIC_TARGET_NAME})
    find_package(JPEG QUIET)
    if (JPEG_FOUND)
        target_link_libraries(earth_mover_distance_2_example JPEG::JPEG)
    else ()
        message(WARNING "earth_mover_distance_2_example needs Boost.GIL + libjpeg; "
                        "libjpeg (find_package(JPEG)) was not found. This is an external, "
                        "example-only dependency, not a METRIC core requirement.")
    endif ()
endif ()

# Legacy MNIST entropy demos (opt-in). NOTE: Datasets::getMnist no longer reads the
# data.cereal archive in the dependency-free core, so these throw at runtime.
if (METRIC_EXAMPLES_WITH_MNIST)
    add_executable(entropy_mnist_example entropy_mnist_example.cpp)
    add_executable(entropy_mnist_riemannian_example entropy_mnist_riemannian_example.cpp)
    target_link_libraries(entropy_mnist_example ${METRIC_TARGET_NAME})
    target_link_libraries(entropy_mnist_riemannian_example ${METRIC_TARGET_NAME})
endif ()
