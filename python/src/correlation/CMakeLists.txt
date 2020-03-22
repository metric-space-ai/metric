set(LIB_NAME correlation)

file(GLOB DISTANCE_SOURCE_FILES ${SOURCE_PATH}/correlation/*.cpp)

pybind11_add_module(${LIB_NAME} ${DISTANCE_SOURCE_FILES})
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "_")
target_link_libraries(${LIB_NAME} PRIVATE ${COMMON_LIBRARIES})