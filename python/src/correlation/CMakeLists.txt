set(LIB_NAME correlation)

file(GLOB DISTANCE_SOURCE_FILES ${SOURCE_PATH}/correlation/*.cpp)

link_directories(${Boost_LIBRARY_DIR})
add_library(${LIB_NAME} SHARED ${DISTANCE_SOURCE_FILES})
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "_")
target_link_libraries(${LIB_NAME} ${COMMON_LIBRARIES})