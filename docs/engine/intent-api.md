# Intent API

Intent APIs are named by what users want to do, not by the algorithm used to do it.

| User intent | C++ surface | Python surface | Current status |
|---|---|---|---|
| Find similar records | `find_neighbors` | `Space.neighbors` | Core Revival |
| Find groups | `find_groups` | `Space.groups` | Core Revival |
| Find representatives | `find_representatives` | `Space.representatives` | Core Revival |
| Compare spaces | `compare`, `correlate` | `Space.compare`, `Space.correlate` | Core Revival |
| Describe a space | `describe_structure` | `Space.describe` | Core Revival |
| Show coordinates | `embed` | `Space.embed` | C++ PCFA path; Python ClassicMDS path |
| Find outliers | `find_outliers` | `Space.outliers` | C++ and Python DBSCAN-noise path |
| Reduce complexity | `reduce` | `Space.reduce` | C++ PCFA-backed path; Python representative/medoid reduction path |
| Compress a space | `compress` | `Space.compress` | C++ farthest-first representative path; Python representative/medoid path |
| Map to another space | `map`, `metric::mappings::*` | `Space.map` | C++ deterministic transform and mapping-adapter paths; Python deterministic transform path |
| Denoise a space | `denoise` | `Space.denoise` | C++ and Python DBSCAN-noise filter paths |

The operator layer may still use algorithm names because it is the implementation layer. The intent layer is the recommended user-facing vocabulary.

## C++

```cpp
auto neighbors = metric::find_neighbors(space, query, 2);
auto groups = metric::find_groups(space, metric::strategies::k_medoids(2));
auto message_groups = metric::find_groups(space, metric::strategies::affinity_propagation(0.7));
auto representatives = metric::find_representatives(space, 2);
auto outliers = metric::find_outliers(space, metric::strategies::dbscan(2.0, 2));
auto dependency = metric::compare(space_a, space_b, metric::strategies::mgc{});
auto embedding = metric::embed(space, metric::strategies::pcfa(2));
auto compression = metric::compress(space, 2, metric::strategies::farthest_first{});
auto mapped = metric::map(space, transform, target_metric);
auto learned_map = metric::map(space, metric::mappings::pcfa(2));
auto denoised = metric::denoise(space, metric::strategies::dbscan(2.0, 2));
auto structure = metric::describe_structure(space);
```

## Python

```python
from metric import Space
from metric.strategies import DistanceProfileCorrelation
from metric.strategies import ClassicMDS

space = Space(records, metric)
groups = space.groups(count=2, representation=space.to_matrix())
representatives = space.representatives(count=2, representation=space.to_matrix())
outliers = space.outliers(count=2, representation=space.to_matrix())
denoised = space.denoise(count=2, representation=space.to_matrix())
embedding = space.embed(strategy=ClassicMDS(dimensions=2), representation=space.to_matrix())
dependency = space.compare(
    other_space,
    DistanceProfileCorrelation(),
    align="ids",
    representation=space.to_matrix(),
    other_representation=other_space.to_matrix(),
)
reduction = space.reduce(count=2, representation=space.to_matrix())
compression = space.compress(count=2, representation=space.to_matrix())
mapped = space.map(transform=transform, metric=target_metric, representation=space.to_matrix())
structure = space.describe(representation=space.to_matrix())
```

Intent results are named dataclasses or structs. They carry selected records, source IDs, diagnostics, strategy names, and representation metadata.
