# METRIC Tutorial

A working end-to-end path: install, build, load real data, get a first result.
Every snippet below mirrors a promoted example under `examples/core/` that runs
as a `ctest` in the `core` preset, so it is known to compile and pass.

## Configuration and installation

C++ is header-only to include, but the stats/entropy layer links LAPACK.

```shell
cmake --preset core
cmake --build --preset core
ctest --preset core
```

Downstream projects consume the installed package; the `metric::metric` target
carries its LAPACK dependency automatically:

```cmake
find_package(metric REQUIRED)
target_link_libraries(program PRIVATE metric::metric)
```

For a raw compile without CMake, link LAPACK yourself:
`c++ -std=c++17 -I. app.cpp -framework Accelerate` (macOS) or `... -llapack` (Linux).

## Spaces

A finite metric space is records plus one metric. The minimal path
(`examples/core/metric_space_strings.cpp`):

```cpp
#include <metric/workflow.hpp>
#include <iostream>
#include <vector>

int main()
{
    std::vector<std::string> words = {"metric", "metrics", "matrix", "tree"};
    auto records = mtrc::record::import_records(words);
    auto space = mtrc::space::build_checked(records, mtrc::Edit<char>{});

    auto structure = mtrc::describe_structure(space);
    std::cout << structure << "\n";                       // one-line provenance summary

    auto neighbors = mtrc::find_neighbors(space, std::string("metricks"), 2);
    std::cout << neighbors << "\n";
}
```

Load real data from a file instead of in-code records
(`examples/core/record_csv_workflow.cpp`):

```cpp
auto records = mtrc::record::read_csv<double>("data.csv");   // validates shape + finiteness
auto space = mtrc::space::build_checked(records, mtrc::Euclidean<double>{});
```

Guard messy input before building:

```cpp
auto space = mtrc::space::space_builder<std::vector<double>>(mtrc::Euclidean<double>{})
                 .add_all(records)
                 .require_non_empty()
                 .require_uniform_dimension()
                 .require_finite()            // fails fast on NaN/Inf with the offending record index
                 .build();
```

## Properties

An existing space answers structural questions. Entropy is the coordinate-space
case: either the source space is already a coordinate space, or you first map/embed
the source and run entropy on the derived coordinate space.
(`examples/tutorial.cpp`, `metric_space_intrinsic_dimension.cpp`; mapped-source
entropy is shown in `examples/engine/condition_monitoring.cpp`):

```cpp
auto e = mtrc::stats::properties::entropy(coordinate_space_or_mapping);
if (e.succeeded()) std::cout << e << "\n";              // EntropyResult(value=..., status=valid, ...)
std::cout << "intrinsic dimension: " << mtrc::intrinsic_dimension(space) << "\n";
```

## Mapping

When coordinates help, derive a coordinate space (PCFA, parametric coordinate solver, or
parametric diffusion coordinate). See `examples/engine/metric_space_mapping_pipeline.cpp`. These are
derived spaces; the source metric stays authoritative.

## Inspecting results

Every result streams a one-line, provenance-carrying summary, and a distance
matrix or embedding exports to CSV:

```cpp
std::cout << neighbors << "\n";                         // operator<< on any *Result
auto matrix = mtrc::space::materialize(space);          // pairwise distances
// mtrc::render::write_csv(std::cout, rows);            // export to CSV
// mtrc::render::write_histogram(std::cout, values);    // quick ASCII distribution
```

## Scope

For what is supported vs. out-of-scope vs. legacy, and for choosing a
representation at scale, see [Supported Surface](../docs/supported-surface.md).
For Python, see [Python API](../docs/api/python.md) — note the availability
banner: construction/distance/pairwise/representation views and exact neighbor
search run today; higher intent methods are being promoted incrementally.
