set(LIB_NAME metric)

file(GLOB BASIC_SOURCE_FILES ${SOURCE_PATH}/*.cpp)

pybind11_add_module(${LIB_NAME} ${BASIC_SOURCE_FILES})
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "_")
target_link_libraries(${LIB_NAME} PRIVATE ${COMMON_LIBRARIES})

add_subdirectory(distance)
add_subdirectory(mapping)
add_subdirectory(transform)
add_subdirectory(space)
add_subdirectory(utils)
#add_subdirectory(correlation)