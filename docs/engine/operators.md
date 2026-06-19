# Engine Operators

Operators are implementation-level functions that compute on spaces or representations.

The operator layer is allowed to use algorithmic names because it is not the first user-facing vocabulary. Intent APIs call into operators after strategies and runtime policies select an execution path.

## C++ Operators

The current C++ engine operators include:

- `metric::operators::knn`
- `metric::operators::range`
- `metric::operators::kmedoids`
- `metric::operators::dbscan`
- `metric::operators::affinity_propagation`
- `metric::operators::entropy`
- `metric::operators::mgc`

They return named result objects:

- `NeighborSet`
- `ClusteringResult`
- `EntropyResult`
- `CorrelationResult`

The clustering operators accept a `MetricSpace` directly and, where promoted, engine distance providers such as
`MatrixCache`. Materialized provider overloads keep all result IDs as stable `RecordId` values while reporting
`representation == "distance_provider"`.

## Python Operators

The current Python operator layer includes:

- nearest and range helpers
- exact kNN and radius graph construction
- graph degree, connectivity, and stretch diagnostics
- graph symmetrization and out-degree pruning
- grouping helpers
- representative selection helpers
- medoid and separated-representative helpers
- intrinsic-dimension diagnostics
- structure diagnostics
- cross-space comparison helpers

Python operators return dataclasses such as `ClusteringResult`, `GraphConstructionResult`, `RepresentativeSet`, `StructureDescription`, and `CorrelationResult`.

## Compatibility

Older tuple-returning and class-specific APIs remain compatibility surfaces. New promoted examples should prefer named result objects so outputs can carry metadata and compose with other engine layers.
