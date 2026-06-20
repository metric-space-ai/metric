# Engine Migration

Existing METRIC APIs remain compatibility surfaces. Migration should be gradual and test-backed.

## Recommended New Code

Start new C++ code with:

```cpp
#include <metric/engine.hpp>

auto space = metric::make_space(records, metric);
auto neighbors = metric::find_neighbors(space, query, 2);
```

Start new Python code with:

```python
from metric import Space

space = Space(records, metric)
neighbors = space.neighbors(query, 2)
```

## Compatibility APIs

The following remain valid for existing code:

- `metric::Space::from_records`
- `metric::MatrixSpace`
- `metric::GraphSpace`
- `metric::TreeSpace`
- C++ compatibility aliases under `metric/compat/aliases.hpp`
- C++ legacy-space adapters under `metric/compat/legacy_space_adapters.hpp`
- historical mapping classes
- historical transform classes
- Python compatibility modules under `metric.distance`, `metric.correlation`, `metric.mapping`, `metric.space`, and `metric.transform`

Legacy index-based C++ spaces can be copied into the engine model explicitly when the metric object is available:

```cpp
#include <metric/compat/legacy_space_adapters.hpp>

auto engine_space = metric::compat::to_metric_space(legacy_space, metric);
auto id = metric::compat::record_id_from_legacy_index(0);
```

## Migration Steps

1. Keep the existing metric callable.
2. Wrap the records and metric in `MetricSpace` or `Space`.
3. Replace direct algorithm calls in examples with intent calls where stable equivalents exist.
4. Replace tuple handling with named result objects.
5. Introduce strategies only when the default intent path is not enough.
6. Introduce runtime policies only when representation or execution cost matters.

Do not migrate broad historical examples into the release gate until they have deterministic fixtures, documented assumptions, and CI coverage.
