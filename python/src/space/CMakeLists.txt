set(LIB_NAME mapping)

file(GLOB DISTANCE_SOURCE_FILES
   ${SOURCE_PATH}/mapping/*.cpp)

include_directories(${Boost_INCLUDE_DIR}
                    ${HEADER_PATH}
                    ${PYTHON_INCLUDE_DIR}
                    ${METRIC_SOURCE_PATH}
                   )
link_directories(${Boost_LIBRARY_DIR})

# ----------------------------------------------------------------------
add_library(${LIB_NAME} SHARED ${DISTANCE_SOURCE_FILES})
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")

target_link_libraries(${LIB_NAME}
    ${Boost_LIBRARIES}
    ${PYTHON3_LIBRARIES}
    ${lapackblas_libraries}
)

# ----------------------------------------------------------------------
add_library(KOC SHARED ${SOURCE_PATH}/mapping/KOC/KOC.cpp)
set_target_properties(KOC PROPERTIES CXX_STANDARD 17)
set_target_properties(KOC PROPERTIES PREFIX "")

target_link_libraries(KOC
    ${Boost_LIBRARIES}
    ${PYTHON3_LIBRARIES}
    ${lapackblas_libraries}
)

# ----------------------------------------------------------------------
add_library(affprop SHARED ${SOURCE_PATH}/mapping/KOC/affprop.cpp)
set_target_properties(affprop PROPERTIES CXX_STANDARD 17)
set_target_properties(affprop PROPERTIES PREFIX "")

target_link_libraries(affprop
    ${Boost_LIBRARIES}
    ${PYTHON3_LIBRARIES}
    ${lapackblas_libraries}
)

# ----------------------------------------------------------------------
add_library(dbscan SHARED ${SOURCE_PATH}/mapping/KOC/dbscan.cpp)
set_target_properties(dbscan PROPERTIES CXX_STANDARD 17)
set_target_properties(dbscan PROPERTIES PREFIX "")

target_link_libraries(dbscan
    ${Boost_LIBRARIES}
    ${PYTHON3_LIBRARIES}
    ${lapackblas_libraries}
)