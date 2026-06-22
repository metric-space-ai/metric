# Metric Spaces

A metric space is a set of records plus a rule for measuring distance between
records of that set. The distance rule defines the geometry. METRIC uses that
geometry directly instead of requiring every record to be converted into a
vector first.

For a set `X` and a distance function `d`, `(X, d)` is a metric space when `d`
satisfies:

- non-negativity: distances are never negative
- identity: a record has zero distance to itself
- symmetry: swapping the record order does not change the distance
- triangle inequality: direct distance is no larger than going through a third record

The function `d` can be evaluated by a closed formula or by an algorithm that
computes a well-defined minimum transformation cost. Euclidean distance is a
closed-form vector metric; edit distance, shortest-path distance, and
Wasserstein/EMD are examples where the value is usually computed by an
algorithmic optimization procedure.

Built-in pseudo-metrics, divergences, similarities, or unproven costs are not
normal METRIC metrics. They must stay out of metric-only algorithms and move
through quarantine unless a precise true-metric variant is admitted.

## Records First

The record type is not fixed. A record can be a string, time series, image descriptor, histogram, graph, tree, vector, or mixed structured object. The metric tells METRIC what similarity means for that record type.

## Finite Spaces

METRIC operates on finite metric spaces: finite record collections equipped with metrics. See [Finite Metric Spaces](finite-metric-space.md) for the engine model.

The same metric axioms apply to infinite and continuous mathematical metric
spaces. METRIC focuses on the finite numeric object used by a C++ program:
records plus a pairwise distance computation over those records. Continuous
tools such as local Euclidean approximation or embeddings can still be useful,
but they are representations derived from the source metric, not the definition
of the space.

## Operators and Mappings

Once the metric space is defined, operators compute on it: nearest neighbors, grouping, embedding, mapping, reduction, denoising, outlier detection, entropy, correlation, sparse graph construction, and diagnostics. Mappings transform records or whole spaces into other metric spaces while keeping metric assumptions visible.
