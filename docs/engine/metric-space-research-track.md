# Metric-Space Research Track

This page is the experimental companion to the
[Metric-Space Modification Roadmap](metric-space-modification-plan.md). It
collects research candidates for modifying, sketching, comparing, or
summarizing finite metric spaces without promoting them as stable API.

The scope is strict:

- Input is a finite metric or metric-measure space `(X, d, mu)`.
- Implementations must work from metric calls, pairwise distances, or a distance
  matrix.
- Coordinates, vector spaces, positive-semidefinite kernels, and record
  generators are adapter layers only. They cannot be hidden assumptions for a
  promoted finite-metric-space operator.
- Experimental outputs must carry lineage, metric status, validity text, and
  diagnostics before they can appear in public examples.

## Research Candidate Map

| Candidate | Intent | Finite-space object | First experiment | Promotion gate |
| --- | --- | --- | --- | --- |
| Hierarchical metric nets | `represent`, `compress`, `equalize` | nested radius-separated covers over source records | deterministic cover tree of representative IDs with parent lineage and coverage radii | exact fixtures for line, edit-distance, and histogram metrics; stable tie-breaking; clear complexity bounds |
| Adaptive density equalization | `equalize` | radius or quota varies by local volume / kNN radius | deterministic per-record priority over local-volume diagnostics, then maximal separated subset | proves density drift improvement without losing coverage bounds; reports target density model |
| Distance-distribution sketch | `sketch` | sampled or exact multiset of pairwise distances | quantile/histogram sketch with source-space provenance | bounded drift against exact distribution on deterministic fixtures |
| kNN-recall sketch | `sketch` | subset or graph intended to preserve neighbor behavior | source-to-sketch kNN recall diagnostics for selected queries | reports recall, query sample, seed, and confidence/error bound where sampled |
| Metric graph spanner | `sketch` | sparse graph on source records with edge weights from `d` | deterministic greedy or online spanner over pairwise metric distances | graph stretch fixtures and disconnected/failure behavior documented |
| Spectral graph sparsifier | `sketch` | sparsified weighted graph derived from a metric graph | research-only graph diagnostic after a graph construction step | cannot be presented as a metric-space operator until the graph construction and spectral objective are explicit |
| Metric-measure drift / GW-style diagnostic | compare reductions | two finite metric-measure spaces | diagnostic comparing source and reduced metric-measure summaries | deterministic small fixtures, explicit measure convention, and clear cost limits |
| Density hierarchy quotient | `quotient` | hierarchy of source groups collapsed to representative records | DBSCAN/HDBSCAN-like hierarchy using medoids as quotient records | all quotient records have source lineage and multiplicity; metric validity of quotient is explicit |

## Experimental Result Contract

Every experimental result must say which finite-space observable it tries to
preserve. A result is incomplete if it only names an algorithm.

Required fields:

- `operator_name`: the public intent being explored, such as `sketch`,
  `equalize`, `quotient`, or `compare`.
- `strategy`: concrete experimental strategy name.
- `source_record_count` and `target_record_count`.
- source `RecordId` lineage for every retained or derived record.
- assignment map when any source record is summarized by another record.
- multiplicity or normalized mass when a result is a metric-measure summary.
- `metric_status`: inherited metric subset, derived graph metric, pseudometric,
  unknown, or not-a-metric.
- `validity`: human-readable statement of the assumptions and what is preserved.
- diagnostics for the stated observable: coverage radius, average assignment
  distance, local-density drift, distance-distribution drift, kNN recall, graph
  stretch, or metric-measure drift.

Optional but expected before promotion:

- deterministic seed and sampling policy
- cost model and dense-matrix preflight rule
- approximation quality or confidence fields when diagnostics are sampled
- failure-mode fixtures for empty, singleton, duplicated, and disconnected cases

Current Python prototypes under `metric.experimental` expose this contract
through `to_dict()` even when their native object names differ during migration:
distance sketches keep `record_count` but also report `source_record_count`,
target-free sketches report `target_record_count == 0`, subset/sketch results
report source and representative indices, graph spanners report edge endpoint
lineage, and metric-measure diagnostics report normalized source/target weights.
They also include a `cost_model` dictionary with the research policy name,
exactness, pair count, distance-evaluation count or estimate, a
`dense_matrix_materialized` flag, and the preflight rule required before
promotion. This makes expensive exact prototypes visible instead of hiding dense
or all-pairs behavior behind an algorithm name.

## Candidate Details

### Hierarchical Metric Nets

Metric nets are the natural finite-space generalization of multi-scale covers.
They should select source records only and report, for every level, the radius,
representative IDs, parent representative IDs, multiplicities, coverage radius,
and average assignment distance.

They belong under `represent` or `compress`; if a level policy intentionally
changes density, it belongs under `equalize`. A stable version must make the
scale convention explicit, for example whether radii are user-supplied,
geometrically decayed, or inferred from the finite distance distribution.

Current prototype:

- `metric.experimental.hierarchical_metric_net(...)` builds a deterministic
  nested radius net over source records only.
- Radii are normalized from coarse to fine, and every finer level starts from
  the previous level's representatives before adding uncovered source records.
- Each level reports source representative indices, nearest coarser parent
  indices, per-source assignments, multiplicities, coverage radius, and average
  assignment distance.
- The prototype is Python-only and intentionally not re-exported from
  `metric.__all__`; it exists to prove the result contract on line and
  edit-distance fixtures before any promoted API is considered.

### Adaptive Density Equalization

The promoted `equalize` route currently uses one global radius. Adaptive
equalization should explore local targets computed from finite-space diagnostics:
kNN radius, local volume, or group multiplicity. The key requirement is that the
target density model is explicit. A user must be able to distinguish:

- flatten density toward uniform metric coverage
- preserve empirical measure with weights
- cap duplicate/local oversampling while preserving sparse tails

An adaptive result should keep source records, report source and target local
volume at the chosen scale, and state whether sparse regions were protected by a
minimum coverage rule.

Current prototype:

- `metric.experimental.adaptive_density_equalization(...)` computes local
  volume counts from metric balls at the requested radius.
- It selects lower-volume source records first, then builds a maximal
  radius-separated subset over metric distances.
- It reports source representative indices, per-source assignments,
  multiplicities, selection priority, coverage radius, average assignment
  distance, and before/after average local-volume count.
- The prototype is Python-only and intentionally not re-exported from
  `metric.__all__`; it tests sparse-tail protection on line and edit-distance
  fixtures before promotion.

### Distance-Distribution Sketch

A distance-distribution sketch preserves the finite multiset
`{d(x_i, x_j): i < j}` rather than individual records. It is useful for comparing
subspaces, detecting drift after reduction, or validating that a sketch did not
destroy the global scale of the space.

The experimental result should report exact or sampled quantiles, histogram
bins, pair count, sample count, sample universe, and a drift value against the
source distribution when a source is available.

Current prototype:

- `metric.experimental.distance_distribution_sketch(...)` computes a research-only
  sketch over unordered pairwise finite metric distances.
- `metric.experimental.distance_distribution_drift(...)` compares two sketches by
  mean drift, median drift, maximum quantile drift, and histogram L1 drift.
- The prototype is Python-only and intentionally not re-exported from
  `metric.__all__`; it is evidence for the research contract, not a promoted
  operator.

### kNN-Recall Sketch

A kNN-recall sketch asks whether nearest-neighbor behavior is preserved after a
subset, graph, or summary is built. It must define the query set, neighbor count,
source reference behavior, and recall calculation. If the query set is sampled,
the sample policy and seed are part of the result contract.

This is a diagnostic first, not a reduction algorithm. Promotion should wait
until there are fixtures where coverage radius and kNN recall disagree, so the
diagnostic catches behavior that simple coverage metrics miss.

Current prototype:

- `metric.experimental.knn_recall_sketch(...)` compares source-space kNN sets
  with kNN sets restricted to a source-record subset.
- The prototype requires explicit `sketch_indices` so lineage is unambiguous.
- It reports per-query source neighbors, sketch neighbors, recall, average
  recall, and minimum recall.
- It is Python-only and intentionally not re-exported from `metric.__all__`.

### Graph Spanners And Spectral Sparsifiers

Graph work is allowed only after the graph object is explicit. A graph spanner
uses metric distances as edge weights and reports stretch. A spectral sparsifier
preserves graph Laplacian behavior and therefore depends on a graph construction
choice; it is not directly a finite metric-space reduction.

Experimental graph results should report edge lineage, edge count, graph
connectivity, stretch or spectral diagnostic, and whether shortest-path
distances are being compared with direct metric distances.

Current prototype:

- `metric.experimental.metric_graph_spanner(...)` builds a deterministic greedy
  spanner over all finite source-record pairs.
- Edge weights are direct source metric distances, and edge endpoints are source
  indices.
- It reports edge count, requested stretch bound, maximum observed stretch,
  average observed stretch, and disconnected pair count.
- The prototype is Python-only and intentionally not re-exported from
  `metric.__all__`. Spectral sparsification remains documentation-only.

### Metric-Measure Drift

Compression and thinning produce a new metric-measure object when they report
multiplicity or weights. A metric-measure drift diagnostic should compare the
source `(X, d, mu)` with the reduced `(Y, d_Y, nu)` under an explicit measure
convention.

Gromov-Wasserstein-style diagnostics belong here as research-only comparison
tools until the exact objective, cost, approximation, and measure normalization
are documented with deterministic fixtures.

Current prototype:

- `metric.experimental.metric_measure_drift(...)` compares normalized weights
  and weighted pairwise distance observables between two finite metric-measure
  spaces.
- It reports normalized source/target weights, weighted average pair distance,
  diameter, weight entropy, and the corresponding target-minus-source drift
  values.
- The prototype accepts explicit target weights, so compression multiplicities
  can be tested as the reduced measure `nu`.
- It is not a full Gromov-Wasserstein solver and is intentionally not
  re-exported from `metric.__all__`.

### Density Hierarchy Quotient

A quotient collapses groups into derived records. In the stable core, quotient
records should normally be source medoids so the result remains meaningful for
arbitrary record types. Synthetic centroids or interpolated records require a
domain constructor and belong under `expand` or an adapter layer.

The result must report group lineage, medoid or representative ID, multiplicity,
within-group radius, average within-group distance, and whether distances between
quotient records are inherited source distances or derived aggregate distances.

Current prototype:

- `metric.experimental.density_hierarchy_quotient(...)` builds radius-graph
  components for each requested scale and uses source medoids as quotient
  records.
- Radii are normalized from coarse to fine, so smaller-radius components report
  their parent component position at the previous level when the hierarchy
  refines.
- Each level reports groups, source medoid indices, per-source assignments,
  multiplicities, parent positions, within-group radii, and average
  within-group distances.
- Distances between quotient representatives inherit the source metric; no
  synthetic centroid or vector-space aggregate is constructed.
- The prototype is Python-only and intentionally not re-exported from
  `metric.__all__`.

## Promotion Checklist

Before any research candidate becomes stable:

- It must work on at least one non-vector metric fixture, such as edit distance
  or histogram transport.
- It must pass line-metric fixtures with exact expected representatives,
  assignments, or diagnostics.
- It must define tie-breaking for equal distances.
- It must reject invalid metric status or document pseudo-metric behavior.
- It must provide source lineage and validity text in the result object.
- It must document when the method should not be used.
- It must appear in examples by intent, not by paper name.

Until these checks exist, the candidate stays research-only.

## Initial Promotion Candidates

The first candidates worth hardening are:

1. Hierarchical metric nets.
2. Metric-measure drift diagnostics.

They are first because they are genuinely finite-metric-native: both can be
specified with source record IDs, metric calls, assignments, weights, and
distance-matrix cost models. They do not require coordinates, kernels,
centroids, or a record generator.

Promotion work should proceed in this order:

- Add C++ smoke fixtures for line, edit-distance, histogram transport, and one
  mixed-record composed metric.
- Fix deterministic tie-breaking and exact expected outputs.
- Add preflight gates for O(n^2) distance scans and dense-matrix materialization.
- Promote only the result contract first; optimized implementations can follow.

Everything else stays behind `metric.experimental` until it can pass the same
finite-space evidence standard. In particular, adaptive equalization needs a
clear target-density model, graph spanners need graph-contract fixtures, and
spectral sparsification remains documentation-only until graph construction is
part of the operator contract.
