cmake_minimum_required(VERSION 3.10)


#find_package(Boost COMPONENTS serialization system REQUIRED)

#find_package(LAPACK)

#include_directories(
#		${PROJECT_SOURCE_DIR}
#		${PROJECT_SOURCE_DIR}/../..
#		${Boost_INCLUDE_DIRS}
#)

#set(BOOST_LINK_DIRS "${BOOST_ROOT}/stage/lib")

#if (UNIX)
#	set(THREADS_PREFER_PTHREAD_FLAG ON)
#	find_package(Threads REQUIRED)
#endif (UNIX)


add_executable(knn_graph_tests knn_graph_tests.cpp)
add_executable(space_matrix_tests space_matrix_tests.cpp)
add_executable(space_tree_tests space_tree_tests.cpp)

target_link_libraries(knn_graph_tests Catch2::Catch2)
target_link_libraries(space_matrix_tests Catch2::Catch2)
target_link_libraries(space_tree_tests Catch2::Catch2)

catch_discover_tests(knn_graph_tests)
catch_discover_tests(space_matrix_tests)
catch_discover_tests(space_tree_tests)

#AUX_SOURCE_DIRECTORY(. TEST_SRCS)
##Run through each source
#foreach (testSrc ${TEST_SRCS})
#
#	#Extract the filename without an extension (NAME_WE)
#	get_filename_component(testName ${testSrc} NAME_WE)
#
#	#Add compile target
#	add_executable(${testName} ${testSrc})
#
#	if (${testSrc} matches knn_graph_test.cpp)
#		sdsd
#	else()
#
#	endif()
#
#	#link to Boost libraries AND your targets and dependencies
#
#	target_include_directories(${testName} PRIVATE ${Boost_INCLUDE_DIRS})
#	target_link_libraries(${testName} PRIVATE ${Boost_LIBRARIES})
#	target_link_libraries(${testName} PRIVATE ${BOOST_TEST_LIB})
#
#	if (LAPACK_LIBRARIES)
#		target_link_libraries(${testName} PRIVATE ${LAPACK_LIBRARIES})
#	endif (LAPACK_LIBRARIES)
#
#	if (CMAKE_SYSTEM_NAME MATCHES Windows)
#		target_link_directories(${testName} PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
#		target_link_libraries(${testName} PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
#		target_link_libraries(${testName} PRIVATE ${Boost_LIBRARIES})
#		target_link_directories(${testName} PRIVATE ${BOOST_LINK_DIRS})
#	endif ()
#
#	if (UNIX)
#		target_link_libraries(${testName} PRIVATE Threads::Threads)
#		target_compile_options(${testName} PRIVATE -DBOOST_TEST_DYN_LINK)
#	endif (UNIX)
#
#	#I like to move testing binaries into a testBin directory
#	#        set_target_properties(${testName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR})
#	#Finally add it to test execution -
#	#Notice the WORKING_DIRECTORY and COMMAND
#	add_test(NAME ${testName}
#			COMMAND ${testName})
#endforeach (testSrc)
