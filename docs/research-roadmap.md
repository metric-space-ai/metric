# Research Roadmap

This roadmap translates the revival plan's research directions into a promotion-gated backlog. It keeps research breadth visible without moving unverified code into the stable release contract.

## Promotion Rule

A research direction moves toward the Core Revival API only after it has:

- a documented user problem and record type
- a metric or metric-family assumption
- deterministic fixtures with expected values or contract assertions
- one promoted C++ or Python example
- CI coverage in the relevant release gate
- release notes that describe assumptions, limitations, and compatibility impact

Until those conditions hold, the work remains Beta, Experimental, or Historical as defined in [API Surface](stability.md).

## Priority 1: Diagnostics For Finite Metric Spaces

Goal: make METRIC useful before users commit to a representation or mapping strategy.

Current stable base:

- nearest-neighbor and range-neighbor helpers
- pairwise distance matrices
- entropy examples
- MGC examples
- intrinsic-dimension diagnostics

Next increments:

- document diagnostic result interpretation for sparse, duplicate-heavy, and noisy record sets
- add representative fixtures for strings, histograms, process curves, and mixed records
- expose diagnostic result objects with metadata such as metric name, sample size, radius policy, and warnings
- compare dense-matrix, graph, and tree behavior on the same fixture

Promotion evidence:

- deterministic regression tests in `tests/core_smoke/` or `python/tests/core/`
- promoted examples under `examples/core/` or `python/examples/metric_space/`
- documentation that states when a diagnostic is a heuristic rather than a proof

## Priority 2: Representative Selection And Compression

Goal: choose small, meaningful subsets of a finite metric space without assuming vector centroids.

Current promoted base:

- C++ `metric::operators::representative_indices` and `representatives` expose deterministic farthest-first traversal over finite metric spaces.
- Python `metric.operators.representative_indices` and `representatives` expose the same traversal rule.
- C++ and Python `coverage_representative_indices` / `coverage_representatives` expose deterministic greedy radius coverage.
- Promoted fixtures use string records with edit distance, histogram records with a one-dimensional transport callable, process curves with alignment distance, and structured records with mixed categorical and numeric penalties.

Candidate strategies:

- medoid or k-medoids representatives
- redundancy reduction by distance threshold
- compression summaries that preserve nearest-neighbor behavior

Required fixtures:

- string records with edit distance
- histogram records with transport distance
- process curves with time-series alignment
- structured records with mixed categorical and numeric penalties

Promotion evidence:

- exact expected representatives for small fixtures
- complexity and tie-breaking rules
- examples that show why a vector centroid is not the right object

## Priority 3: Sparse Graph Representations

Goal: make graph representations a first-class runtime choice for finite metric spaces.

Candidate strategies:

- exact k-nearest-neighbor graph construction
- radius graph construction
- symmetrization and weighting policies
- sparsification primitives with documented assumptions
- graph diagnostics such as connectivity, degree distribution, and edge stretch

Promotion evidence:

- consistency tests against dense pairwise distances
- deterministic graph fixtures with expected edge sets
- documentation that distinguishes exact, approximate, directed, undirected, weighted, and normalized graphs

## Priority 4: Cross-Space Dependency Discovery

Goal: compare paired metric spaces when records have different native types.

Candidate strategies:

- MGC over paired finite metric spaces
- distance correlation over paired spaces
- permutation-test wrappers with reproducible seeds
- diagnostics for sample count, duplicate distances, and degenerate spaces

Required fixtures:

- process curves paired with event strings
- histograms paired with structured process records
- vector baseline fixtures for compatibility checks

Promotion evidence:

- deterministic small-fixture regression values
- documented interpretation of correlation and p-value outputs
- examples that avoid implying causality from correlation

## Priority 5: Denoising And Manifold Cleanup

Goal: transform a finite metric space into a cleaner or simpler finite metric space while preserving metric assumptions.

Candidate strategies:

- outlier removal by local neighborhood density
- redundant-record pruning
- graph-based smoothing over metric neighborhoods
- metric-space reconstruction or reverse mapping where an explicit strategy exists

Promotion evidence:

- before/after fixtures with objective expected outcomes
- clear preservation guarantees, or explicit statements that a transform does not preserve metric structure
- examples that report discarded records and diagnostic warnings

## Priority 6: Adapter Layer For Vector Databases

Goal: treat vector databases as storage or search backends, not as the conceptual model.

Candidate strategies:

- adapter for storing explicit metric-space records and metadata
- adapter for using embeddings as one optional representation
- fallback path that preserves the original metric and records
- comparison workflows that show domain metric versus embedding behavior

Promotion evidence:

- adapter interface that never requires the user to discard record identity or metric assumptions
- tests that prove round-tripping of records, IDs, distances, and metadata
- documentation that distinguishes backend search from metric-space modeling

## Priority 7: Benchmark Suite

Goal: compare metric-space workflows against vector or embedding baselines on representative data.

Candidate datasets:

- string edit-distance fixtures
- histogram or empirical-distribution fixtures
- process-curve fixtures
- mixed industrial-record fixtures
- vector-only control fixtures

Benchmark rules:

- correctness and interpretability checks come before speed charts
- every benchmark records metric, representation, runtime policy, seed, hardware, and dataset version
- benchmark publishing stays separate from correctness CI until results are deterministic enough to archive

## Non-Goals During Revival

The revival should not:

- promote neural-network or autoencoder modules as stable APIs before deterministic fixtures exist
- add new algorithm families without documentation and release-gate tests
- imply METRIC is a general vector database
- convert all records to embeddings by default
- make benchmark speedups substitute for metric-space correctness

## Next Candidate Branches

Near-term branches should stay small and evidence-driven:

- add medoid or redundancy-threshold representative fixtures now that farthest-first and radius-coverage fixtures exist
- add graph construction documentation with exact versus approximate terminology
- add MGC interpretation docs for paired metric spaces
- add an industrial-record fixture that combines strings, process curves, histograms, and numeric penalties
