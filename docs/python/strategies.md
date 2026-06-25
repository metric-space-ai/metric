# Python Strategy Overrides

Normal Python examples should start with records, a metric, and intent methods.
Strategy objects are expert controls under `metric.strategies`; they are not
exported as top-level algorithm names.

## Strategy Vocabulary

| Strategy | Intent Path | Notes |
|---|---|---|
| `MDS` / `ClassicMDS` | `space.embed(...)` | Deterministic classical MDS over the exact finite distance matrix. |
| `KMedoids` | `space.groups(...)`, `space.compress(...)` | Deterministic representative grouping and medoid-based compression. |
| `DBSCAN` | `space.groups(...)`, `space.outliers(...)`, `space.density_filter(...)` | Deterministic density grouping and DBSCAN-unassigned record filtering. |
| `FarthestFirst` | `space.representatives(...)`, `space.reduce(...)`, `space.compress(...)` | Deterministic representative selection. |
| `Coverage` | `space.representatives(...)`, `space.reduce(...)`, `space.compress(...)` | Intent alias for coverage/k-center representative selection by requested count. |
| `KCenter` | `space.representatives(...)`, `space.reduce(...)`, `space.compress(...)` | Explicit k-center alias for coverage-style compression. |
| `RadiusCoverage` | `space.compress(...)`, `compress_space(...)` | Greedy radius cover; the radius determines the representative count. |
| `PreserveDistribution` | `space.thin(...)`, `space.distribution_sample(...)` | Deterministic regular thinning that keeps empirical order/density representative. |
| `UniformDensity` | `space.thin(...)`, `space.uniform_density_sample(...)`, `space.equalize(...)` | Maximal metric radius net that intentionally flattens sampling density. |
| `DistanceProfileCorrelation` | `space.compare(...)` | Pearson correlation of exact pairwise distance profiles. |

```python
from metric import Space
from metric.metrics import Edit
from metric.strategies import FarthestFirst, KMedoids, PreserveDistribution, RadiusCoverage, UniformDensity

space = Space(["cat", "cot", "coat", "dog"], metric=Edit())
representatives = space.representatives(count=2, strategy=FarthestFirst(seed_index=0))
groups = space.groups(strategy=KMedoids(groups=2, max_iterations=50))
compressed = space.compress(strategy=RadiusCoverage(radius=1))
panel = space.thin(2, strategy=PreserveDistribution(offset=0))
net = space.thin(strategy=UniformDensity(radius=1))
equalized = space.equalize(radius=1)
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

## Native Mapping Artifact Vocabulary

`ParametricDiffusionCoordinates` names a native C++ preset workflow for a derived mapping artifact. In
METRIC terms, it combines source distances, diffusion compression, a native
coordinate-map component, and diagnostics. The Python facade must delegate to C++
before this strategy becomes executable.

`Space.map(strategy=ParametricDiffusionCoordinates(...))` delegates to C++ only for
`Space.vectors(...)` values that use the default native-compatible Euclidean
vector metric. Spaces with custom Python metrics raise `StrategyUnavailableError`
instead of silently replacing their metric semantics. Use C++
`metric::mappings::parametric_diffusion_coordinates` or Python
`metric.mappings.derive_parametric_diffusion_coordinates(...)` when you want the
explicit native vector-row path.

Native mapping artifacts can be inspected from Python with
`metric.mappings.load_native_mapping_artifact(...)`. That projection exposes
manifest, mapping, strategy, pipeline, component, source, diagnostic, and
network-byte metadata only. For native parametric diffusion coordinate artifacts, that includes derived
pipeline-plan component choices such as `record_coordinate_codec` and
`distance_table_pairwise_access`. It also exposes native component names such as
`exponential_affinity_kernel` and `lazy_row_normalized_diffusion_operator` when
they are present in the manifest.
`transform(...)` and `inverse_transform(...)` on the
projection raise `StrategyUnavailableError` until the native C++ artifact binding
is promoted.

`metric.mappings.derive_parametric_diffusion_coordinates(...)` delegates artifact
derivation, transform, inverse transform, diffusion geometry, and native map solving
to C++ for native Euclidean vector rows. `ParametricDiffusionCoordinates` and the explicit derive helper
may pass supported native distance-access, affinity-kernel, and
diffusion-operator names through the binding. Python only marshals list or
NumPy row data and exposes the native mapping artifact.

`ParametricDiffusionCoordinates` is not an embedding strategy; `Space.embed(strategy=ParametricDiffusionCoordinates(...))`
raises `StrategyUnavailableError`.

## Roadmap Strategies

Only finite-metric-space roadmap strategies remain importable. They exist to
name a future metric-space operation, not to preserve old algorithm entry
points.

| Strategy | Roadmap Area |
|---|---|
| `DiffusionEmbedding` | Diffusion and diffusion-coordinate embedding. |

Examples must show that roadmap entries are not promoted execution paths yet.
