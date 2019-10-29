cmake_minimum_required(VERSION 3.12)

set(Boost_DEBUG on)  
find_package(Boost REQUIRED)


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
 

file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp) 

#Run through each source
foreach(exampleSrc ${EXAMPLE_SRCS})

	#Extract the filename without an extension (NAME_WE)
	get_filename_component(exampleName ${exampleSrc} NAME_WE)
	
	add_executable(${exampleName} ${exampleSrc})


	if(UNIX)
		target_compile_features(${exampleName} PUBLIC cxx_std_17)
		target_compile_options(${exampleName} PRIVATE -pthread -O3)
		set_target_properties(${exampleName} PROPERTIES LINK_FLAGS -pthread)
	endif(UNIX)

	find_package(LAPACK)


	if(LAPACK_LIBRARIES)
		target_link_libraries(${exampleName} PRIVATE ${LAPACK_LIBRARIES})	
	endif(LAPACK_LIBRARIES)

	if(CMAKE_SYSTEM_NAME MATCHES Windows)

		target_link_directories(${exampleName} PRIVATE ${PROJECT_SOURCE_DIR}/../../mkl/lib/intel64_win)
		target_link_libraries(${exampleName} PRIVATE mkl_core.lib mkl_sequential.lib mkl_intel_lp64.lib)
	endif()

	target_link_libraries(${exampleName} PRIVATE ${Boost_LIBRARIES})
endforeach(exampleSrc)









