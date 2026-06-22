# Metrics as Recoding Costs

In METRIC, a metric is best understood as the minimum cost of recoding one
record as another record from the same domain. This framing keeps the metric
tied to the data semantics instead of treating it as a generic closed formula.

The metric value can come from a formula or from an algorithm that solves a
well-defined minimum-cost transformation problem.

Examples:

- edit distance: minimum cost of rewriting one string into another
- TWED or ERP: minimum cost of aligning time series with temporal penalties or
  gap costs
- EMD/Wasserstein: minimum cost of moving mass between histograms or
  distributions
- shortest-path distance: minimum cost path through a graph
- tree or graph edit distance: minimum valid edit script under admitted costs
- Euclidean distance: closed-form coordinate displacement

The optimal transformation path does not need to be unique. The distance value
must be unique for the declared domain, parameters, and numeric tolerance. The
same record collection can produce different metric spaces under different
metrics. That is expected. The metric defines which changes matter.

## Metric Requirements

Every operator documents whether it requires a true metric or can accept a weaker distance. The common true-metric requirements are:

- non-negativity
- identity
- symmetry
- triangle inequality

When a built-in distance violates one of these properties, it is not part of the
long-term public METRIC metric library. It must be quarantined and then either
replaced by an admitted true-metric variant or removed from normal discovery,
examples, bindings, and metric-only routing.

## Finite Numeric View

For a finite dataset, the practical metric is the numeric function:

```text
d: X x X -> R
```

where `X` is the finite record set. The function may be evaluated lazily,
cached, materialized as a distance matrix, represented through a graph or tree,
or used to fit a derived mapping. Those are execution choices. They do not
change the source metric.

This is compatible with the usual mathematical definition of a metric space,
which also covers infinite and continuous sets. Continuous settings add topics
such as topology, smoothness, geodesics, and local coordinate charts. METRIC
focuses on the finite numeric object that a C++ program can compute while
preserving the same metric axioms.

## Custom Metrics

Custom callables are an advanced extension point. A metric callable receives two
records and returns a numeric distance:

```cpp
distance = metric(lhs, rhs);
```

The C++ core also provides a typed adapter for custom callables:

```cpp
auto distance = mtrc::make_metric<Record>(callable);
```

The return type must be orderable and numeric enough for the operator using it.
For metric-only algorithms, the callable also needs an admission record and a
truthful `metric_traits` law. See [Custom Metric Example](../examples/custom-metric.md).
