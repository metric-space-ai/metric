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
target_link_libraries(${LIB_NAME} ${Boost_LIBRARIES} ${PYTHON3_LIBRARIES} ${lapackblas_libraries}
)

if (BUILD_DEBUG)
   target_link_libraries(${LIB_NAME}
      debug ${Boost_PYTHON_DEBUG_LIBRARY}
      debug ${OPENTHREADS_LIBRARY_DEBUG}
      debug ${OSGDB_LIBRARY_DEBUG}
      debug ${OSG_LIBRARY_DEBUG}
   )

endif(BUILD_DEBUG)

#INCLUDE(${CMAKE_SOURCE_DIR}/Footer.cmake)