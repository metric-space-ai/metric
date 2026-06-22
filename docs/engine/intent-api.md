# Public Components

The public C++ surface reads as finite metric-space work, not as a list of
paper names.

```cpp
auto neighbors = mtrc::stats::search::find_neighbors(
    space,
    query,
    mtrc::count{3});

auto indexed = mtrc::stats::search::find_neighbors(
    space,
    query,
    mtrc::count{3},
    mtrc::stats::search::cover_tree{});
```

Higher-level helpers state:

- source finite metric space
- authoritative metric
- owning `mtrc` namespace
- concrete implementation choice
- result object, lineage, diagnostics, and failure modes
