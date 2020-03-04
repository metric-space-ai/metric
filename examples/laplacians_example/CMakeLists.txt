cmake_minimum_required(VERSION 3.12)

project(laplacians_examples)

set(Boost_DEBUG on)  
find_package(Boost)

include_directories(
    ${PROJECT_SOURCE_DIR}
    )

include_directories(${Boost_INCLUDE_DIRS}) 

find_package(LAPACK)

#Run through each source

# error: ‘sparsify’ is not a member of ‘metric’; did you mean ‘sparse’?
#add_executable(Laplacians_test Laplacians_test.cpp sparsification_test.cpp tests.cpp)
add_executable(Laplacians_test Laplacians_test.cpp tests.cpp)
target_link_libraries(Laplacians_test PRIVATE ${LAPACK_LIBRARIES})

#link to Boost libraries AND your targets and dependencies
target_link_libraries(Laplacians_test ${Boost_LIBRARIES})
set_target_properties(Laplacians_test PROPERTIES CXX_STANDARD 17)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	target_link_directories(Laplacians_test PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(Laplacians_test PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	target_link_libraries(Laplacians_test PRIVATE ${Boost_LIBRARIES})
endif()
