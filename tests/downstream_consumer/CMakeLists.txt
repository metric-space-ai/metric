cmake_minimum_required(VERSION 3.19)

project(metric_downstream_consumer LANGUAGES CXX)

find_package(metric REQUIRED)

add_executable(metric_downstream_smoke main.cpp)
target_link_libraries(metric_downstream_smoke PRIVATE metric::metric)

enable_testing()
add_test(NAME metric_downstream_smoke COMMAND metric_downstream_smoke)
