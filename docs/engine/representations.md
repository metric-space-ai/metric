# Engine Representations

Representations are execution structures over one metric space. They do not redefine the record set.

```text
MetricSpace -> Representation -> Operator
```

The current C++ engine representation adapters live under `metric::representations`:

- `ImplicitDistanceProvider<Space>`
- `MatrixCache<Space>`
- `CoverTreeIndex<Space>`
- `KnnGraphIndex<Space>`
- `GraphTopology<Space>`

Each adapter captures the source `space.version()` when it is built and exposes `is_stale()`.

## Matrix Cache

`MatrixCache` eagerly stores all pairwise distances. Use it when repeated `RecordId` queries or all-pairs operators make the materialization cost explicit and worthwhile.

```cpp
metric::representations::MatrixCache<decltype(space)> matrix(space);
auto distance = matrix.distance(space.id(0), space.id(1));
```

## Neighbor Indexes

`CoverTreeIndex` and `KnnGraphIndex` currently provide exact deterministic neighbor ordering over finite spaces while preserving the intended engine vocabulary. Their internals can later be replaced by specialized index implementations without changing the representation role.

## Graph Topology

`GraphTopology` stores explicit weighted edges using `RecordId` endpoints. It is the bridge between metric-space distances and sparse graph workflows.

## Python

The stable Python facade exposes:

- `Space.to_matrix()` and `metric.representations.matrix(space)` for explicit matrix materialization
- `Space.to_tree()` and `metric.representations.tree(space)` for an exact tree-style neighbor index over the same records and metric
- `Space.to_graph(count=...)` and `metric.representations.graph(space, count=...)` for an exact kNN graph index with construction metadata

The first Python `TreeIndex` preserves the representation vocabulary and deterministic neighbor semantics while using exact scans internally. The first Python `GraphIndex` wraps the promoted exact kNN graph construction result so sparse local-structure workflows can share the same edge metadata as `metric.operators.exact_knn_graph(...)`.
