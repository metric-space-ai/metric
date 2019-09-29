cmake_minimum_required(VERSION 3.12)

set(Boost_DEBUG on)  
find_package(Boost)


project(energies_examples)


include_directories(
	${PROJECT_SOURCE_DIR}
)
	
file(COPY "assets" DESTINATION ".")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
	
# for Visual Studio
if(MSVC) 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC) 

include_directories(${Boost_INCLUDE_DIRS}) 


add_executable(energies_example energies_example.cpp)


if(UNIX)
    target_compile_features(energies_example PUBLIC cxx_std_17)
    target_compile_options(energies_example PRIVATE -pthread)
    set_target_properties(energies_example PROPERTIES LINK_FLAGS -pthread)
	
    # set_target_properties(energies_example PROPERTIES COMPILE_FLAGS "-stdlib=libc++ -pthread")
	# target_link_libraries(energies_example PRIVATE stdc++fs)
    # target_link_libraries(energies_example PRIVATE c++experimental pthread)
endif(UNIX)

find_package(LAPACK)


if(LAPACK_LIBRARIES)
	target_link_libraries(energies_example PRIVATE ${LAPACK_LIBRARIES})	
endif(LAPACK_LIBRARIES)

if(CMAKE_SYSTEM_NAME MATCHES Windows)

	target_link_directories(energies_example PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
	target_link_libraries(energies_example PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
endif()

target_link_libraries(energies_example PRIVATE ${Boost_LIBRARIES})









