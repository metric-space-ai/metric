# Python Strategy Overrides

Normal Python examples should start with records, a metric, and intent methods.
Strategy objects are expert controls under `metric.strategies`; they are not
exported as top-level algorithm names.

## Strategy Vocabulary

| Strategy | Intent Path | Notes |
|---|---|---|
| `MDS` / `ClassicMDS` | `space.embed(...)` | Deterministic classical MDS over the exact finite distance matrix. |
| `KMedoids` | `space.groups(...)` | Deterministic representative grouping vocabulary; not promoted in the default wheel yet. |
| `DBSCAN` | `space.groups(...)`, `space.outliers(...)`, `space.denoise(...)` | Deterministic density grouping and noise filtering. |
| `FarthestFirst` | `space.representatives(...)`, `space.reduce(...)`, `space.compress(...)` | Deterministic representative selection. |
| `DistanceProfileCorrelation` | `space.compare(...)` | Pearson correlation of exact pairwise distance profiles. |

```python
from metric import Space
from metric.metrics import Edit
from metric.strategies import FarthestFirst, KMedoids

space = Space(["cat", "cot", "coat", "dog"], metric=Edit())
representatives = space.representatives(count=2, strategy=FarthestFirst(seed_index=0))
# groups = space.groups(strategy=KMedoids(groups=2, max_iterations=50))
# KMedoids grouping is importable vocabulary but not promoted in the default wheel yet.
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

## Native Fitted Map Vocabulary

`PhateAE` names a native C++ preset workflow for a fitted map. In METRIC terms,
it combines source distances, diffusion compression, a native fitted-map
component, and diagnostics. The Python facade must delegate to C++ before this
strategy becomes executable.

`Space.map(strategy=PhateAE(...))` delegates to C++ only for
`Space.vectors(...)` values that use the default native-compatible Euclidean
vector metric. Spaces with custom Python metrics raise `StrategyUnavailableError`
instead of silently replacing their metric semantics. Use C++
`metric::mappings::native_phate_autoencoder` or Python
`metric.mappings.native_phate_autoencoder_fit_vectors(...)` when you want the
explicit native vector-row path.

Native fitted mapping artifacts can be inspected from Python with
`metric.mappings.load_native_mapping_artifact(...)`. That projection exposes
manifest, mapping, strategy, pipeline, component, source, diagnostic, and
network-byte metadata only. For native PHATE-AE artifacts, that includes fitted
pipeline-plan component choices such as `feature_record_codec` and
`distance_table_pairwise_access`. It also exposes native component names such as
`exponential_affinity_kernel` and `lazy_row_normalized_diffusion_operator` when
they are present in the manifest.
`transform(...)` and `inverse_transform(...)` on the
projection raise `StrategyUnavailableError` until the native C++ model binding
is promoted.

`metric.mappings.native_phate_autoencoder_fit_vectors(...)` delegates fitting,
transform, inverse transform, diffusion geometry, and native fitted-map training
to C++ for native Euclidean vector rows. `PhateAE` and the explicit fit helper
may pass supported native distance-access, affinity-kernel, and
diffusion-operator names through the binding. Python only marshals list or
NumPy row data and exposes the native model handle.

`PhateAE` is not an embedding strategy; `Space.embed(strategy=PhateAE(...))`
raises `StrategyUnavailableError`.

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

Roadmap strategy imports are acceptable in expert documentation, but examples
must show that they are not promoted execution paths yet.
