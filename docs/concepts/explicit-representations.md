# Explicit Representations

A finite metric space can be represented implicitly or explicitly.

An implicit representation stores records and a metric, then computes distances on demand. This is flexible and often sufficient for exploratory workflows or metrics that are cheap to evaluate.

An explicit representation stores derived structure from the metric space. METRIC exposes these core forms:

- `mtrc::MatrixSpace`: full pairwise distance matrix
- `mtrc::GraphSpace`: sparse nearest-neighbor graph
- `mtrc::TreeSpace`: search/index representation for neighbor access

## Matrix

`mtrc::MatrixSpace` stores all pairwise distances. It gives direct lookup and straightforward nearest-neighbor behavior at the cost of quadratic storage.

Use it when the dataset is small to medium sized, the metric is expensive enough to cache, or exact pairwise inspection matters.

## Graph

`mtrc::GraphSpace` stores local neighbor structure. It is useful when sparse local geometry drives the workflow.

Graph representations document whether edges are exact, approximate, directed, symmetrized, weighted, or normalized. The promoted terminology is defined in [Graph Representation Terminology](graph-representations.md).

## Tree

`mtrc::TreeSpace` stores a search structure. It improves neighbor access when the metric and data distribution are suitable.

Tree performance depends on metric behavior and intrinsic dimension. A tree is an expert representation, not a universal replacement for a matrix.

## Representation Choice

The engine chooses representations automatically for common workflows. Expert users can override the choice through runtime policy or strategy configuration.

Compatibility names remain available:

- `mtrc::Matrix`
- `mtrc::Tree`
- `mtrc::KNNGraph`
