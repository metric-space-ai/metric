file(COPY "assets" DESTINATION ".")

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

# Examples that require external, example-only tooling the METRIC core does NOT need.
# They are handled explicitly below and skipped unless opted in (see examples/CMakeLists.txt):
#   - earth_mover_distance_2_example: Boost.GIL + libjpeg (JPEG pixel loading only)
set(DISTANCE_OPT_IN_EXAMPLES
    earth_mover_distance_2_example)

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
