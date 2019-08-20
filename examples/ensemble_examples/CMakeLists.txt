include_directories(
    ${PROJECT_SOURCE_DIR}
    )
function(MakeExample EXAMPLE_NAME) 
add_executable(${EXAMPLE_NAME} ${EXAMPLE_NAME}.cpp)
target_link_libraries(${EXAMPLE_NAME} PRIVATE ${LAPACK_LIBRARIES})
set_target_properties(${EXAMPLE_NAME} PROPERTIES   RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/examples")
endfunction()

MakeExample(Boosting_example)
MakeExample(CWA_demo)
MakeExample(MetricDecisionTree_example)
