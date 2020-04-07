set(LIB_NAME space)

file(GLOB SPACE_SOURCE_FILES ./*.cpp)

pybind11_add_module(${LIB_NAME} ${SPACE_SOURCE_FILES})
target_link_libraries(${LIB_NAME} PUBLIC base_python_module)
