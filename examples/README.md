# Examples

The examples tree contains both the revived public examples and broader historical examples.

## Core Revival Examples

`examples/core/` is the promoted example set for the revival. These examples are small, deterministic, non-vector-first where possible, and executed by the `core` and `dev` CTest presets.

The current core examples cover:

- string/edit metric spaces with an edit-distance-vs-anagram-vector baseline
- process-curve alignment, including licensed external PowerDemand and
  InternalBleeding gallery benchmarks and a non-vector native PHATE-AE mapping
  fixture
- custom metric callables
- explicit matrix, graph, and tree representations
- engine pipeline metadata inspection through the PHATE-AE builder example
- a native engine benchmark report that consolidates representation-cost and
  application-workflow evidence for promoted fixtures
- mixed structured records with a native composed metric and numeric-vector
  baseline benchmark
- distribution/image recoding with native transport cost and flat-vector
  baseline benchmark
- PHATE-AE learned mapping through explicit native pipelines with vector-row
  and process-curve feature-codec fixtures
- entropy diagnostics
- intrinsic-dimension diagnostics
- MGC correlation between metric spaces with a raw-vector pairing baseline
- TWED for time-series records
- EMD and native transport-vs-vector baselines for histogram records

These examples are release-gate examples. A failure here is a core regression.

## Historical and Extended Examples

The other example folders preserve broader project history:

- `correlation_examples/`
- `distance_examples/`
- `dnn_examples/`
- `energies_examples/`
- `ensemble_examples/`
- `laplacians_example/`
- `mapping_examples/`
- `space_examples/`
- `transform_examples/`
- `utils_examples/`

Those folders are not first-page API promises and are not revival release gates. They should be promoted one area at a time only after they have deterministic expected output, clear docs, and CI coverage.

## Optional, example-only third-party dependencies

The METRIC C++ core is header-only and dependency-free, and so are the examples that build by default. A handful of historical example programs still rely on external tooling that **METRIC itself does not require**. They are quarantined behind opt-in CMake switches (all default `OFF`), so a plain `-DMETRIC_BUILD_EXAMPLES=ON` build pulls in **no** third-party library. Enabling a switch means *you* must provide the named tool; METRIC never vendors or fetches it.

| CMake switch | External tool | Gated examples |
| --- | --- | --- |
| `METRIC_EXAMPLES_WITH_JSON` | [nlohmann/json](https://github.com/nlohmann/json) (header-only) | `mapping_examples/SOM_example`, `mapping_examples/SOM_2_example`, and all of `energies_examples/` |
| `METRIC_EXAMPLES_WITH_BOOST_GIL` | Boost.GIL + libjpeg | `distance_examples/earth_mover_distance_2_example` |
| `METRIC_EXAMPLES_WITH_MNIST` | An external MNIST dataset | `mapping_examples/SOM_and_MNIST_example`, `distance_examples/entropy_mnist_example`, `distance_examples/entropy_mnist_riemannian_example` |

Notes:

- The other mapping examples that previously `#include`d `nlohmann/json.hpp` (`SOM_3`, `auto_detect_metric`, `kohonen_outliers_clustering`, `KOC_MNIST`, `reverse_diffusion`) only carried a **dead** JSON include; it has been removed and they now build dependency-free. `KOC_MNIST` and `kohonen_outliers_clustering` additionally use POSIX `<dirent.h>` (not portable to MSVC).
- `SOM_and_MNIST_example` reads raw MNIST IDX data via a **bundled example-only third-party reader** under [`mapping_examples/assets/mnist/`](mapping_examples/assets/mnist/) (`mnist_reader.hpp`, Baptiste Wicht, MIT). This is not part of the METRIC core.
- The two `entropy_mnist*` distance examples call `Datasets::getMnist()`, which in the dependency-free core no longer reads the legacy `data.cereal` archive (it throws at runtime). They are kept for reference only.
- `dnn_examples/` is disabled in [CMakeLists.txt](CMakeLists.txt): it ships a stale ~47 MB `data.cereal` MNIST archive the core can no longer read.
- A few orphaned third-party artifacts remain under `assets/` trees and are not compiled by any target: `transform_examples/assets/AudioFile.h` (Adam Stark, **GPL-3.0** — a license island in this MPL-2.0 tree) and `space_examples/assets/3dparty/serialize/archive.h` (home-grown serialize header, no consumer).

The support status for each area is tracked in [../docs/stability.md](../docs/stability.md). The CI boundary is documented in [../docs/testing-and-ci.md](../docs/testing-and-ci.md).
