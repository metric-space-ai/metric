# Python Real-Data Records

Python spaces can start from real record shapes. The metric defines geometry;
METRIC does not require an embedding step before users can ask intent questions.

> **Availability.** The space construction in these examples runs today
> (`Space.from_dataframe`, `Space.vectors`, `space.distance`, `space.pairwise`,
> the `to_matrix`/`to_tree`/`to_graph` views). The analysis calls shown —
> `space.neighbors(...)`, `space.outliers(...)`, `space.embed(...)` — currently
> raise `StrategyUnavailableError` in the default core wheel because their native
> bindings are not promoted yet. For these analyses today, build the space in
> C++ (`<metric/workflow.hpp>`).

## DataFrame Rows

```python
from metric import Space
import pandas as pd

df = pd.DataFrame(
    [
        {"sample_id": "s0", "temperature": 20.0, "pressure": 1.00, "status": "ok"},
        {"sample_id": "s1", "temperature": 21.0, "pressure": 1.03, "status": "ok"},
        {"sample_id": "s2", "temperature": 35.0, "pressure": 1.40, "status": "warn"},
    ]
)


def row_distance(lhs, rhs):
    numeric = abs(lhs["temperature"] - rhs["temperature"]) + 10.0 * abs(
        lhs["pressure"] - rhs["pressure"]
    )
    status = 5.0 if lhs["status"] != rhs["status"] else 0.0
    return numeric + status


space = Space.from_dataframe(df, metric=row_distance, id_column="sample_id")

neighbors = space.neighbors(space.record("s0"), count=2)
outliers = space.outliers(count=1)

print(neighbors.to_dict())
print(outliers.to_dict())
```

`id_column` becomes the stable ID set. The row dictionaries passed to
`row_distance` do not include that ID field.

## NumPy Time-Series Records

```python
from metric import Space
import numpy as np

records = np.array(
    [
        [0.0, 0.1, 0.2, 0.3],
        [0.0, 0.2, 0.4, 0.6],
        [2.0, 2.1, 1.9, 2.2],
    ]
)


def aligned_curve_distance(lhs, rhs):
    return float(np.mean(np.abs(lhs - rhs)))


space = Space(records, metric=aligned_curve_distance)
embedding = space.embed(dimensions=2)

print(embedding.coordinates)
```

NumPy records are preserved as records. The metric callable receives the row
objects and decides how the distance should be computed.
