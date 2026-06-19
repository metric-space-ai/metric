cmake_minimum_required(VERSION 3.19)

project(panda_metric_downstream_consumer LANGUAGES CXX)

find_package(panda_metric REQUIRED)

add_executable(panda_metric_downstream_smoke main.cpp)
target_link_libraries(panda_metric_downstream_smoke PRIVATE panda_metric::panda_metric)

enable_testing()
add_test(NAME panda_metric_downstream_smoke COMMAND panda_metric_downstream_smoke)
