# Graph Representation Terminology

Graph representations are derived structures over a finite metric space. They keep local metric-space relationships sparse, but they do not replace the metric that defined the space.

This page defines the terms METRIC docs use before graph construction becomes a first-page operator API.

The promoted core result helpers are `exact_knn_graph` and `exact_radius_graph`. They return `GraphConstructionResult` values with directed `(source_index, target_index, distance)` edges plus construction metadata. The edge-list helpers `exact_knn_graph_edges` and `exact_radius_graph_edges` remain available when callers only need the directed edge tuples. `graph_connectivity_diagnostics`, `graph_degree_diagnostics`, and `graph_stretch_diagnostics` report deterministic diagnostics for graph construction results. `prune_graph_out_degree` applies deterministic out-degree sparsification to directed graph results. `symmetrize_graph` converts a graph construction result into deterministic undirected edges with documented symmetrization and reciprocal weighting policies. See [Exact Graph Edge Fixtures](../examples/graph-construction.md) for the fixture shape. Normalized weights remain a roadmap item.

## Required Metadata

Every promoted graph-construction result should state:

- source record IDs
- metric used to compute edge evidence
- construction rule, such as k-nearest neighbors or radius threshold
- whether the edge set is exact or approximate
- whether edges are directed, undirected, or symmetrized
- whether edge payloads are metric distances, affinities, booleans, or normalized weights
- any sparsification policy and degree cap applied after construction
- tie-breaking rule for equal distances
- deterministic seed or stochastic build policy when approximation is used

Without this metadata, a graph is an expert representation, not a promoted result contract.

## Exact kNN Graph

An exact k-nearest-neighbor graph has one outgoing neighbor set per source record. For each record, the selected neighbors are the `k` nearest other records under the metric, after applying a documented tie-breaking rule.

`exact_knn_graph` and `exact_knn_graph_edges` use exhaustive pairwise distances, exclude self-loops, preserve source-record order, and resolve equal distances by target record order. They preflight the directed `n * (n - 1)` metric work with `exact_graph_options` and refuse over-budget construction before metric calls; pass `exact_graph_options{0}` only when unbounded exact construction is intentional. The result metadata records strategy `exact_knn`, the record count, edge count, exact/directed policy, `k`, metric-distance payloads, no weighting, no sparsification, no symmetrization, no normalization, and the tie-breaking rule.

Exactness must be supported by exhaustive pairwise distances, a proved exact algorithm, or deterministic tests that compare against dense pairwise distances on small fixtures.

kNN graphs are naturally directed. If record `a` includes record `b` among its nearest neighbors, `b` does not necessarily include `a`.

## Approximate kNN Graph

An approximate k-nearest-neighbor graph is built from a heuristic, randomized search, pruned candidate set, external ANN backend, or iterative refinement process.

Approximate graphs must not be documented as exact unless their edge sets are verified against the dense pairwise result for the relevant fixture. They should report construction parameters, deterministic seeds when present, and recall or consistency diagnostics when those diagnostics are available.

`mtrc::KNNGraph` is an expert API for approximate kNN graph construction. New promoted examples should state when they are using it as an approximate graph representation.

## Radius Graph

A radius graph connects records whose metric distance is within a threshold.

`exact_radius_graph` and `exact_radius_graph_edges` use exhaustive pairwise distances, exclude self-loops, preserve source-record order, and emit every directed edge whose distance is within `radius`. They share the same `exact_graph_options` preflight as exact kNN graph construction and scan pairs directly instead of materializing a full pair-index list. The result metadata records strategy `exact_radius`, the record count, edge count, exact/directed policy, `radius`, metric-distance payloads, no weighting, no sparsification, no symmetrization, no normalization, and the source/target scan rule.

An exact directed radius graph has edge `i -> j` when `distance(i, j) <= radius`, subject to a documented self-loop policy. An exact undirected radius graph uses the same threshold but stores one undirected relationship for each qualifying pair.

A radius graph is approximate when it is built from candidate pruning, approximate neighbors, or any method that may miss qualifying pairs.

## Degree Diagnostics

Graph degree diagnostics are descriptive checks over an existing graph result. They do not prove connectivity, quality, recall, or metric preservation by themselves.

`graph_degree_diagnostics` returns record count, edge count, direction policy, per-record endpoint degrees, per-record out-degrees, per-record in-degrees, isolated-record count, max degree, average degree, and a named degree policy.

For directed graph results, out-degree counts stored edges from each source, in-degree counts stored edges into each target, and endpoint degree is `out + in`. The degree policy is `directed_in_out`.

For undirected graph results, each stored edge contributes one endpoint count to each endpoint. The in/out vectors are zero-filled because the stored source and target are orientation conventions, not directed neighbor contracts. The degree policy is `undirected_endpoint`.

The helper validates that every edge endpoint is within `metadata.record_count`. Invalid endpoint IDs are rejected because degree diagnostics must be tied to the source record set.

## Connectivity Diagnostics

Graph connectivity diagnostics describe connected components in an existing graph result. They do not prove neighbor recall, edge-stretch quality, or metric preservation by themselves.

`graph_connectivity_diagnostics` returns record count, edge count, direction policy, deterministic per-record component labels, component count, isolated-record count, largest component size, connected status, and a named connectivity policy.

For directed graph results, connectivity is weak connectivity over the stored edges. Direction is ignored for component membership because exact kNN and radius graph helpers emit directed edges even when the user wants to know whether the representation covers one connected record set. The connectivity policy is `weak_undirected_reachability`.

For undirected graph results, each stored edge contributes one endpoint relationship between its source and target. The connectivity policy is `undirected_reachability`.

Component labels are assigned by scanning source record IDs in order. A record with no incident stored edge is counted as isolated. The helper validates that every edge endpoint is within `metadata.record_count`.

## Stretch Diagnostics

Graph stretch diagnostics compare shortest paths through an existing graph with direct metric distances in the source finite metric space. They help detect whether a sparse graph preserves pairwise distances for the evaluated records, but they do not prove approximate-neighbor recall or quality outside the supplied record set.

`graph_stretch_diagnostics` accepts source records, the metric callable, and a graph construction result. It returns record count, edge count, direction policy, evaluated pair count, reachable pair count, unreachable pair count, zero-metric pair count, max stretch, average stretch over reachable pairs, and a named stretch policy. It preflights direct metric comparisons, dense shortest-path matrix cells, and the all-pairs closure estimate with `graph_stretch_options`; pass `graph_stretch_options{0, 0, 0}` only when unbounded diagnostics are intentional.

For directed graph results, shortest paths follow stored edge direction and the stretch policy is `directed_shortest_path`. For undirected graph results, each stored edge is evaluated as a bidirectional endpoint relationship and the stretch policy is `undirected_shortest_path`.

Pairs whose direct metric distance is zero are not divided into stretch ratios; they are counted as zero-metric pairs. Pairs that have nonzero metric distance but no graph path are counted as unreachable. The helper validates that `metadata.record_count` matches the supplied records and that every edge endpoint is within that count.

## Degree Sparsification

Sparsification removes edges from an existing graph result. It does not make a graph exact by itself and it must report the rule used to remove edges.

`prune_graph_out_degree` is the promoted deterministic sparsification primitive. It accepts a directed `GraphConstructionResult`, groups existing edges by `source_index`, sorts each group by `(distance, target_index)`, and keeps at most `max_out_degree` edges for each source. It preserves the source graph's construction metadata such as `strategy`, `k`, `radius`, edge payload, weighting, and exactness, then records `sparsification="out_degree"`, `max_out_degree`, the new edge count, and tie-break rule `source_index_then_distance_then_target_index`.

Out-degree pruning is only defined for directed graph results. It rejects already-undirected or symmetrized graph results because `source_index` in an undirected stored edge is an orientation convention, not an outgoing-neighbor contract. Symmetrize after pruning when the workflow needs a smaller undirected graph.

## Direction And Symmetrization

Directed graphs preserve the construction rule exactly as emitted.

Undirected or symmetrized graphs must document their policy:

- `union`: keep an undirected edge when either directed edge exists
- `mutual`: keep an undirected edge only when both directed edges exist
- `weighted merge`: combine both directed weights with a named reducer such as minimum, maximum, or average

The policy changes graph connectivity and degree distributions, so it is part of the result contract.

`symmetrize_graph` promotes two deterministic policies:

- `union`: keep an undirected edge when either directed edge exists
- `mutual`: keep an undirected edge only when both directed edges exist

It emits undirected edges as `source_index < target_index`. When both directions provide a distance, the reciprocal weighting policy is either `minimum_distance` or `maximum_distance`. Normalized weights and affinity transforms remain separate roadmap items.

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

Until those requirements are met, graph APIs remain available as explicit representations and expert surfaces rather than first-page graph-construction promises.
