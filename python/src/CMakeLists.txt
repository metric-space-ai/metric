set(LIB_NAME metric)

file(GLOB BASIC_SOURCE_FILES ./*.cpp)

pybind11_add_module(${LIB_NAME} ${BASIC_SOURCE_FILES})
target_link_libraries(${LIB_NAME} PUBLIC base_python_module)

add_subdirectory(distance)
add_subdirectory(mapping)
add_subdirectory(transform)
add_subdirectory(space)
add_subdirectory(utils)
add_subdirectory(correlation)