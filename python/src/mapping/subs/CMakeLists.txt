# ----------------------------------------------------------------------
add_library(KOC SHARED ${SOURCE_PATH}/mapping/subs/KOC.cpp)
set_target_properties(KOC PROPERTIES CXX_STANDARD 17)
set_target_properties(KOC PROPERTIES PREFIX "_")
target_link_libraries(KOC ${COMMON_LIBRARIES})

# ----------------------------------------------------------------------
add_library(affprop SHARED ${SOURCE_PATH}/mapping/subs/affprop.cpp)
set_target_properties(affprop PROPERTIES CXX_STANDARD 17)
set_target_properties(affprop PROPERTIES PREFIX "_")
target_link_libraries(affprop ${COMMON_LIBRARIES})

# ----------------------------------------------------------------------
add_library(dbscan SHARED ${SOURCE_PATH}/mapping/subs/dbscan.cpp)
set_target_properties(dbscan PROPERTIES CXX_STANDARD 17)
set_target_properties(dbscan PROPERTIES PREFIX "_")
target_link_libraries(dbscan ${COMMON_LIBRARIES})
