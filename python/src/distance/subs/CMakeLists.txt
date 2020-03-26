set(LIB_NAME voi)

# ----------------------------------------------------------------------
pybind11_add_module(${LIB_NAME} ${SOURCE_PATH}/distance/subs/VOI.cpp)
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "_")
target_link_libraries(${LIB_NAME} PRIVATE ${COMMON_LIBRARIES})
