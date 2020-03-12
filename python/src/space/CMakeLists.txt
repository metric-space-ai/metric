set(LIB_NAME space)

file(GLOB SPACE_SOURCE_FILES ${SOURCE_PATH}/space/*.cpp)

add_library(${LIB_NAME} SHARED ${SPACE_SOURCE_FILES})
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "_")
target_link_libraries(${LIB_NAME} ${COMMON_LIBRARIES})
