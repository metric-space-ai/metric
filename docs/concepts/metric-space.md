# Metric Spaces

A metric space is a set of records plus a rule for measuring distance between records of that set. The distance rule defines the geometry. METRIC uses that geometry directly instead of requiring every record to be converted into a vector first.

For a set `X` and a distance function `d`, `(X, d)` is a metric space when `d` satisfies the assumptions required by the operator using it:

- non-negativity: distances are never negative
- identity: a record has zero distance to itself
- symmetry: swapping the record order does not change the distance
- triangle inequality: direct distance is no larger than going through a third record

Some operators also support pseudo-metrics or domain-specific dissimilarities. Each operator documents its metric assumptions.

## Records First

The record type is not fixed. A record can be a string, time series, image descriptor, histogram, graph, tree, vector, or mixed structured object. The metric tells METRIC what similarity means for that record type.

## Finite Spaces

METRIC operates on finite metric spaces: finite record collections equipped with metrics. See [Finite Metric Spaces](finite-metric-space.md) for the engine model.

## Operators and Mappings

Once the metric space is defined, operators compute on it: nearest neighbors, grouping, embedding, mapping, reduction, denoising, outlier detection, entropy, correlation, sparse graph construction, and diagnostics. Mappings transform records or whole spaces into other metric spaces while keeping metric assumptions visible.
