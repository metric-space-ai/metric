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
| `metric_status` | Target metric status where the result creates or retains a metric space. Python callable metrics report `"unknown"` unless the facade can prove more. |
| `validity` | Human-readable statement of what the result preserves and what assumptions apply. |
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

Each `Neighbor` has `id`, `record`, `distance`, and `rank`.

## Groups

`space.groups(...)` returns `ClusteringResult`.

Key fields include `assignments`, `medoids`, `core_records`, `unassigned_records`,
`cluster_sizes`, `record_count`, `cluster_count`, `unassigned_count`, `iterations`,
`converged`, `algorithm`, and `representation`.

Labels and medoids are source-record identifiers in the current finite space.
DBSCAN density-unassigned records are records with `unassigned_label == -1` in the concrete DBSCAN grouping output.

## Embeddings

`space.embed(...)` returns `EmbeddingResult`.

Key fields include `coordinates`, `embedded_space`, `source_space`, `mapping_artifact`,
`source_record_ids`, `dimensions`, `stress`, `trustworthiness`, `strategy`,
`representation`, and `diagnostics`.

The embedded space is a derived `Space` over coordinate records with an explicit
Euclidean metric. The source space remains available through `source_space`.

## Mapping, Thinning, Density Filtering, Reduce, And Compress

`space.map(...)` and `space.density_filter(...)` return `MappingResult`.
`space.thin(...)`, `space.distribution_sample(...)`, and `space.equalize(...)`
also return `MappingResult` over retained source records; uniform-density
thinning/equalization populates `diagnostics` with coverage,
nearest-neighbor-density drift, and local-volume count/density drift fields.
`space.reduce(...)` returns `ReductionResult`. `space.compress(...)` returns
`CompressionResult`.

These result objects preserve lineage through fields such as
`source_record_ids`, `source_record_count`, `target_record_count`,
`assignments`, `nearest_representative_distances`, representative
multiplicities, and normalized representative weights. For
distribution-preserving thinning, no full-source assignment map is implied; the
retained records carry normalized sample weights. For uniform-density thinning
and equalization, assignments, representative multiplicities, normalized
weights, coverage radius, and average assignment distance are top-level fields
and are also mirrored in diagnostics. `CompressionResult` reports the same
measure fields derived from source-to-representative assignments, so callers can
treat the compressed space as a weighted finite metric-measure summary.
Compression and mapping-style modifiers report `metric_status` and `validity`;
promoted subset paths use retained source records, while Python callable metrics
keep `metric_status="unknown"` unless a stronger target-law status is already
known. Current
deterministic map, density-filter, reduction, and compression paths are lossy
where reconstruction would require a derived inverse map, so `inverse_supported`
is `False` and `inverse_transform(...)` raises `UnsupportedOperationError`.

## Outliers

`space.outliers(...)` returns `OutlierResult`.

Important fields include `outliers`, `record_count`, `cluster_count`,
`unassigned_count`, `operator_name`, `strategy`, and `representation`. Each
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
