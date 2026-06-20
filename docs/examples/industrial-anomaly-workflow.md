# Industrial Anomaly Workflow

Industrial records are often mixed objects rather than clean vectors. A single observation may contain time-series windows, categorical events, image-derived histograms, maintenance text, and numeric sensor summaries. METRIC models each part with its own metric, then computes directly on the resulting finite metric spaces.

## Pattern

1. Define the observation boundary.
2. Choose a domain metric for each record family.
3. Build one or more finite metric spaces.
4. Use neighbor search, grouping, outlier, and comparison operators to inspect local and cross-modal structure.
5. Use embedding or mapping strategies when a downstream workflow needs coordinates, visualization, or a learned embedding function.
6. Use result diagnostics to inspect metric assumptions, runtime choices, and source-to-target lineage.

Possible metric choices:

- process curves: TWED or another time-series metric
- event streams: edit distance or a custom symbolic metric
- histograms: EMD or another distributional metric
- image patches: SSIM-derived or task-specific structured metric
- numeric summaries: Euclidean, Manhattan, Chebyshev, or a standardized variant

## Workflow Shape

```python
from metric import Space
from metric.strategies import DBSCAN, DistanceProfileCorrelation

curves = Space(curve_windows, metric=curve_distance)
events = Space(event_sequences, metric=event_distance)

curve_outliers = curves.outliers(DBSCAN(radius=2.0, min_points=3))
event_groups = events.groups(DBSCAN(radius=1.0, min_points=2))
cross_modal = curves.compare(events, DistanceProfileCorrelation())

mapper = curves.map(preserve="diffusion_geometry", learn_mapping=True)
latent = mapper.transform(new_curve_windows)
```

The first three calls are current Python core intent methods. The `map` calls show the target mapping shape: mapping and embedding remain optional downstream representations. The primary model is the metric space.

The CI-tested C++ engine fixture [mixed_structured_records.cpp](../../examples/engine/mixed_structured_records.cpp) uses the same workflow shape over mixed industrial-style records. Its metric combines numeric summaries, status categories, maintenance-message edit distance, histogram transport, and aligned curve distance before running `find_neighbors`, `find_groups`, and `find_outliers` with explicit matrix-cache runtime diagnostics.
