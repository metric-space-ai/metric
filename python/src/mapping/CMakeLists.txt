set(LIB_NAME mapping)

file(GLOB DISTANCE_SOURCE_FILES ${SOURCE_PATH}/mapping/*.cpp)

# ----------------------------------------------------------------------
add_library(${LIB_NAME} SHARED ${DISTANCE_SOURCE_FILES})
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "_")
target_link_libraries(${LIB_NAME} ${COMMON_LIBRARIES})

add_subdirectory(subs)
