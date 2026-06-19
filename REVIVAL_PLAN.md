# METRIC Revival Plan

## North Star

METRIC is a numerical computing framework for finite metric spaces and metric-space algorithms. Vector spaces are supported as a special case, not assumed as the foundation.

The project should be re-established around this claim. The central object is not an embedding vector, a neural network, or a tabular feature matrix. The central object is a finite set of records equipped with a metric. Once a metric is assigned, the dataset becomes a metric space, and METRIC provides numerical representations, operators, mappings, and diagnostics for that space.

This framing matters because many real-world data objects are not naturally vectors:

- time series and process curves
- images and structured pixel grids
- histograms and empirical distributions
- text, symbolic sequences, and event streams
- graphs, trees, and relational structures
- mixed industrial records with domain-specific semantics

Vector-space methods remain important, but they are one case in a broader metric-space setting. METRIC should make that broader setting explicit, programmable, testable, and practical.

## Positioning Shift

### Old Public Impression

The current README and package metadata make the project look like a broad machine-learning framework with C++ performance and Python bindings. That framing competes with scikit-learn, PyTorch, FAISS, Qdrant, and neural-network tooling, where METRIC is not positioned strongly enough.

### New Public Frame

METRIC should be presented as metric-space numerics:

- define metrics for arbitrary record types
- explicitly represent finite metric spaces as matrices, sparse graphs, and search trees
- compute operators on metric spaces, such as entropy, correlation, sparsification, nearest-neighbor search, and intrinsic-dimensionality diagnostics
- map between metric spaces without assuming a vector-space foundation
- use vector spaces only when the data and metric justify that specialization

### One-Sentence Description

METRIC is a C++ and Python framework for numerical algorithms on finite metric spaces, supporting arbitrary data types, explicit metric-space representations, and mappings between spaces.

## Conceptual Pillars

### 1. Metric

A metric is the domain-specific cost of transforming or recoding one record into another record of the same set. For simple vector-valued records this may be Euclidean, Manhattan, cosine, or another norm-induced metric. For structured data it may be edit distance, TWED, EMD, SSIM, graph edit distance, or a custom optimization procedure.

The API should make custom metrics first-class and should document which algorithms require true metrics, pseudo-metrics, or merely distances.

### 2. Space

A finite metric space is a record set plus a metric. METRIC should support both implicit and explicit spaces:

- implicit: records plus a metric functor, distances computed on demand
- matrix: full pairwise distance representation
- graph: sparse metric-space representation
- tree: search/index representation for neighbor access

This is the core numerical layer. It should be independently useful without any ML vocabulary.

### 3. Operators

Operators compute properties of a metric space or transform its representation:

- nearest-neighbor and range search
- graph construction and sparsification
- entropy and intrinsic-dimensionality measures
- correlation between metric spaces
- denoising and redundancy reduction
- representative selection and compression

Operators should document input assumptions, complexity, stability limits, and whether they preserve metric-space structure.

### 4. Mappings

Mappings transform records or spaces into other metric spaces:

- clustering as quantized mapping
- dimensionality or feature extraction as continuous mapping
- reconstruction or reverse mapping where available
- cross-space dependency modeling

This should be framed as metric-space mapping, not generic ML. Neural networks can remain as optional approximators, but they should not define the conceptual center of the project.

## Current State Observations

The repository already contains substantial work:

- broad distance collection, including vector, random/distributional, and structured metrics
- explicit metric-space data structures: matrix, graph, tree
- correlation and entropy machinery
- mapping algorithms, including clustering, SOM/KOC, PCFA, DSPCC, Redif, ESN, autoencoder-related code
- transforms such as wavelets, DCT, HOG, and energy encoders
- C++ tests, Python bindings, examples, benchmarks, and a whitepaper

However, the project is not currently easy to adopt:

- the top-level build expects external CMake packages that are not fully documented
- CMake options are inconsistent (`BUILD_TESTS` vs. `METRIC_BUILD_TESTS`)
- Python packaging targets old Python versions and old pybind11
- README examples are not copy/paste safe
- the public language still suggests a generic ML framework
- several test groups or important assertions are disabled
- Python API coverage is partial and sometimes inconsistent with the C++ concepts

The first revival phase must therefore focus on clarity and reproducibility before new algorithms are added.

## Revival Workstreams

## Workstream A: Reframe the Project Publicly

Goal: make the metric-space numerics scope obvious within the first screen of the repository.

Tasks:

- rewrite the README opening around the new North Star
- replace "machine learning framework" as the primary phrase with "numerical computing framework for finite metric spaces"
- explain clearly that vector spaces are supported special cases
- introduce the four core concepts: Metric, Space, Operator, Mapping
- move historical AI/industrial narrative into a separate background section
- add a short "When should I use METRIC?" section
- add a short "When should I not use METRIC?" section
- correct all first-page examples so they compile or run
- update repository description, package metadata, and badges

Acceptance criteria:

- a new user can explain the distinction between METRIC and vector-search libraries after reading the first page
- first C++ and Python examples run in CI
- README does not imply that METRIC is competing primarily with neural-network frameworks

## Workstream B: Restore Reproducible Builds

Goal: the repository builds from a clean checkout on current Linux and macOS without manual dependency archaeology.

Tasks:

- normalize CMake options:
  - `METRIC_BUILD_TESTS`
  - `METRIC_BUILD_EXAMPLES`
  - `METRIC_BUILD_BENCHMARKS`
  - `METRIC_BUILD_PYTHON`
- remove or alias the stale `BUILD_TESTS` option
- choose a dependency strategy:
  - package-manager mode for system installs
  - optional `FetchContent` or vendored fallback for developer onboarding
- document required dependencies for C++ core separately from examples and Python bindings
- add a clean configure preset for default developer builds
- add a minimal "core only" build profile
- verify install/export targets with a tiny downstream CMake consumer

Acceptance criteria:

- `cmake --preset dev` configures on a clean supported system
- `cmake --build --preset dev` completes
- `ctest --preset dev` runs tests
- install package can be consumed by `find_package(panda_metric)` or a renamed package target

## Workstream C: Modernize Python Packaging

Goal: Python users can install and use METRIC without compiling the world manually.

Tasks:

- move Python packaging to `pyproject.toml`
- evaluate `scikit-build-core` for CMake-backed wheels
- update pybind11 to a current supported version
- define supported Python versions explicitly
- build wheels for current CPython versions on Linux, macOS, and Windows
- ensure NumPy array handling is stable and documented
- separate pure Python convenience modules from compiled extension modules
- remove TestPyPI install instructions from the main path
- publish a modern package under a deliberate name

Potential package names:

- `metric-space`
- `metric-space-numerics`
- `metric-py` only if ownership and continuity are intentional

Acceptance criteria:

- `pip install .` works from a clean checkout
- wheel builds are CI artifacts
- a user can run Python examples without editing library paths
- Python API exposes the core concepts rather than only scattered wrappers

## Workstream D: Define the Core API

Goal: make the conceptual model visible in both C++ and Python APIs.

Proposed C++ layers:

- `metric::Metric`
- `metric::Space`
- `metric::MatrixSpace`
- `metric::GraphSpace`
- `metric::TreeSpace`
- `metric::operators::*`
- `metric::mappings::*`

Proposed Python layers:

- `metric.metrics`
- `metric.spaces`
- `metric.operators`
- `metric.mappings`
- `metric.transforms`

Tasks:

- audit current public classes and functions
- mark stable vs experimental APIs
- introduce compatibility aliases where names are historically misspelled, such as `Manhatten`
- standardize naming around "record", "metric", "space", "operator", and "mapping"
- decide whether "PANDA" remains historical branding or is removed from public API
- document requirements for metric functors
- document which algorithms accept custom callables

Acceptance criteria:

- there is a documented minimal API for constructing a finite metric space
- examples use the same vocabulary across C++ and Python
- old APIs can remain, but the recommended path is coherent

## Workstream E: Make Metric-Space Numerics Testable

Goal: test the mathematical contracts, not only compilation.

Tasks:

- add property tests for metric requirements:
  - non-negativity
  - identity / pseudo-metric identity caveat
  - symmetry
  - triangle inequality where required
- add tests for matrix, graph, and tree consistency
- add tests comparing explicit and implicit distance computation
- restore disabled MGC tests with asserted expected values
- add regression tests for entropy and intrinsic-dimensionality examples
- add tests for user-defined metrics
- add small fixtures for non-vector record types
- keep slow industrial examples outside default CI

Acceptance criteria:

- default tests catch wrong metric-space behavior
- disabled test blocks are either restored or documented as intentionally out of scope
- CI has a fast path and an extended path

## Workstream F: Documentation and Examples

Goal: teach metric-space thinking before listing algorithms.

Required docs:

- `docs/concepts/metric-space.md`
- `docs/concepts/finite-metric-space.md`
- `docs/concepts/vector-space-as-special-case.md`
- `docs/concepts/explicit-representations.md`
- `docs/concepts/metrics-as-recoding-costs.md`
- `docs/api/cpp.md`
- `docs/api/python.md`
- `docs/examples/custom-metric.md`
- `docs/examples/structured-data.md`
- `docs/examples/correlation-between-spaces.md`
- `docs/examples/industrial-anomaly-workflow.md`

First examples to make excellent:

- custom string/edit metric space
- time-series space using TWED
- image/histogram space using EMD or SSIM
- metric-space entropy / intrinsic dimension
- MGC between two different data modalities
- explicit Matrix vs Graph vs Tree representation

Acceptance criteria:

- every promoted example is executed in CI
- examples are small, readable, and clearly non-vector-only where possible
- docs consistently distinguish vector-space methods from metric-space methods

## Workstream G: Separate Stable Core from Experimental Research

Goal: keep the project credible while preserving research breadth.

Proposed stability tiers:

- Stable:
  - distance metrics
  - explicit spaces
  - nearest-neighbor search
  - entropy and MGC
  - graph sparsification primitives
- Beta:
  - mappings with active tests and examples
  - structured metrics with known limitations
  - transforms used in documented workflows
- Experimental:
  - neural-network approximators
  - autoencoder/DNN code
  - domain-specific demos
  - incomplete graph edit / Hausdorff / Riemannian extensions

Tasks:

- label modules by stability
- move old demos and incomplete experiments into clearly marked folders
- avoid advertising experimental code as core capability
- preserve historical work without letting it obscure the main project

Acceptance criteria:

- users can identify what is production-ready, research-grade, and historical
- README focuses on stable core
- experimental code no longer blocks installation or basic tests

## Workstream H: Modern CI and Release Process

Goal: make maintenance routine instead of heroic.

Tasks:

- update GitHub Actions versions
- define Linux/macOS/Windows C++ matrix
- define Python wheel matrix
- add clang-format or formatting check
- add static analysis only after build is stable
- add release checklist
- publish source archive and wheels from tags
- archive benchmark results separately from correctness CI

Acceptance criteria:

- every pull request proves core build, tests, and docs examples
- releases are reproducible
- release notes distinguish API changes, algorithm changes, and packaging changes

## Workstream I: Research and Product Direction

Goal: develop beyond restoration without losing focus.

Promising directions:

- metric-space intrinsic dimension as a diagnostic for industrial datasets
- representative selection and compression of finite metric spaces
- cross-space correlation and dependency discovery
- sparse graph representations of metric spaces
- metric-space denoising and manifold cleanup
- adapter layer for vector databases as storage/search backends, not as the conceptual model
- integrations with scikit-learn as wrappers around metric-space operators
- benchmark suite comparing vector embeddings against domain metrics on structured datasets

Non-goals for the revival phase:

- competing with PyTorch as a neural-network framework
- building a general vector database
- turning all data into embeddings by default
- adding new algorithms before the build, API, and docs are trustworthy

## Suggested Milestones

### Milestone 0: Repo Boots

Duration: 1-2 weeks.

Deliverables:

- CMake options fixed
- dependency docs corrected
- clean C++ configure/build/test path
- README opening replaced with new framing
- first examples corrected

### Milestone 1: Python Re-entry

Duration: 2-4 weeks.

Deliverables:

- `pyproject.toml`
- modern pybind11
- basic wheels in CI
- Python smoke examples
- minimal Python API for metrics and spaces

### Milestone 2: Core Metric-Space API

Duration: 3-6 weeks.

Deliverables:

- stable namespaces and recommended API
- Matrix/Graph/Tree examples
- metric contract tests
- MGC/entropy tests restored
- docs for finite metric spaces

### Milestone 3: Public Relaunch

Duration: 2-3 weeks.

Deliverables:

- refreshed README
- documentation site
- tagged release
- PyPI package
- examples gallery
- short technical manifesto explaining why metric-space numerics is broader than vector-search workflows

### Milestone 4: Research Roadmap

Duration: ongoing.

Deliverables:

- prioritized backlog for mappings, compression, denoising, and cross-space analytics
- benchmark datasets
- comparison against embedding-only baselines
- industrial anomaly workflow demo

## Immediate Next Changes

The first branch should avoid broad refactoring. It should make the project understandable and runnable.

Recommended first pull request:

1. Add this revival plan.
2. Rewrite the README title block and opening section.
3. Fix the first C++ example or replace it with a minimal compiling finite metric-space example.
4. Rename or alias CMake test options so `cmake ..` behaves as documented.
5. Add one CI job that configures, builds, and runs a small core test target.

Recommended second pull request:

1. Add `docs/concepts/finite-metric-space.md`.
2. Add one Python and one C++ example using non-vector data.
3. Restore one disabled correctness test for MGC or entropy.
4. Split "stable" and "experimental" module lists in the README.

Recommended third pull request:

1. Start Python packaging modernization.
2. Upgrade pybind11.
3. Add wheel build artifacts.
4. Add Python smoke tests for `metrics`, `spaces`, and one operator.

## Decision Points

Open questions to settle early:

- Should the public package name remain `panda_metric`, become `metric`, or move to a clearer name such as `metric-space`?
- Should `PANDA` remain as historical origin, or should new public docs use only `METRIC`?
- Which modules are stable enough for the first relaunch release?
- Which Python versions should be supported?
- Should dependency management prioritize system packages, vendored fallback, or both?
- Should DNN/autoencoder code remain in-tree as experimental, move to a separate repo, or be archived?

## Success Definition

The revival succeeds when a technical user can:

1. install METRIC without manual dependency hunting
2. define a custom metric for a non-vector record type
3. build an explicit finite metric space
4. run at least one operator on that space
5. understand why this is broader than vector-space or embedding-only workflows
6. cite stable documentation for the mathematical assumptions and algorithmic limits

The core message should remain stable throughout the revival:

METRIC is about computing with metric spaces directly. Vector spaces are useful, but they are not the foundation.
