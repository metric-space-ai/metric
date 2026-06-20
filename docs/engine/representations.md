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

The factory helpers mirror the Python facade and keep examples focused on the
representation role:

- `implicit(space)`
- `matrix(space)`
- `matrix(space, matrix_cache_mode::lazy)`
- `cover_tree(space)`
- `knn_graph(space, k)` / `graph(space, k)`
- `topology(space)`

Search strategies can also materialize the matching representation explicitly:

```cpp
auto implicit = metric::representations::make(space, metric::strategies::brute_force{});
auto matrix = metric::representations::make(space, metric::strategies::matrix_cache{});
auto tree = metric::representations::make(space, metric::strategies::cover_tree{});
auto graph = metric::representations::make(space, metric::strategies::knn_graph(3));
```

Each adapter captures the source `space.version()` when it is built and exposes `is_stale()`.

## Matrix Cache

`MatrixCache` can eagerly store all pairwise distances or fill entries lazily. Use eager materialization when repeated `RecordId` queries or all-pairs operators make the upfront cost explicit and worthwhile. Use the lazy mode when the workflow needs distance-provider semantics but should only compute requested pairs.

```cpp
auto matrix = metric::representations::matrix(space);
auto distance = matrix.distance(space.id(0), space.id(1));

auto lazy_matrix = metric::representations::matrix(
    space,
    metric::representations::matrix_cache_mode::lazy);
auto first = lazy_matrix.distance(space.id(0), space.id(2));  // miss
auto again = lazy_matrix.distance(space.id(0), space.id(2));  // hit
auto stats = lazy_matrix.stats();
```

`stats()` reports cache hits, misses, fill ratio, and whether symmetric storage is used. `diagnostics()` reports whether the matrix cache was eager/materialized or lazy, the number of cached distances, the number of evaluated distances, memory estimates, and stale-version warnings.

## Neighbor Indexes

`CoverTreeIndex` and `KnnGraphIndex` currently provide exact deterministic neighbor ordering over finite spaces while preserving the intended engine vocabulary. Their internals can later be replaced by specialized index implementations without changing the representation role.

`KnnGraphIndex::sampled_recall(provider, sample_count)` and `KnnGraphIndex::stats_against(provider, sample_count)` compare graph neighbors with an exact distance provider, usually `MatrixCache`, when recall evidence is needed. This validation is explicit so approximate-quality checks do not hide extra all-pairs work.

## Graph Topology

`GraphTopology` stores explicit weighted edges using `RecordId` endpoints. It is the bridge between metric-space distances and sparse graph workflows.

## Python

The stable Python facade exposes:

- `Space.to_matrix()` and `metric.representations.matrix(space)` for explicit matrix materialization
- `Space.to_tree()` and `metric.representations.tree(space)` for an exact tree-style neighbor index over the same records and metric
- `Space.to_graph(count=...)` and `metric.representations.graph(space, count=...)` for an exact kNN graph index with construction metadata

The first Python `TreeIndex` preserves the representation vocabulary and deterministic neighbor semantics while using exact scans internally. The first Python `GraphIndex` wraps the promoted exact kNN graph construction result so sparse local-structure workflows can share the same edge metadata as `metric.operators.exact_knn_graph(...)`.

The promoted Python representation-swap example lives at `python/examples/engine/representation_swap.py`. It uses one string/edit `Space`, then inspects the implicit path, matrix materialization, exact tree-style neighbor lookup, exact kNN graph adjacency, runtime diagnostics, and stale-representation checks over the same record IDs.
