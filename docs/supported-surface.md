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
- **Clustering:** `kmedoids`, `dbscan`, `affinity_propagation` — and only these.
- **Outliers:** DBSCAN-noise and k-NN-distance scoring — and only these.
- **Representatives / reduction / compression:** farthest-first, coverage, k-medoids.
- **Dependence:** MGC significance between aligned spaces (a dependence test,
  never a metric).
- **Derived coordinate spaces (maps):** PCFA (linear), the native autoencoder,
  and PHATE-AE. Classical MDS is available as the embedding baseline.
- **I/O:** `mtrc::record::read_csv` / `read_tsv` / `write_csv` / `write_tsv`;
  record import/export/compose/group.
- **Inspection:** every `*Result` streams a one-line summary (`operator<<` /
  `mtrc::summary`); `mtrc::render::write_csv` / `to_csv` / `write_histogram` for
  distance matrices, embeddings, and value distributions.

## Out of scope (not provided)

These are deliberately absent; do not expect them in the supported surface:

- Neighbor-graph manifold embeddings (UMAP / t-SNE / Isomap / LLE). Only PCFA,
  the native autoencoder, PHATE-AE, and classical MDS exist.
- Broad clustering families beyond the three above (no k-means, GMM, spectral,
  or hierarchical clustering in the promoted surface).
- Supervised learning (no classification or regression). METRIC is an
  unsupervised / structural-analysis framework. A metric k-NN classifier can be
  built on top of the search surface, but none ships.
- Time-series forecasting / changepoint / seasonal tooling. Time series are
  supported only as records under elastic distances (TWED/ERP).
- Anomaly families beyond DBSCAN-noise + k-NN-distance (no LOF / isolation-forest
  / one-class).

## Legacy / unpromoted (present but not supported)

- `<metric/mapping.hpp>` is a deprecated legacy umbrella (ESN, KOC, SOM,
  ensembles, kmeans, hierarchical clustering). It is not covered by the core test
  gate. Use `<metric/stats.hpp>` / `<metric/modify.hpp>` instead.
- The `reduce()` SOM / KOC / DSPCC strategy options and the KOC mapping
  (`KOCMapping` / `KOCModel` / `koc()`) are constructible but not promoted and
  raise at runtime. Use `pcfa_options`, the native autoencoder, or PHATE-AE.

## Quarantined non-metrics

Some catalog distribution functions are NOT true metrics and are quarantined:
Sorensen, Cosine (use Angular), RandomEMD, SSIM, Kohonen, CramervonMises,
KolmogorovSmirnov. They remain constructible for compatibility but are not
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
