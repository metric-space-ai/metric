set(LIB_NAME distance)

set(DISTANCE_SOURCE_FILES init.cpp Edit.cpp TWED.cpp subs/standards.cpp)

pybind11_add_module(${LIB_NAME} ${DISTANCE_SOURCE_FILES})
target_link_libraries(${LIB_NAME} PUBLIC base_python_module)
target_compile_definitions(${LIB_NAME} PRIVATE METRIC_PYTHON_EMBED_STANDARDS_IN_DISTANCE=1)
