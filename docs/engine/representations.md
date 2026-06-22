# Space Storage And Indexes

Storage and index objects live under `mtrc::space::storage`. They make metric
values cheaper to reuse or query, but the source `MetricSpace` remains the
authority.

Current C++ storage/index components:

- `LiveDistances<Space>`: lazy pairwise metric access
- `DistanceTable<Space>`: eager or lazy pairwise metric-value table
- `CoverTreeIndex<Space>`: exact neighbor-query component with cover-tree API
  shape; performance hardening is tracked separately from correctness
- `KnnGraphIndex<Space>`: graph-backed neighbor index
- `GraphTopology<Space>`: explicit graph topology over source records

```cpp
auto live = mtrc::space::storage::implicit(space);
auto table = mtrc::space::storage::DistanceTable<decltype(space)>(space);
auto tree = mtrc::space::storage::CoverTreeIndex<decltype(space)>(space);
```

Use these components when a workflow needs explicit memory/speed tradeoffs,
reproducible metric-value access, or diagnostics for the execution form. A
storage/index component is promoted first by correctness and result contract;
performance claims require separate benchmark evidence.

## Stale Detection And Recovery

A `MetricSpace` carries a version that advances on every record mutation. A
representation is built for one version and becomes stale the moment the space
mutates. Each representation reports this through `is_stale()`. Recovery is never
automatic on a read path, so stale values are never served silently.

`mtrc::space::cache` turns staleness into a workflow:

```cpp
namespace cache = mtrc::space::cache;

mtrc::space::storage::DistanceTable<decltype(space)> table(space);
// ... mutate the space ...
if (cache::is_stale(table)) {
    cache::refresh(table, space);   // rebuild in place to the current version
}
cache::require_fresh(table);        // or throw StaleRepresentationError instead of reading stale data
```

- `cache::is_stale(rep)` / `cache::status(rep)` report staleness.
- `cache::require_fresh(rep)` throws `mtrc::StaleRepresentationError` when stale.
- `cache::rebuild(rep, space)` returns a fresh representation of the same kind,
  reusing the storage component's own constructor and preserving its defining
  parameters (for example a `KnnGraphIndex`'s `k`).
- `cache::refresh(rep, space)` rebuilds in place only when stale and reports
  whether a rebuild happened.

`mtrc::space::distances::require_fresh` / `checked_value` apply the same rule to a
pairwise-value provider.

Lazy `DistanceTable` providers also fail fast on direct `distance(...)` reads
once stale. This is stricter than eager tables: an eager table is a complete
frozen snapshot of one old version, while a stale lazy table could otherwise mix
old cached cells with new live metric evaluations on cache misses.
