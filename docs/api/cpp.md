# C++ API

The current C++ core API exposes metric constructors, a minimal `metric::Metric` wrapper for custom callables, a minimal `metric::Space` facade, the first `metric::MetricSpace` engine layer, and explicit finite-space representations. The broader intent facade is part of the engine roadmap; promoted examples use the CI-tested core surface.

Recommended includes:

```cpp
#include <metric/concepts.hpp>
#include <metric/distance.hpp>
#include <metric/engine.hpp>
#include <metric/operators.hpp>
#include <metric/space.hpp>
```

## Finite Metric Space

```cpp
#include <metric/distance.hpp>
#include <metric/space.hpp>

#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> records = {"cat", "cot", "coat", "dog"};
auto space = metric::Space::from_records(records, metric::Edit<std::string>{});

auto nearest = space.neighbors(std::string("cut"), 2);
std::cout << records[nearest[0].first] << ": " << nearest[0].second << "\n";
```

The record type is not required to be a vector. The metric defines the geometry.

## Metric Callables

Any callable accepted by the target operator can be used as a metric. The revived C++ API also exposes a small `metric::Metric<Record, Callable>` wrapper when code wants a named metric object with explicit record and distance types:

```cpp
#include <metric/concepts.hpp>

struct PaddedHamming {
    auto operator()(const std::string &lhs, const std::string &rhs) const -> std::size_t;
};

static_assert(metric::is_metric_callable_v<PaddedHamming, std::string>);

auto distance = metric::make_metric<std::string>(PaddedHamming{});
auto space = metric::Space::from_records(records, distance);
```

`metric::Metric<Record, Callable>` is a typed callable adapter. It does not validate the mathematical axioms at compile time; runtime tests and documented operator assumptions still define whether non-negativity, identity, symmetry, or the triangle inequality are required.

## Operators

Use the free operator helpers when a workflow does not need to keep a `Space` object:

```cpp
#include <metric/distance.hpp>
#include <metric/operators.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {"cat", "cot", "coat", "dog"};

auto distances = metric::operators::pairwise_distance_matrix(records, metric::Edit<std::string>{});
auto nearest = metric::operators::nearest_neighbors(records, metric::Edit<std::string>{}, std::string("cut"), 2);
auto close = metric::operators::range_neighbors(records, metric::Edit<std::string>{}, std::string("cut"), 1);
auto knn_graph = metric::operators::exact_knn_graph(records, metric::Edit<std::string>{}, 1);
auto knn_edges = metric::operators::exact_knn_graph_edges(records, metric::Edit<std::string>{}, 1);
auto radius_graph = metric::operators::exact_radius_graph(records, metric::Edit<std::string>{}, 1);
auto radius_edges = metric::operators::exact_radius_graph_edges(records, metric::Edit<std::string>{}, 1);
auto connectivity_info = metric::operators::graph_connectivity_diagnostics(knn_graph);
auto degree_info = metric::operators::graph_degree_diagnostics(knn_graph);
auto stretch_info = metric::operators::graph_stretch_diagnostics(records, metric::Edit<std::string>{}, radius_graph);
auto undirected = metric::operators::symmetrize_graph(knn_graph, "union", "minimum_distance");
auto pruned = metric::operators::prune_graph_out_degree(
    metric::operators::exact_knn_graph(records, metric::Edit<std::string>{}, 2),
    1);
auto selected_ids = metric::operators::representative_indices(records, metric::Edit<std::string>{}, 2);
auto selected_records = metric::operators::representatives(records, metric::Edit<std::string>{}, 2);
auto center_id = metric::operators::medoid_index(records, metric::Edit<std::string>{});
auto center_record = metric::operators::medoid(records, metric::Edit<std::string>{});
auto separated_ids = metric::operators::separated_representative_indices(records, metric::Edit<std::string>{}, 2);
auto separated_records = metric::operators::separated_representatives(records, metric::Edit<std::string>{}, 2);
auto covered_ids = metric::operators::coverage_representative_indices(records, metric::Edit<std::string>{}, 1);
auto covered_records = metric::operators::coverage_representatives(records, metric::Edit<std::string>{}, 1);
auto dimension = metric::operators::intrinsic_dimension(records, metric::Edit<std::string>{});
```

The promoted C++ operator helpers are `pairwise_distance_matrix`, `nearest_neighbors`, `range_neighbors`, `GraphConnectivityDiagnostics`, `graph_connectivity_diagnostics`, `GraphDegreeDiagnostics`, `graph_degree_diagnostics`, `GraphStretchDiagnostics`, `graph_stretch_diagnostics`, `exact_knn_graph`, `exact_knn_graph_edges`, `exact_radius_graph`, `exact_radius_graph_edges`, `symmetrize_graph`, `prune_graph_out_degree`, `representative_indices`, `representatives`, `medoid_index`, `medoid`, `separated_representative_indices`, `separated_representatives`, `coverage_representative_indices`, `coverage_representatives`, and `intrinsic_dimension`.

The engine operator layer also exposes `metric::operators::knn`, `metric::operators::range`, `metric::operators::kmedoids`, `metric::operators::dbscan`, `metric::operators::entropy`, and `metric::operators::mgc`. The always-on operators are available through `<metric/engine.hpp>` or the specific headers under `<metric/operators/>`; entropy remains in `<metric/operators/entropy.hpp>` because it depends on the LAPACK-backed entropy implementation. These overloads accept `metric::MetricSpace`, engine distance providers such as `MatrixCache`, and neighbor indexes such as `CoverTreeIndex` where appropriate. They return named results such as `metric::NeighborSet<Distance>`, `metric::ClusteringResult<Distance>`, `metric::EntropyResult<double>`, and `metric::CorrelationResult<double>` with stable metadata.

The engine intent layer exposes semantic helpers for the same building blocks:

```cpp
auto neighbors = metric::find_neighbors(space, std::string("cut"), 2);
auto indexed = metric::find_neighbors(space, std::string("cut"), 2, metric::strategies::cover_tree{});
auto groups = metric::find_groups(space, metric::strategies::k_medoids(2));
auto density_groups = metric::find_groups(space, metric::strategies::dbscan(1.0, 2));
```

These helpers keep algorithm names in `metric::strategies` while returning the same engine result objects as the lower-level operator layer.

The first engine mapping adapter lives under `metric::mappings`. `make_clustered_space_mapping(clustering)` follows the `fit(space) -> model` and `model.transform(space) -> MappingResult<DerivedSpace>` convention. The derived clustered space stores one record per non-noise cluster, keeps the source `RecordId` lineage in `source_records`, uses the cluster representative distance as the derived metric, and marks inverse reconstruction as unsupported.

`representative_indices` and `representatives` use deterministic farthest-first traversal over the finite metric space. They select existing records rather than vector centroids, start from `seed_index=0` by default, and resolve equal-distance ties by record order.

`medoid_index` and `medoid` select the existing record with the smallest total distance to all records. Equal total-distance ties are resolved by record order.

`separated_representative_indices` and `separated_representatives` scan records in order and keep a candidate when it is at least `minimum_distance` from every selected representative. This is deterministic redundancy-threshold reduction, not an optimal packing proof.

`exact_knn_graph` and `exact_radius_graph` return `GraphConstructionResult` values with `.edges` and `.metadata`. The metadata records the construction strategy, record count, edge count, directed/self-loop/exact policy, the active `k` or `radius` parameter, edge payload meaning, sparsification policy, symmetrization policy, normalization policy, and the tie-breaking rule. `exact_knn_graph_edges` and `exact_radius_graph_edges` remain convenience helpers that return only directed edge tuples shaped as `(source_index, target_index, distance)`. Exact graph helpers exclude self-loops, preserve source-record order, and resolve equal kNN distances by target record order.

`graph_connectivity_diagnostics` returns `GraphConnectivityDiagnostics` for a graph construction result. It reports deterministic component labels, component count, isolated-record count, largest component size, connected status, and connectivity policy. Directed graph results use weak undirected reachability over stored edges; undirected graph results use endpoint reachability.

`graph_degree_diagnostics` returns `GraphDegreeDiagnostics` for a graph construction result. Directed graphs report `out_degrees`, `in_degrees`, combined endpoint `degrees`, isolated count, max degree, average degree, and degree policy `directed_in_out`. Undirected graph results report endpoint `degrees` with zero-filled in/out vectors and degree policy `undirected_endpoint`.

`graph_stretch_diagnostics` returns `GraphStretchDiagnostics` for a graph construction result plus the source records and metric. It computes shortest-path distances over the stored graph, compares them to the metric distance, and reports evaluated pairs, reachable pairs, unreachable pairs, zero-metric pairs, max stretch, average stretch over reachable pairs, and stretch policy. Directed graph results use directed shortest paths; undirected graph results use bidirectional endpoint paths.

`symmetrize_graph` converts a graph construction result to undirected `source_index < target_index` edges. The supported symmetrization policies are `union` and `mutual`; the supported reciprocal weighting policies are `minimum_distance` and `maximum_distance`. The returned metadata records the selected symmetrization and weighting policies.

`prune_graph_out_degree` keeps at most `max_out_degree` existing directed edges per source record. It sorts each source's candidate edges by `(distance, target_index)`, keeps the first entries, and records `sparsification="out_degree"` plus `max_out_degree` in the returned metadata. It does not compute new distances and rejects already-undirected graph results because their stored source index is not an out-degree contract.

`coverage_representative_indices` and `coverage_representatives` use deterministic greedy radius coverage. They scan records in order, choose the first uncovered record as a representative, and mark every record within `radius` as covered.

`intrinsic_dimension` returns an expansion-dimension estimate based on neighborhood growth. It is a finite-space diagnostic, not an exact manifold dimension.

## Custom Metrics

A custom metric is any callable object accepted by the target operator:

```cpp
struct PaddedHamming {
    auto operator()(const std::string &lhs, const std::string &rhs) const -> double;
};

auto space = metric::Space::from_records(records, PaddedHamming{});
```

Algorithms that require metric-space guarantees assume non-negativity, identity, symmetry, and the triangle inequality. Operators that can work with weaker distances document those assumptions.

## Representations

The engine exposes representation adapters over one `metric::MetricSpace`:

- `metric::representations::ImplicitDistanceProvider<Space>`
- `metric::representations::MatrixCache<Space>`
- `metric::representations::CoverTreeIndex<Space>`
- `metric::representations::KnnGraphIndex<Space>`
- `metric::representations::GraphTopology<Space>`

These adapters preserve stable `RecordId` values. `MatrixCache` returns cached pairwise distances, neighbor indexes return `metric::Neighbor<Distance>` records, and every adapter reports stale state when the source space version changes.

The compatibility core can also materialize explicit representations when a workflow needs control over memory, speed, approximation, or reproducibility.

- `metric::MatrixSpace<Record, Metric>`
- `metric::GraphSpace<Record, Metric>`
- `metric::TreeSpace<Record, Metric>`

`metric::MatrixSpace` stores the full pairwise matrix. `metric::GraphSpace` stores a sparse nearest-neighbor graph. `metric::TreeSpace` provides tree-based neighbor access.

Graph construction terminology for exact, approximate, directed, symmetrized, weighted, and normalized graphs is documented in [Graph Representation Terminology](../concepts/graph-representations.md).

## Engine Roadmap

The implemented C++ facade currently covers finite-space construction, neighbor access through `metric::Space::from_records` and `neighbors`, and free operator helpers for pairwise distances and neighbor queries. Additional intent names such as `groups`, `embed`, `map`, `reduce`, `denoise`, `outliers`, and `compare` describe the public direction and should be promoted only when they are backed by stable strategies, result objects, examples, and CI.

## Compatibility Names

The following names remain available for existing users:

- `metric::Matrix<Record, Metric>`
- `metric::Tree<Record, Metric>`
- `metric::KNNGraph<Record, Metric>`
- `metric::Manhatten<T>`

New code uses the engine vocabulary and `*Space` names unless it intentionally targets the lower-level compatibility surface.
