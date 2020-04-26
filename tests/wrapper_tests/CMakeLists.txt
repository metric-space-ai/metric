# see https://cmake.org/cmake/help/latest/module/FindBoost.html
set (Boost_USE_STATIC_LIBS_OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

find_package(BLAS)

include_directories(${Boost_INCLUDE_DIR})
add_executable(lapack_tests lapack_tests.cpp)
target_link_libraries(lapack_tests ${Boost_LIBRARIES} ${LAPACK_LIBRARIES})

add_test(name lapack_tests command lapack_tests)