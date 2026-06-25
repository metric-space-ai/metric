# ---------------------------------------------------------------------------
# Optional, example-only third-party dependencies.
#
# The METRIC C++ core is header-only and dependency-free, and so are the
# examples that build by default. A few historical example programs still rely
# on external tooling that METRIC itself does NOT require. They are quarantined
# behind the opt-in switches below and are skipped by default, so a plain
# `-DMETRIC_BUILD_EXAMPLES=ON` build pulls in NO third-party library.
#
# Enabling a switch means YOU must provide the named tool on your system;
# METRIC never bundles or fetches it.
# ---------------------------------------------------------------------------
option(METRIC_EXAMPLES_WITH_JSON
       "Build example programs that require nlohmann/json (external; NOT a METRIC dependency)" OFF)
option(METRIC_EXAMPLES_WITH_BOOST_GIL
       "Build example programs that require Boost.GIL + libjpeg (external; NOT a METRIC dependency)" OFF)
option(METRIC_EXAMPLES_WITH_MNIST
       "Build example programs that need an external MNIST dataset (raw IDX via the bundled example-only MIT reader)" OFF)

# When the JSON examples are requested, try to locate nlohmann/json without
# fetching it. If found we link the imported target; otherwise we warn and let
# the examples assume <nlohmann/json.hpp> is on the compiler include path. Never
# a hard requirement of METRIC.
set(METRIC_EXAMPLES_JSON_LINK "")
if (METRIC_EXAMPLES_WITH_JSON)
    find_package(nlohmann_json QUIET)
    if (nlohmann_json_FOUND)
        set(METRIC_EXAMPLES_JSON_LINK nlohmann_json::nlohmann_json)
        message(STATUS "METRIC examples: nlohmann/json found; JSON-dependent examples enabled")
    else ()
        message(WARNING
                "METRIC_EXAMPLES_WITH_JSON=ON but nlohmann/json was not found via find_package(nlohmann_json). "
                "JSON-dependent examples will be built assuming <nlohmann/json.hpp> is on the compiler include "
                "path. This is an external, example-only dependency, not a METRIC core requirement.")
    endif ()
endif ()
if (METRIC_EXAMPLES_WITH_BOOST_GIL)
    message(STATUS "METRIC examples: Boost.GIL EMD example enabled (requires Boost.GIL + libjpeg; external, not a METRIC dependency)")
endif ()
if (METRIC_EXAMPLES_WITH_MNIST)
    message(STATUS "METRIC examples: MNIST examples enabled (require an external MNIST dataset; not a METRIC dependency)")
endif ()

# add_subdirectory(correlation_examples)
add_subdirectory(distance_examples)
#add_subdirectory(graph_pyhton_assets)
add_subdirectory(laplacians_example)
add_subdirectory(space_examples)
add_subdirectory(transform_examples)
