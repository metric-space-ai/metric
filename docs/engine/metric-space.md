# Metric Space

`mtrc::MetricSpace<Record, Metric>` owns records plus the selected metric. It
assigns stable `mtrc::RecordId` values and answers pairwise metric queries.

```cpp
#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {"cat", "cot", "coat", "dog"};
auto space = mtrc::make_space(records, mtrc::Edit<char>{});

auto lhs = space.id(0);
auto rhs = space.id(1);
auto value = space.distance(lhs, rhs);
```

Records can be inserted, replaced, and erased. These mutations update the space
version so storage and index objects can detect stale state.

The metric remains authoritative. A graph, table, tree, or map is an execution
or derived form over the same source space.
