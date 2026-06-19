# Python API

The current Python core API exposes metric constructors, a minimal `Space` facade, finite-space helpers, and small operator helpers. It is intentionally small while the broader engine facade is restored.

The stable entry point is `Space`: a finite record set plus a metric with cached pairwise distances and intent-named helpers for neighbors, groups, representatives, cross-space comparison, and structure diagnostics. `FiniteMetricSpace` and `MatrixSpace` remain available for explicit representation vocabulary, and `Space.to_matrix()` returns an explicit finite matrix-space view.

## Basic Use

```python
from metric import Edit, Space

records = ["cat", "cot", "coat", "dog"]
space = Space(records, Edit())

print(space.distance(0, 1))
print(space.neighbors("cut", k=2))
```

The records are strings. Edit distance defines the geometry without an embedding step.

## Core Objects

- `metrics`: standard metric constructors
- `Space`: minimal intent-first finite metric space facade
- `FiniteMetricSpace`: finite metric space over records
- `MatrixSpace`: compatibility alias for `FiniteMetricSpace`
- `operators`: small helpers for pairwise distances, nearest neighbors, range neighbors, exact graph results and edges, graph connectivity diagnostics, graph degree diagnostics, graph stretch diagnostics, graph pruning, grouping, representative selection, cross-space comparison, medoids, separated representatives, and intrinsic-dimension diagnostics
- `strategies`: strategy objects for intent methods, starting with `KMedoids`, `DBSCAN`, `FarthestFirst`, and `DistanceProfileCorrelation`
- `mappings`: beta compatibility bridge for installed mapping bindings
- `transforms`: beta compatibility bridge for installed transform bindings

## Core Methods

```python
len(space)
space[index]
space.distance(lhs_index, rhs_index)
space.pairwise_distances()
space.to_matrix()
space.neighbors(query, k=10)
space.nearest(query)
space.within_radius(query, radius=1)
space.groups(strategy=KMedoids(groups=2))
space.groups(strategy=DBSCAN(radius=1, min_points=2))
space.compare(other_space, strategy=DistanceProfileCorrelation())
space.correlate(other_space)
space.representatives(k=3)
space.representatives(k=3, strategy=FarthestFirst(seed_index=1))
space.describe()
space.describe_structure()
space.knn(query, k=10)
space.nn(query)
space.rnn(query, radius=1)
```

## Operators

```python
from metric.operators import (
    ClusteringResult,
    CorrelationResult,
    GraphConstructionMetadata,
    GraphConstructionResult,
    GraphConnectivityDiagnostics,
    GraphDegreeDiagnostics,
    GraphStretchDiagnostics,
    RepresentativeSet,
    StructureDescription,
    coverage_representative_indices,
    coverage_representatives,
    compare_spaces,
    correlate_spaces,
    dbscan,
    describe_structure,
    exact_knn_graph,
    exact_knn_graph_edges,
    exact_radius_graph,
    exact_radius_graph_edges,
    find_groups,
    find_representatives,
    graph_connectivity_diagnostics,
    graph_degree_diagnostics,
    graph_stretch_diagnostics,
    intrinsic_dimension,
    kmedoids,
    medoid,
    medoid_index,
    nearest_neighbors,
    pairwise_distance_matrix,
    prune_graph_out_degree,
    range_neighbors,
    representative_indices,
    representatives,
    separated_representative_indices,
    separated_representatives,
    symmetrize_graph,
)
from metric.strategies import DBSCAN, DistanceProfileCorrelation, FarthestFirst, KMedoids

distances = pairwise_distance_matrix(records, Edit())
neighbors = nearest_neighbors(records, Edit(), "cut", k=2)
close = range_neighbors(records, Edit(), "cut", radius=1)
knn_graph = exact_knn_graph(records, Edit(), k=1)
knn_edges = exact_knn_graph_edges(records, Edit(), k=1)
radius_graph = exact_radius_graph(records, Edit(), radius=1)
radius_edges = exact_radius_graph_edges(records, Edit(), radius=1)
connectivity_info = graph_connectivity_diagnostics(knn_graph)
degree_info = graph_degree_diagnostics(knn_graph)
stretch_info = graph_stretch_diagnostics(records, Edit(), radius_graph)
undirected = symmetrize_graph(knn_graph, policy="union", weighting="minimum_distance")
pruned = prune_graph_out_degree(exact_knn_graph(records, Edit(), k=2), max_out_degree=1)
groups = find_groups(records, Edit(), KMedoids(groups=2))
density_groups = find_groups(records, Edit(), DBSCAN(radius=1, min_points=2))
dependency = compare_spaces(records, Edit(), other_records, other_metric, DistanceProfileCorrelation())
selected_ids = representative_indices(records, Edit(), k=2)
selected_records = representatives(records, Edit(), k=2)
center_id = medoid_index(records, Edit())
center_record = medoid(records, Edit())
separated_ids = separated_representative_indices(records, Edit(), minimum_distance=2)
separated_records = separated_representatives(records, Edit(), minimum_distance=2)
covered_ids = coverage_representative_indices(records, Edit(), radius=1)
covered_records = coverage_representatives(records, Edit(), radius=1)
dimension = intrinsic_dimension(records, Edit())
representative_result = find_representatives(records, Edit(), k=2, strategy=FarthestFirst(seed_index=0))
structure = describe_structure(records, Edit())
```

`find_groups` returns a `ClusteringResult` with source-record assignments, medoid record IDs, cluster sizes, optional DBSCAN core/noise records, iteration metadata, algorithm metadata, and representation metadata. `Space.groups(...)` exposes the same result from the `Space` facade. Passing an integer group count selects deterministic `KMedoids`; passing `KMedoids` or `DBSCAN` makes the strategy explicit.

`compare_spaces` returns a `CorrelationResult` for two aligned finite metric spaces with the same record count. The first Python-core strategy is `DistanceProfileCorrelation`, which computes the Pearson correlation of upper-triangular pairwise distance profiles. `Space.compare(...)` and `Space.correlate(...)` expose the same result with metric-space representation metadata.

`representative_indices` and `representatives` use deterministic farthest-first traversal over the finite metric space. They select existing records rather than vector centroids, start from `seed_index=0` by default, and resolve equal-distance ties by record order.

`find_representatives` returns a `RepresentativeSet` with selected source indices, nearest-representative distances for every record, coverage radius, average nearest-representative distance, strategy metadata, and representation metadata. `Space.representatives(...)` exposes the same result from the `Space` facade.

`medoid_index` and `medoid` select the existing record with the smallest total distance to all records. Equal total-distance ties are resolved by record order.

`separated_representative_indices` and `separated_representatives` scan records in order and keep a candidate when it is at least `minimum_distance` from every selected representative. This is deterministic redundancy-threshold reduction, not an optimal packing proof.

`exact_knn_graph` and `exact_radius_graph` return `GraphConstructionResult` objects with `.edges` and `.metadata`. The metadata records the construction strategy, record count, edge count, directed/self-loop/exact policy, the active `k` or `radius` parameter, edge payload meaning, sparsification policy, symmetrization policy, normalization policy, and the tie-breaking rule. `exact_knn_graph_edges` and `exact_radius_graph_edges` remain convenience helpers that return only directed edge tuples shaped as `(source_index, target_index, distance)`. Exact graph helpers exclude self-loops, preserve source-record order, and resolve equal kNN distances by target record order.

`graph_connectivity_diagnostics` returns `GraphConnectivityDiagnostics` for a graph construction result. It reports deterministic component labels, component count, isolated-record count, largest component size, connected status, and connectivity policy. Directed graph results use weak undirected reachability over stored edges; undirected graph results use endpoint reachability.

`graph_degree_diagnostics` returns `GraphDegreeDiagnostics` for a graph construction result. Directed graphs report `out_degrees`, `in_degrees`, combined endpoint `degrees`, isolated count, max degree, average degree, and degree policy `directed_in_out`. Undirected graph results report endpoint `degrees` with zero-filled in/out vectors and degree policy `undirected_endpoint`.

`graph_stretch_diagnostics` returns `GraphStretchDiagnostics` for a graph construction result plus the source records and metric. It computes shortest-path distances over the stored graph, compares them to the metric distance, and reports evaluated pairs, reachable pairs, unreachable pairs, zero-metric pairs, max stretch, average stretch over reachable pairs, and stretch policy. Directed graph results use directed shortest paths; undirected graph results use bidirectional endpoint paths.

`symmetrize_graph` converts a graph construction result to undirected `source_index < target_index` edges. The supported symmetrization policies are `union` and `mutual`; the supported reciprocal weighting policies are `minimum_distance` and `maximum_distance`. The returned metadata records the selected symmetrization and weighting policies.

`prune_graph_out_degree` keeps at most `max_out_degree` existing directed edges per source record. It sorts each source's candidate edges by `(distance, target_index)`, keeps the first entries, and records `sparsification="out_degree"` plus `max_out_degree` in the returned metadata. It does not compute new distances and rejects already-undirected graph results because their stored source index is not an out-degree contract.

`coverage_representative_indices` and `coverage_representatives` use deterministic greedy radius coverage. They scan records in order, choose the first uncovered record as a representative, and mark every record within `radius` as covered.

`intrinsic_dimension` returns an expansion-dimension estimate based on finite-space neighborhood growth. Treat it as a diagnostic that depends on the metric, sample density, duplicates, and available radii.

`describe_structure` returns a `StructureDescription` with record count, evaluated pair count, zero-distance pair count, minimum nonzero distance, maximum distance, average distance, and intrinsic-dimension estimate. `Space.describe()` and `Space.describe_structure()` expose the same diagnostics from the `Space` facade.

## Custom Metrics

```python
def padded_hamming(lhs: str, rhs: str) -> int:
    width = max(len(lhs), len(rhs))
    return sum(
        (lhs[i] if i < len(lhs) else "_") != (rhs[i] if i < len(rhs) else "_")
        for i in range(width)
    )

space = Space(["red", "reed", "road", "blue"], padded_hamming)
print(space.neighbors("read", k=2))
```

## NumPy Records

NumPy arrays are accepted as records when the supplied metric callable accepts the array objects it receives. The revived Python facade does not force records into vectors or apply an implicit embedding step:

```python
import numpy as np
from metric import Space

records = np.array([[0.0, 0.0], [3.0, 4.0], [6.0, 8.0]])

def euclidean(lhs, rhs) -> float:
    return float(np.linalg.norm(lhs - rhs))

space = Space(records, euclidean)
print(space.distance(0, 1))
```

`pairwise_distances()` and `metric.operators.pairwise_distance_matrix()` currently return Python lists of lists. Use `np.asarray(...)` in user code when an ndarray result is needed.

`to_matrix()` returns an independent `FiniteMetricSpace` / `MatrixSpace` view with its own cached pairwise distances. It is useful when code wants to make materialization explicit while keeping the same records and metric callable.

## Engine Roadmap

The implemented facade currently covers neighbor access, grouping, cross-space comparison, representative selection, and structure diagnostics. Additional intent names such as `embed`, `map`, `reduce`, `denoise`, and `outliers` describe the public direction and should be promoted only when they are backed by stable strategies, result objects, examples, and CI.

## Compatibility

Compatibility modules remain importable for existing code. New Python examples use `metric.metrics`, `metric.spaces`, and `metric.operators`.

`metric.mappings` and `metric.transforms` are stable import locations for beta compatibility surfaces:

```python
import metric

print(metric.mappings.STABILITY)
print(metric.mappings.available())
print(metric.transforms.available())
```

They intentionally do not promote mapping or transform algorithms into the core wheel contract. They report the public names that the installed wheel exposes and raise `ImportError` from `legacy_module()` when a legacy binding is not present.
