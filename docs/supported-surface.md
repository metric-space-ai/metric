# Supported Surface, Scope & Scale

This page states plainly what METRIC supports today, what is intentionally out of
scope, what is legacy/unpromoted, and how to choose a representation at scale. It
is the single reference for "is X available, and does it run?" so the runtime
surface never surprises you.

## Promoted (supported) surface

- **Records & metrics:** the catalog true metrics (Euclidean, Manhattan,
  Chebyshev, Minkowski/P-norm, Edit, transport/EMD-Wasserstein, TWED/ERP-style
  alignment, Ruzicka/weighted-Jaccard, composed structured-record metrics) plus
  user metrics via `mtrc::make_metric` / `mtrc::make_true_metric`.
- **Space:** `mtrc::make_space`, the `SpaceBuilder` (opt-in `require_non_empty`,
  `require_unique_records`, `require_true_metric`, `require_uniform_dimension`,
  `require_finite`), `build_checked`, subspace/merge, persistence save/load.
- **Search:** exact k-NN, nearest, and range queries.
- **Structure / stats:** `describe_structure`, `profile`, `entropy`,
  `intrinsic_dimension`, `regular_sample`, distance distribution.
- **Grouping:** `find_groups` / `Space.groups(...)` over finite metric spaces,
  with representative, radius-density, and affinity-propagation strategies behind
  that surface.
- **Singular-record scoring:** density-unassigned records and k-NN-distance
  scoring — and only these.
- **Representatives / reduction / compression / thinning:** farthest-first,
  coverage/k-center, radius coverage, medoid representatives,
  distribution-preserving thinning, uniform-density radius nets, equalization by
  uniform-density radius nets.
- **Dependence:** MGC significance between aligned spaces (a dependence test,
  never a metric).
- **Derived coordinate spaces (maps):** PCFA (linear), solver-backed
  coordinate solver coordinate maps, and parametric diffusion coordinate. Classical MDS is available as a
  coordinate-space baseline.
- **I/O:** `mtrc::record::read_csv` / `read_tsv` / `write_csv` / `write_tsv`;
  record import/export/compose/group.
- **Inspection:** every `*Result` streams a one-line summary (`operator<<` /
  `mtrc::summary`); `mtrc::render::write_csv` / `to_csv` / `write_histogram` for
  distance matrices, embeddings, and value distributions.

## Out of scope (not provided)

These are deliberately absent; do not expect them in the supported surface:

- Neighbor-graph manifold embeddings (UMAP / t-SNE / Isomap / LLE). Only PCFA,
  the parametric coordinate solver, parametric diffusion coordinate, and classical MDS exist.
- Broad grouping families beyond the promoted finite-space strategies (no
  k-means, GMM, spectral, or hierarchical clustering in the promoted surface).
- Classification and regression surfaces. METRIC is not a supervised or
  unsupervised ML collection; it is a library for computing with finite metric
  spaces. A classifier can be built on top of a metric-space search surface, but
  it is an application layer and none ships here.
- Time-series forecasting / changepoint / seasonal tooling. Time series are
  supported only as records under elastic distances (TWED/ERP).
- Anomaly-detection families beyond the promoted singular-record scores (no LOF /
  isolation-forest / one-class).

## Historical Sources

- The old broad mapping umbrella is no longer part of the promoted surface or
  default build. Use `<metric/stats.hpp>` / `<metric/modify.hpp>` /
  `<metric/engine.hpp>` instead.
- Placeholder strategy options for old mapping algorithms were removed from
  `mtrc::modify`. A future operator must enter through a finite-metric-space
  contract with lineage, metric-status reporting, diagnostics, and tests.
- The [Metric-Space Research Track](engine/metric-space-research-track.md)
  lists experimental finite-metric-space candidates such as graph spanners,
  spectral graph sparsifiers, metric-measure drift, adaptive equalization, and
  quotient hierarchies. Those entries are not supported APIs until promoted here.

## Quarantined non-metrics

Some catalog distribution functions are NOT true metrics and are quarantined:
Sorensen, Cosine (use Angular), RandomEMD, SSIM, CramervonMises,
KolmogorovSmirnov. They remain explicit quarantine entries and are not
admitted by metric-geometry operators; `mtrc::metric` discovery reports their
admission status and the admitted alternative. Prefer the admitted metric named
by discovery (e.g. Wasserstein/EMD instead of RandomEMD, Angular instead of
Cosine). RandomEMD in particular has degenerate-input failure modes — guard
empty / zero-range inputs before using it.

## Scale & performance

METRIC is built for finite metric spaces that fit in memory. Choose a
representation deliberately:

| Need | Use | Cost |
| --- | --- | --- |
| One/few queries, large n | `LiveDistances` (compute on demand) | O(n) per query, O(1) memory |
| Many queries, moderate n | eager `DistanceTable` (materialize) | O(n^2) time + memory |
| Repeated neighbor queries | cover-tree index | build once, amortized queries |

Guidance:

- The eager `DistanceTable` is a dense O(n^2) array. For ~20k records that is
  multiple GB. Do not materialize a large space blindly; prefer `LiveDistances`
  or a cover-tree below your memory budget, and only materialize when you will
  reuse the full matrix.
- **Threading:** there is no real parallelism in the space/search/stats path yet.
  `execution::parallel` is a declared preference that does not currently
  accelerate work (it only gates a thread-safety check). Do not assume multi-core
  speedup.
- **Incremental updates:** mutating a space invalidates its derived indexes/
  tables; recovery currently rebuilds them. Batch mutations before querying.
- **Out-of-core / streaming:** not supported. The whole record set and any
  materialized table live in RAM. Chunk your data at the application level if it
  exceeds memory.
