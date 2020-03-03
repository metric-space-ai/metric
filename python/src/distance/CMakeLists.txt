set(LIB_NAME distance)

file(GLOB DISTANCE_SOURCE_FILES
   ${SOURCE_PATH}/distance/*.cpp)

include_directories(${Boost_INCLUDE_DIR}
                    ${HEADER_PATH}
                    ${PYTHON_INCLUDE_DIR}
                    ${METRIC_SOURCE_PATH}
                   )

link_directories(${Boost_LIBRARY_DIR})
add_library(${LIB_NAME} SHARED ${DISTANCE_SOURCE_FILES})
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")

target_link_libraries(${LIB_NAME}
    ${Boost_LIBRARIES}
    ${PYTHON3_LIBRARIES}
    ${lapackblas_libraries}
)

#INCLUDE(${CMAKE_SOURCE_DIR}/Footer.cmake)