# Python Strategy Overrides

Normal Python examples should start with records, a metric, and intent methods.
Strategy objects are expert controls under `metric.strategies`; they are not
exported as top-level algorithm names.

## Promoted Strategies

| Strategy | Intent Path | Notes |
|---|---|---|
| `MDS` / `ClassicMDS` | `space.embed(...)` | Deterministic classical MDS over the exact finite distance matrix. |
| `KMedoids` | `space.groups(...)`, `space.reduce(...)`, `space.compress(...)` | Deterministic representative grouping and reduction. |
| `DBSCAN` | `space.groups(...)`, `space.outliers(...)`, `space.denoise(...)` | Deterministic density grouping and noise filtering. |
| `FarthestFirst` | `space.representatives(...)`, `space.reduce(...)`, `space.compress(...)` | Deterministic representative selection. |
| `DistanceProfileCorrelation` | `space.compare(...)` | Pearson correlation of exact pairwise distance profiles. |

```python
from metric import Space
from metric.metrics import Edit
from metric.strategies import KMedoids

space = Space(["cat", "cot", "coat", "dog"], metric=Edit())
groups = space.groups(strategy=KMedoids(groups=2, max_iterations=50))
```

Use strategy objects only when the default intent behavior is not explicit
enough for the workflow.

## Representation Overrides

```python
from metric import Space
from metric.metrics import Edit

space = Space(["cat", "cot", "coat", "dog"], metric=Edit())
matrix = space.to_matrix()

groups = space.groups(count=2, representation=matrix)
```

Representations make execution metadata explicit. A stale representation raises
`StaleRepresentationError` and must be rebuilt from the source space.

## Runtime Overrides

```python
from metric import Space
from metric import RuntimePolicy
from metric.metrics import Edit

space = Space(["cat", "cot", "coat", "dog"], metric=Edit())
policy = RuntimePolicy(exact=True, cache="materialized", parallel=True)

neighbors = space.neighbors("cut", count=2, runtime=policy)
```

Runtime policy objects live in `metric.runtime` and are also re-exported from
`metric`. The promoted Python facade is exact-first. Approximate runtime
requests fail explicitly until approximate result contracts are promoted.

## Roadmap Strategies

The following strategy objects are importable from `metric.strategies` so users
can write stable engine vocabulary, but execution raises
`StrategyUnavailableError` until deterministic fixtures, diagnostics, and
release gates are promoted:

| Strategy | Roadmap Area |
|---|---|
| `DiffusionEmbedding` | Diffusion and PHATE-style embedding. |
| `PCFA` | PCFA reduction or mapping. |
| `SOM` | Self-organizing-map reduction or mapping. |
| `KOC` | Kohonen outlier chart reduction or mapping. |
| `DSPCC` | DSPCC reduction or dependency-preserving mapping. |
| `PhateAE` | PHATE autoencoder style learned mapping. |

Roadmap strategy imports are acceptable in expert documentation, but examples
must show that they are not promoted execution paths yet.
