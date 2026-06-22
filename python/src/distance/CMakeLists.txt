set(LIB_NAME distance)

if (METRIC_PYTHON_BUILD_FULL)
    file(GLOB DISTANCE_SOURCE_FILES ./*.cpp)
else ()
    set(DISTANCE_SOURCE_FILES init.cpp Edit.cpp subs/standards.cpp)
endif ()

pybind11_add_module(${LIB_NAME} ${DISTANCE_SOURCE_FILES})
target_link_libraries(${LIB_NAME} PUBLIC base_python_module)
if (METRIC_PYTHON_BUILD_FULL)
    target_compile_definitions(${LIB_NAME} PRIVATE METRIC_PYTHON_BUILD_FULL=1)
else ()
    target_compile_definitions(${LIB_NAME} PRIVATE METRIC_PYTHON_EMBED_STANDARDS_IN_DISTANCE=1)
endif ()

if (METRIC_PYTHON_BUILD_FULL)
    add_subdirectory(subs)
endif ()
