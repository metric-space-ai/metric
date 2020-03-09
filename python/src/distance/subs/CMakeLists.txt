# ----------------------------------------------------------------------
add_library(VOI SHARED ${SOURCE_PATH}/distance/subs/VOI.cpp)
set_target_properties(VOI PROPERTIES CXX_STANDARD 17)
set_target_properties(VOI PROPERTIES PREFIX "_")
target_link_libraries(VOI ${COMMON_LIBRARIES})
