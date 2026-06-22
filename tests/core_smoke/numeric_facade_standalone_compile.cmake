# Compiles each curated mtrc::numeric Level-2 facade header standalone, proving
# the default dependency-free, header-only boundary holds for every facade.
# Invoked as a ctest COMMAND through `cmake -P`. Each facade is compiled in its
# own translation unit with BLAS off so no facade may silently depend on another
# header or on an optional backend.

if(NOT DEFINED METRIC_CXX OR NOT DEFINED METRIC_SOURCE_DIR OR NOT DEFINED METRIC_WORK_DIR)
    message(FATAL_ERROR "numeric_facade_standalone_compile.cmake requires METRIC_CXX, METRIC_SOURCE_DIR, METRIC_WORK_DIR")
endif()

set(_facades scalar vector matrix sparse linear_algebra graph random parallel io)

foreach(_facade IN LISTS _facades)
    set(_src "${METRIC_WORK_DIR}/facade_standalone_${_facade}.cpp")
    file(WRITE "${_src}" "#include <metric/numeric/${_facade}.hpp>\nint main() { return 0; }\n")

    set(_cmd "${METRIC_CXX}")
    if(METRIC_CXX17_FLAG)
        list(APPEND _cmd "${METRIC_CXX17_FLAG}")
    endif()
    list(APPEND _cmd -DMETRIC_NUMERIC_BLAS_MODE=0 -I "${METRIC_SOURCE_DIR}" -c "${_src}" -o "${_src}.o")

    execute_process(COMMAND ${_cmd} RESULT_VARIABLE _rc OUTPUT_VARIABLE _out ERROR_VARIABLE _err)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "facade metric/numeric/${_facade}.hpp failed standalone header-only compile:\n${_out}${_err}")
    endif()
endforeach()

message(STATUS "all nine mtrc::numeric facade headers compiled standalone (dependency-free)")
