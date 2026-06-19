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

## Neighbor Execution

`runtime::exact()` uses the lazy metric-space path for neighbor lookup.

`runtime::materialized(runtime::exact())` uses a `MatrixCache` for `RecordId` neighbor lookup and reports `representation == "matrix_cache"` in the returned `NeighborSet`.

`runtime::approximate()` is reserved. It currently throws for neighbor lookup instead of silently selecting an approximate algorithm.

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
- Parallel policy metadata is exposed before broad parallel execution is implemented.
- Python does not yet expose runtime policies as first-class objects.

This conservative behavior is intentional: no hidden all-pairs materialization or approximate search should happen without an explicit policy or documented default.
