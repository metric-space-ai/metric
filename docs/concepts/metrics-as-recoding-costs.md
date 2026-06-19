# Metrics as Recoding Costs

In METRIC, a metric is best understood as the cost of recoding one record as another record from the same domain. This framing keeps the metric tied to the data semantics instead of treating it as a generic formula.

Examples:

- edit distance: cost of rewriting one string into another
- TWED: cost of aligning time series with temporal penalties
- EMD: cost of moving mass between histograms or distributions
- SSIM-derived distance: cost of perceptual image disagreement
- Euclidean distance: cost in a vector coordinate system

The same record collection can produce different metric spaces under different metrics. That is expected. The metric defines which changes matter.

## Metric Requirements

Every operator documents whether it requires a true metric or can accept a weaker distance. The common true-metric requirements are:

- non-negativity
- identity
- symmetry
- triangle inequality

When a distance violates one of these properties, nearest-neighbor behavior, graph sparsification, clustering, and correlation diagnostics may still run, but the interpretation changes.

## Custom Metrics

Custom metrics are first-class. A metric callable receives two records and returns a numeric distance:

```cpp
distance = metric(lhs, rhs);
```

The C++ core also provides a typed adapter for custom callables:

```cpp
auto distance = metric::make_metric<Record>(callable);
```

The return type must be orderable and numeric enough for the operator using it. See [Custom Metric Example](../examples/custom-metric.md).
