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

# ----------------------------------------------------------------------
add_library(ensembles SHARED ${SOURCE_PATH}/mapping/subs/ensembles.cpp)
set_target_properties(ensembles PROPERTIES CXX_STANDARD 17)
set_target_properties(ensembles PROPERTIES PREFIX "_")
target_link_libraries(ensembles ${COMMON_LIBRARIES})

# ----------------------------------------------------------------------
add_library(dt SHARED ${SOURCE_PATH}/mapping/subs/dt.cpp)
set_target_properties(dt PROPERTIES CXX_STANDARD 17)
set_target_properties(dt PROPERTIES PREFIX "_")
target_link_libraries(dt ${COMMON_LIBRARIES})

# ----------------------------------------------------------------------
add_library(esn SHARED ${SOURCE_PATH}/mapping/subs/ESN.cpp)
set_target_properties(esn PROPERTIES CXX_STANDARD 17)
set_target_properties(esn PROPERTIES PREFIX "_")
target_link_libraries(esn ${COMMON_LIBRARIES})
