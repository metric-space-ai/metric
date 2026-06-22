# Python Result Objects

Python intent methods return named result objects. Compatibility iteration is
kept where older APIs returned tuples, but new code should use fields and
conversion helpers.

## Common Pattern

Most promoted result objects expose:

| Field Or Method | Meaning |
|---|---|
| `exact` | Whether the result came from the exact promoted path. |
| `strategy` or `algorithm` | Strategy metadata selected by the facade. |
| `representation` | Representation metadata such as `"metric_space"` or `"matrix"`. |
| `diagnostics` | Structured diagnostics when the operation produces them. |
| `to_dict()` | Plain Python data suitable for serialization. |
| `to_numpy()` | Numeric array where the result has numeric payloads. |
| `to_pandas()` | DataFrame conversion when pandas is installed. |

If pandas is missing, `to_pandas()` raises `OptionalDependencyError` with
guidance to install pandas or use `to_dict()`.

## Neighbors

`space.neighbors(query, count=...)` returns `NeighborResult`.

Important fields:

| Field | Meaning |
|---|---|
| `query` | Query record for single-query calls. |
| `query_id` | Query ID when available. |
| `neighbors` | Tuple of `Neighbor` objects for a single query. |
| `rows` | Per-source rows for queryless all-record neighbor calls. |
| `distances` | Ordered distances used by `to_numpy()`. |

Each `Neighbor` has `id`, `record`, `distance`, and `rank`. It also remains
tuple-compatible as `(id, distance)` for older code.

## Groups

`space.groups(...)` returns `ClusteringResult`.

Key fields include `assignments`, `medoids`, `core_records`, `noise_records`,
`cluster_sizes`, `record_count`, `cluster_count`, `noise_count`, `iterations`,
`converged`, `algorithm`, and `representation`.

Labels and medoids are source-record identifiers in the current finite space.
DBSCAN noise uses `noise_label == -1`.

## Embeddings

`space.embed(...)` returns `EmbeddingResult`.

Key fields include `coordinates`, `embedded_space`, `source_space`, `model`,
`source_record_ids`, `dimensions`, `stress`, `trustworthiness`, `strategy`,
`representation`, and `diagnostics`.

The embedded space is a derived `Space` over coordinate records with an explicit
Euclidean metric. The source space remains available through `source_space`.

## Mapping, Denoise, Reduce, And Compress

`space.map(...)` and `space.denoise(...)` return `MappingResult`.
`space.reduce(...)` returns `ReductionResult`. `space.compress(...)` returns
`CompressionResult`.

These result objects preserve lineage through fields such as
`source_record_ids`, `source_record_count`, `target_record_count`,
`assignments`, and `nearest_representative_distances`. Current deterministic
map, denoise, reduction, and compression paths are lossy where reconstruction
would require a fitted inverse map, so `inverse_supported` is `False` and
`inverse_transform(...)` raises `UnsupportedOperationError`.

## Outliers

`space.outliers(...)` returns `OutlierResult`.

Important fields include `outliers`, `record_count`, `cluster_count`,
`noise_count`, `operator_name`, `strategy`, and `representation`. Each
`Outlier` has `record_id` and `score`.

## Compare

`space.compare(...)` and `space.correlate(...)` return `CorrelationResult`.

Important fields include `value`, `statistic_name`, `p_value`, `matched_ids`,
`dropped_left_ids`, `dropped_right_ids`, `align`, `local_scores`,
`left_representation`, and `right_representation`. The default promoted
strategy is distance-profile correlation over exact pairwise distance profiles.

## Structure Diagnostics

`space.describe()` returns `StructureDescription` with finite-space diagnostics:
record count, pair count, zero-distance pair count, minimum nonzero distance,
maximum distance, average distance, intrinsic-dimension estimate, exactness,
strategy, and representation metadata.
