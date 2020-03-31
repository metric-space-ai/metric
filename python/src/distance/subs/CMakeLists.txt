set(LIB_NAME voi)

pybind11_add_module(${LIB_NAME} ./VOI.cpp)
target_link_libraries(${LIB_NAME} PUBLIC base_python_module)
