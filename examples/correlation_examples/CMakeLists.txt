file(COPY "assets" DESTINATION ".")

add_executable(advanced_example advanced_example.cpp)
target_link_libraries(advanced_example metric)

add_executable(intro_example intro_example.cpp)
target_link_libraries(intro_example metric)

add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example metric)