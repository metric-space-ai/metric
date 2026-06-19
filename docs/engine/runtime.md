# Runtime Policies

Runtime policies make execution cost and approximation explicit.

The first C++ runtime policy surface lives under `metric::runtime`:

- `exact`
- `approximate`
- `lazy`
- `materialized`
- `serial`
- `parallel`
- `cache`
- `diagnostics`

`runtime::diagnostics(...)` returns a `RuntimeDiagnostics` value with the normalized policy name, exact/materialized/parallel flags, selected representation, optional intent label, support status, and reason text for unsupported policies.

```cpp
auto policy = metric::runtime::materialized(metric::runtime::exact());
auto info = metric::runtime::diagnostics(policy, {}, "neighbors");
```

Metrics are treated as thread-safe by default. A metric can specialize `metric::metric_traits<Metric>` with `thread_safe = false` to reject `runtime::parallel(...)` execution until it provides a safe implementation. `runtime::diagnostics_for_metric<Metric>(...)` and `runtime::diagnostics_for_space(space, ...)` report the unsupported state with a reason.

## Neighbor Execution

`runtime::exact()` uses the lazy metric-space path for neighbor lookup.

`runtime::materialized(runtime::exact())` uses a `MatrixCache` for `RecordId` neighbor lookup and reports `representation == "matrix_cache"` in the returned `NeighborSet`.

`runtime::approximate()` is reserved. It currently throws for neighbor lookup instead of silently selecting an approximate algorithm.

## Grouping Execution

`find_groups(..., runtime::exact())` uses the lazy metric-space path for promoted grouping strategies.

`find_groups(..., runtime::materialized(runtime::exact()))` uses a `MatrixCache` for promoted k-medoids, DBSCAN, and affinity-propagation grouping strategies and reports `representation == "matrix_cache"` in the returned `ClusteringResult`.

`runtime::approximate()` is reserved for grouping as well. It throws instead of silently selecting an approximate clustering path.

## Outlier And Denoise Execution

`find_outliers(..., runtime::materialized(runtime::exact()))` uses a `MatrixCache` for DBSCAN-noise detection and nearest-reference scoring, then reports `representation == "matrix_cache"` in the returned `OutlierResult`.

`denoise(..., runtime::materialized(runtime::exact()))` uses a `MatrixCache` for the DBSCAN-noise filtering step while deriving the output space from the original records. The returned `MappingResult` reports `representation == "matrix_cache"`.

Approximate runtime policies throw for these promoted paths until backed by explicit approximate implementations.

## Python Runtime Facade

Python exposes `RuntimePolicy`, `CachePolicy`, `RuntimeDiagnostics`, and `runtime_diagnostics(...)` through `metric.runtime` and the top-level `metric` package. Promoted `Space` intent methods accept `runtime=` and currently execute exact deterministic paths only; approximate policies are explicit placeholders. `Space.groups(..., runtime=RuntimePolicy(cache="materialized"))` records `representation == "matrix"` when no explicit representation override is supplied.

```python
from metric import RuntimePolicy

policy = RuntimePolicy(exact=True, parallel=True, cache="materialized")
diagnostics = space.runtime_diagnostics(
    runtime=policy,
    representation=space.to_matrix(),
    intent="neighbors",
)
```

`diagnostics.representation` records the selected execution representation, while `supported` and `reason` explain whether the normalized policy is promoted by the current Python facade.

## Cache Staleness

Representations capture `space.version()` at construction. `runtime::cache(...)` wraps a representation and delegates `is_stale()` to that version check.

```cpp
auto matrix = metric::runtime::cache(metric::representations::MatrixCache<decltype(space)>(space));
auto before = matrix.is_stale();
space.touch();
auto after = matrix.is_stale();
```

## Current Limits

- Approximate policies are explicit placeholders until backed by real approximate execution paths.
- Parallel policy metadata is exposed before broad parallel execution is implemented, and non-thread-safe metric traits reject parallel runtime policies.
- C++ and Python runtime diagnostics report policy and representation metadata; they do not yet switch execution to approximate or broadly parallel implementations.

This conservative behavior is intentional: no hidden all-pairs materialization or approximate search should happen without an explicit policy or documented default.
