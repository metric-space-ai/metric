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
