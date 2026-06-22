find_package(Threads REQUIRED)

file(COPY "assets" DESTINATION ".")

# Every example in this folder serialises its results with nlohmann/json, an external,
# example-only dependency that the METRIC core does NOT require. The whole folder is
# therefore opt-in behind METRIC_EXAMPLES_WITH_JSON (default OFF).
if (METRIC_EXAMPLES_WITH_JSON)
    file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
    foreach (exampleSrc ${EXAMPLE_SRCS})
        get_filename_component(exampleName ${exampleSrc} NAME_WE)
        add_executable(${exampleName} ${exampleSrc})
        target_link_libraries(${exampleName} PRIVATE ${METRIC_TARGET_NAME} Threads::Threads ${METRIC_EXAMPLES_JSON_LINK})
    endforeach ()
else ()
    message(STATUS "energies_examples: skipped (all examples need nlohmann/json; set METRIC_EXAMPLES_WITH_JSON=ON)")
endif ()
