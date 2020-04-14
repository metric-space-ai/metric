set(LIB_NAME utils)

file(GLOB UTILS_SOURCE_FILES ./*.cpp)

pybind11_add_module(${LIB_NAME} ${UTILS_SOURCE_FILES})
target_link_libraries(${LIB_NAME} PUBLIC base_python_module)
