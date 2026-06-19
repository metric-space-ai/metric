# Graph Representation Terminology

Graph representations are derived structures over a finite metric space. They keep local metric-space relationships sparse, but they do not replace the metric that defined the space.

This page defines the terms METRIC docs use before graph construction becomes a first-page operator API.

The promoted core result helpers are `exact_knn_graph` and `exact_radius_graph`. They return `GraphConstructionResult` values with directed `(source_index, target_index, distance)` edges plus construction metadata. The edge-list helpers `exact_knn_graph_edges` and `exact_radius_graph_edges` remain available when callers only need the directed edge tuples. All four helpers exclude self-loops and are tested against deterministic fixtures. See [Exact Graph Edge Fixtures](../examples/graph-construction.md) for the fixture shape. Symmetrization policies and normalized weights remain roadmap items.

## Required Metadata

Every promoted graph-construction result should state:

- source record IDs
- metric used to compute edge evidence
- construction rule, such as k-nearest neighbors or radius threshold
- whether the edge set is exact or approximate
- whether edges are directed, undirected, or symmetrized
- whether edge payloads are metric distances, affinities, booleans, or normalized weights
- tie-breaking rule for equal distances
- deterministic seed or stochastic build policy when approximation is used

Without this metadata, a graph is an expert representation, not a promoted result contract.

## Exact kNN Graph

An exact k-nearest-neighbor graph has one outgoing neighbor set per source record. For each record, the selected neighbors are the `k` nearest other records under the metric, after applying a documented tie-breaking rule.

`exact_knn_graph` and `exact_knn_graph_edges` use exhaustive pairwise distances, exclude self-loops, preserve source-record order, and resolve equal distances by target record order. The result metadata records strategy `exact_knn`, the record count, edge count, exact/directed policy, `k`, metric-distance payloads, no symmetrization, no normalization, and the tie-breaking rule.

Exactness must be supported by exhaustive pairwise distances, a proved exact algorithm, or deterministic tests that compare against dense pairwise distances on small fixtures.

kNN graphs are naturally directed. If record `a` includes record `b` among its nearest neighbors, `b` does not necessarily include `a`.

## Approximate kNN Graph

An approximate k-nearest-neighbor graph is built from a heuristic, randomized search, pruned candidate set, external ANN backend, or iterative refinement process.

Approximate graphs must not be documented as exact unless their edge sets are verified against the dense pairwise result for the relevant fixture. They should report construction parameters, deterministic seeds when present, and recall or consistency diagnostics when those diagnostics are available.

`metric::KNNGraph` is a compatibility and expert API for approximate kNN graph construction. New promoted examples should state when they are using it as an approximate graph representation.

## Radius Graph

A radius graph connects records whose metric distance is within a threshold.

`exact_radius_graph` and `exact_radius_graph_edges` use exhaustive pairwise distances, exclude self-loops, preserve source-record order, and emit every directed edge whose distance is within `radius`. The result metadata records strategy `exact_radius`, the record count, edge count, exact/directed policy, `radius`, metric-distance payloads, no symmetrization, no normalization, and the source/target scan rule.

An exact directed radius graph has edge `i -> j` when `distance(i, j) <= radius`, subject to a documented self-loop policy. An exact undirected radius graph uses the same threshold but stores one undirected relationship for each qualifying pair.

A radius graph is approximate when it is built from candidate pruning, approximate neighbors, or any method that may miss qualifying pairs.

## Direction And Symmetrization

Directed graphs preserve the construction rule exactly as emitted.

Undirected or symmetrized graphs must document their policy:

- `union`: keep an undirected edge when either directed edge exists
- `mutual`: keep an undirected edge only when both directed edges exist
- `weighted merge`: combine both directed weights with a named reducer such as minimum, maximum, or average

The policy changes graph connectivity and degree distributions, so it is part of the result contract.

## Weights And Normalization

Graph edge payloads must state what they mean:

- metric distance: raw distance from the source metric
- affinity: a transformed similarity, often from a kernel
- boolean adjacency: edge existence only
- normalized weight: row-normalized, degree-normalized, Laplacian-normalized, or another named normalization

Normalized weights are not raw metric distances. They should carry the normalization policy and any scale parameters.

## Promotion Requirements

Graph construction moves from expert representation to promoted operator only when it has:

- deterministic fixtures with expected edge sets
- comparison against dense pairwise distances for exact modes
- documented approximate behavior and diagnostics for approximate modes
- examples that name direction, weighting, symmetrization, and normalization policies
- release-gate tests in the C++ or Python core path

Until those requirements are met, graph APIs remain available as explicit representations and compatibility surfaces rather than first-page graph-construction promises.
