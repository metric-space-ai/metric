# Engine Mental Model

The engine starts with one object:

```text
RecordSet + Metric -> MetricSpace
```

`MetricSpace` owns the finite record set, the metric callable, stable `RecordId` values, and version state. Representations, operators, intents, strategies, mappings, and runtime policies are built around that object instead of replacing it.

## Phase 1 Surface

The first C++ engine skeleton introduces:

- `metric::RecordId`
- `metric::Neighbor<Distance>`
- `metric::metric_traits`
- `metric::MetricCallable`
- `metric::MetricSpaceLike`
- `metric::MetricSpace<Record, Metric>`
- `metric::make_space(records, metric)`

This is intentionally small. It does not migrate legacy algorithms, add Python bindings, or replace existing compatibility classes. It establishes the central object that later representation adapters and intent APIs can use.

## Phase 2 Surface

The first representation adapters live under `metric::representations`:

- `ImplicitDistanceProvider<Space>`
- `MatrixCache<Space>`
- `CoverTreeIndex<Space>`
- `KnnGraphIndex<Space>`
- `GraphTopology<Space>`

They are views or materialized execution structures over the same `MetricSpace`; they do not own or redefine the record set. Each adapter captures the source space version when it is created and exposes `is_stale()` so later mutable space operations can invalidate cached or indexed representations explicitly.

`CoverTreeIndex` and `KnnGraphIndex` currently provide exact deterministic neighbor ordering over the finite space while preserving the intended engine vocabulary. The implementation can be replaced by specialized tree or graph algorithms later without changing the representation role.

## Phase 3 Surface

The first engine operators live under `metric::operators` and return named result objects:

- `metric::NeighborSet<Distance>`
- `metric::ClusteringResult<Distance>`
- `metric::EntropyResult<double>`
- `metric::CorrelationResult<double>`
- `metric::operators::knn(space, query, count)`
- `metric::operators::knn(space, query_id, count)`
- `metric::operators::knn(distance_provider, query_id, count)`
- `metric::operators::knn(neighbor_index, query, count)`
- `metric::operators::range(space, query, radius)`
- `metric::operators::range(distance_provider, query_id, radius)`
- `metric::operators::kmedoids(space, cluster_count)`
- `metric::operators::kmedoids(distance_provider, cluster_count)`
- `metric::operators::dbscan(space, radius, min_points)`
- `metric::operators::dbscan(distance_provider, radius, min_points)`
- `metric::operators::entropy(space)`
- `metric::operators::entropy(records, metric)`
- `metric::operators::mgc(space_a, space_b)`
- `metric::operators::mgc(records_a, metric_a, records_b, metric_b)`

This is still the implementation layer, not the final intent API. It lets the same nearest-neighbor operation run against an implicit `MetricSpace`, a materialized `MatrixCache`, or a neighbor index while returning stable `RecordId` values. It also lets the first grouping operators run against either a space or distance provider while returning stable `RecordId` payloads.
Density clustering uses the same `ClusteringResult` contract and marks noise records with `ClusteringResult<Distance>::noise_label`.

## Phase 3b Surface

The first semantic intent helpers live at the `metric::find_*` layer:

- `metric::find_neighbors(space, query, count)`
- `metric::find_neighbors(space, query_id, count)`
- `metric::find_neighbors(space, query, count, metric::strategies::cover_tree{})`
- `metric::find_neighbors(space, query, count, metric::strategies::knn_graph(graph_neighbors))`
- `metric::find_neighbors(space, query_id, count, metric::strategies::matrix_cache{})`
- `metric::find_groups(space, group_count)`
- `metric::find_groups(space, metric::strategies::k_medoids(group_count))`
- `metric::find_groups(space, metric::strategies::dbscan(radius, min_points))`
- `metric::compare(space_a, space_b)`
- `metric::compare(space_a, space_b, metric::strategies::mgc{})`
- `metric::correlate(space_a, space_b)`
- `metric::reduce(space, metric::strategies::pcfa(components))`

These are the first user-facing names that describe intent before algorithm. Strategy objects select implementation details while preserving the same result types returned by the operator layer.

## Phase 4 Surface

The first mapping convention lives under `metric::mappings`:

- `metric::MappingResult<DerivedSpace>`
- `metric::Mapping<Mapping, Space>`
- `metric::MappingModel<Model, Space>`
- `metric::mappings::fit(mapping, space)`
- `metric::mappings::transform(model, space)`
- `metric::mappings::make_clustered_space_mapping(clustering)`
- `metric::mappings::clustered_space(space, clustering)`
- `metric::mappings::pcfa(components)`
- `metric::mappings::pcfa_space(space, components)`

Clustered-space mapping turns a `ClusteringResult` into a derived `MetricSpace`. Each derived record represents one non-noise cluster, stores the source `RecordId`s that formed it, and uses the source-space distance between cluster representatives as the derived metric. Inverse reconstruction is explicit and currently marked unsupported.

PCFA mapping lives in `<metric/mappings/pcfa.hpp>` because it wraps the LAPACK-backed legacy PCFA implementation. It fits a linear encoder to vector-like records, transforms a source space into an encoded metric space, stores one-to-one source lineage, and exposes explicit inverse reconstruction through the fitted model.

The PCFA-backed reduce intent lives in `<metric/intent/reduce.hpp>` for the same reason. It keeps the user-facing operation semantic while selecting PCFA as an explicit reduction strategy.

## Current Contract

The initial `MetricSpace` owns records by value. `RecordId` is an opaque wrapper over dense internal storage. `version()` exists and starts at `0`; later representation adapters can use it for stale-cache checks when mutating space operations are introduced.

Distance evaluation stays direct:

```cpp
#include <metric/distance.hpp>
#include <metric/engine.hpp>
#include <metric/operators/entropy.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {"metric", "metrics", "matrix", "tree"};
auto space = metric::make_space(records, metric::Edit<char>{});

auto lhs = space.id(0);
auto rhs = space.id(1);
auto distance = space.distance(lhs, rhs);

metric::representations::MatrixCache<decltype(space)> matrix(space);
auto cached_distance = matrix.distance(lhs, rhs);

metric::representations::CoverTreeIndex<decltype(space)> tree(space);
auto neighbors = tree.knn(std::string("metricks"), 2);

auto exact_neighbors = metric::operators::knn(space, std::string("metricks"), 2);
auto indexed_neighbors = metric::operators::knn(tree, std::string("metricks"), 2);

auto groups = metric::operators::kmedoids(matrix, 2);
auto dense_groups = metric::operators::dbscan(matrix, 1, 2);
auto structure_entropy = metric::operators::entropy(space);
auto dependency = metric::operators::mgc(space, space);
auto clustered = metric::mappings::clustered_space(space, groups);

auto user_neighbors = metric::find_neighbors(space, std::string("metricks"), 2);
auto user_groups = metric::find_groups(space, metric::strategies::k_medoids(2));
auto user_dependency = metric::compare(space, space, metric::strategies::mgc{});
```

The important shift is vocabulary: engine code starts from a metric space and stable record IDs. Algorithm names and representation choices come later as strategies and execution structures. A representation reports stale state when the source space version changes:

```cpp
auto before = matrix.is_stale(); // false
space.touch();
auto after = matrix.is_stale(); // true
```
