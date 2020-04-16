set(LIB_NAME transform)

file(GLOB DISTANCE_SOURCE_FILES ./*.cpp)

pybind11_add_module(${LIB_NAME} ${DISTANCE_SOURCE_FILES})
target_link_libraries(${LIB_NAME} PUBLIC base_python_module)
