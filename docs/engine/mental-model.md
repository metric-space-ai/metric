# Finite-Space Mental Model

METRIC starts from records and a metric. The records can be strings, curves,
histograms, graphs, vectors, or structured domain objects. The metric gives one
numeric value for each record pair.

```text
Record set X
Metric d(a, b)
Finite metric space (X, d)
```

The framework then answers questions about `(X, d)`:

- search: which records are near this record?
- properties: what density or dimension does the space exhibit, and what
  entropy does an embedded coordinate representation exhibit?
- correlation: how does this space relate to a paired space?
- sampling: which records or metric-space walks summarize the finite geometry?
- structural analysis: which groups, representatives, or outliers exist?
- transformation: what derived or modified space is constructed?
- mapping: when coordinates are useful, how does a derived coordinate space
  preserve the source metric?

Execution forms such as distance tables, graphs, trees, local coordinate views,
or fitted maps are derived from the source space. They do not replace it. An
algorithm that requires coordinates, such as the current kpN entropy estimator,
belongs after an explicit mapping/embedding step in that pipeline.

Vector spaces are included as the coordinate-record special case.
