set(LIB_NAME voi)

# ----------------------------------------------------------------------
add_library(${LIB_NAME} SHARED ${SOURCE_PATH}/distance/subs/VOI.cpp)
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "_")
target_link_libraries(${LIB_NAME} ${COMMON_LIBRARIES})
