# Intent API

Intent APIs are named by what users want to do, not by the algorithm used to do it.

| User intent | C++ surface | Python surface | Current status |
|---|---|---|---|
| Find similar records | `find_neighbors` | `Space.neighbors` | Core Revival |
| Find groups | `find_groups` | `Space.groups` | Core Revival |
| Find representatives | `find_representatives` | `Space.representatives` | Core Revival |
| Compare spaces | `compare`, `correlate` | `Space.compare`, `Space.correlate` | Core Revival |
| Describe a space | `describe_structure` | `Space.describe` | Core Revival |
| Find outliers | `find_outliers` | `Space.outliers` | C++ and Python DBSCAN-noise path |
| Reduce complexity | `reduce` | `Space.reduce` | C++ PCFA-backed path; Python representative/medoid reduction path |
| Map to another space | `map`, `metric::mappings::*` | `Space.map` | C++ and Python deterministic transform paths; C++ mapping conventions |
| Denoise a space | `denoise` | `Space.denoise` | C++ and Python DBSCAN-noise filter paths |

The operator layer may still use algorithm names because it is the implementation layer. The intent layer is the recommended user-facing vocabulary.

## C++

```cpp
auto neighbors = metric::find_neighbors(space, query, 2);
auto groups = metric::find_groups(space, metric::strategies::k_medoids(2));
auto representatives = metric::find_representatives(space, 2);
auto outliers = metric::find_outliers(space, metric::strategies::dbscan(2.0, 2));
auto dependency = metric::compare(space_a, space_b, metric::strategies::mgc{});
auto mapped = metric::map(space, transform, target_metric);
auto denoised = metric::denoise(space, metric::strategies::dbscan(2.0, 2));
auto structure = metric::describe_structure(space);
```

## Python

```python
from metric import DistanceProfileCorrelation, Space
from metric.strategies import DBSCAN, KMedoids

space = Space(records, metric)
groups = space.groups(KMedoids(groups=2))
outliers = space.outliers(DBSCAN(radius=2.0, min_points=2))
denoised = space.denoise(DBSCAN(radius=2.0, min_points=2))
dependency = space.compare(other_space, DistanceProfileCorrelation())
reduction = space.reduce(count=2)
mapped = space.map(transform, metric=target_metric)
structure = space.describe()
```

Intent results are named dataclasses or structs. They carry selected records, source IDs, diagnostics, strategy names, and representation metadata.
