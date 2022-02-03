file(COPY "assets" DESTINATION ".")

add_executable(advanced_example advanced_example.cpp)
target_link_libraries(advanced_example ${METRIC_TARGET_NAME})

add_executable(intro_example intro_example.cpp)
target_link_libraries(intro_example ${METRIC_TARGET_NAME})

add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example ${METRIC_TARGET_NAME})