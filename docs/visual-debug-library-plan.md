# METRIC Visual Debug Library Plan

This plan defines the JavaScript/WebGL visualization track for METRIC. The goal
is not to create C++ rendering code. The goal is an interactive browser library
for debugging, inspecting, explaining, and presenting finite metric-space
algorithms on real datasets.

## Product Goal

METRIC should not only compute finite metric spaces. It should make them
inspectable.

Users need to see:

- records
- metric values
- nearest-neighbor structure
- query winners and failures
- representatives
- clusters and outliers
- entropy and density signals
- graph representations
- cross-space dependence
- derived coordinate spaces such as PHATE/AE outputs
- solver or mapping loss traces

SVG is not enough for this. SVG is useful for README and static documentation,
but it does not scale to large point clouds, heatmaps, interactive linked views,
or algorithm debugging.

## Scope

Build a JavaScript/TypeScript visualization package for METRIC evidence.

Working package name:

```text
@metric-space/visual-debug
```

This package is a browser-side visualization and debugging layer. It is not part
of the C++ computational core.

## Candidate Base: Babyplots

Babyplots is a serious candidate because it already provides browser-native
interactive 3D plotting on a canvas/WebGL stack, with TypeScript sources and
support for point clouds, heat maps, surfaces, shape clouds, and image stacks.

The evaluation should determine whether to:

1. use Babyplots as an optional dependency,
2. fork Babyplots and build METRIC-specific linked views on top,
3. upstream useful generic improvements to Babyplots while keeping METRIC views
   in a separate package,
4. or build a smaller METRIC-specific WebGL layer if Babyplots is not flexible
   enough.

A fork is acceptable if it gives METRIC a coherent visualization language and
debugging experience. It must remain a JavaScript/browser package, not a C++
dependency.

## Non-Negotiables

- Real algorithms and metric math stay in C++.
- The visual library renders data produced by METRIC. It must not secretly
  recompute metric results in JavaScript.
- JavaScript may compute view transforms, camera state, filtering, brushing,
  aggregation for rendering, and visual layout.
- JavaScript must not become the source of truth for metric values, MGC,
  entropy, PHATE/AE targets, representatives, or graph construction.
- The visual layer must support large data via Canvas/WebGL, not SVG-only
  rendering.
- Static SVG fallback remains required for GitHub README and non-interactive
  docs.
- No prompt, agent, or planning leaks in public visualizations.

## Evidence Input Contract

All visualizations consume exported METRIC evidence.

Initial format:

- JSON for interactive views
- CSV for simple archival tables
- optional binary columnar format later if JSON becomes too large

Required top-level JSON shape:

```json
{
  "schema": "metric.evidence.v1",
  "provenance": {},
  "records": [],
  "spaces": [],
  "pair_values": [],
  "queries": [],
  "winners": [],
  "representatives": [],
  "graphs": [],
  "coordinates": [],
  "diagnostics": []
}
```

The first implementation can be narrower, but the schema must leave room for
the full METRIC workflow.

## First Views

### 1. Record Gallery

Purpose:

- inspect original records before any derived representation
- show time series, strings, histograms, categories, and mixed records

Needed for:

- process curves
- mixed industrial records
- histogram/image recoding

### 2. Query Debugger

Purpose:

- compare query record, metric-space winner, and vector-baseline winner
- show metric value, baseline value, expected role, and margin
- make failure cases inspectable

Needed for:

- condition monitoring hero
- mixed-record hero
- PHATE/AE out-of-sample debugging

### 3. Metric Heatmap

Purpose:

- render pair values as a large WebGL heatmap
- reorder by label, representative, cluster, or selected path
- compare source metric-space pair values with baseline pair values

Needed for:

- process curves
- strings/edit
- histograms/transport
- cross-space diagnostics

### 4. Space Graph

Purpose:

- inspect kNN/radius graph edges
- brush nodes and inspect source records
- show representatives, outliers, and clusters

Needed for:

- graph construction
- representative selection
- anomaly detection
- diffusion/dynamics

### 5. Derived Coordinate View

Purpose:

- render PHATE/AE/MDS/PCFA/SOM coordinates as 2D or 3D point clouds
- link a point back to its source record and nearest metric-space neighbors
- show geometry-preservation diagnostics

Needed for:

- PHATE/AE derived spaces
- dimensionality reduction debugging
- presentation of finite metric spaces when coordinates are useful

### 6. Cross-Space Dependency View

Purpose:

- inspect paired records across two spaces
- show both source-space structures
- show MGC statistic and permutation/null evidence
- show shuffled or vector baseline failure

Needed for:

- cross-space dependency hero
- multimodal industrial workflows

### 7. Entropy And Local Structure View

Purpose:

- inspect local density, entropy, intrinsic dimension, and outlier scores
- link local statistics to concrete records

Needed for:

- condition monitoring
- anomaly detection
- record-set cleanup

## Babyplots Evaluation Spike

Create a small separate spike, not a permanent dependency yet.

Input:

- exported process-curve external evidence from
  `examples/engine/process_curve_external_gallery.cpp`
- derived PHATE/AE coordinates when available
- synthetic large heatmap fixture if the UCR slice is too small

Questions:

- Can Babyplots render at least 100k points smoothly in the target browsers?
- Can it render large heatmaps without SVG bottlenecks?
- Can we attach custom metadata to points and cells?
- Can selections in one view update another view?
- Can it support linked views, not just standalone plots?
- Can it load METRIC evidence JSON without rewriting the schema around
  Babyplots internals?
- Can it render static fallback images for docs?
- Is the Apache 2.0 license compatible with the intended METRIC packaging?

Deliverable:

```text
docs/visual-debug/babyplots-evaluation.md
```

This document must recommend one of:

- use Babyplots directly
- fork Babyplots
- build a METRIC-specific renderer
- combine Babyplots for 3D views with a custom WebGL heatmap/graph renderer

## First Implementation Package

If the spike is positive, create:

```text
visual-debug/
  package.json
  src/
    index.ts
    evidence/
      schema.ts
      loader.ts
    views/
      record-gallery.ts
      query-debugger.ts
      metric-heatmap.ts
      space-graph.ts
      coordinate-view.ts
      cross-space-view.ts
      entropy-view.ts
  examples/
    process-curve-condition-monitoring/
  README.md
```

The first example should load exported UCR process-curve evidence and show:

- curve gallery
- query debugger
- metric heatmap
- baseline heatmap
- linked selection between views

## C++ Export Requirements

C++ examples must be able to export JSON evidence for the visual library.

First target:

```bash
build/core/examples/engine/engine_process_curve_external_gallery \
  --export-json docs/examples/assets/process-curve-external/evidence.json
```

The JSON output must contain all values needed for the first visual-debug
example.

## Integration With Docs/Site

The static site should eventually embed the visual-debug package for interactive
pages, while README and GitHub docs keep static fallbacks.

Target structure:

- README: static teaser image + link
- docs/examples: static SVG fallback + reproducible C++ evidence
- docs/site: interactive visual-debug view
- visual-debug package: reusable library and examples

## Acceptance Criteria For The First Milestone

- A Babyplots evaluation exists under `docs/visual-debug/`.
- One C++ executable exports `metric.evidence.v1` JSON.
- One browser demo loads that JSON and renders at least:
  - record gallery
  - query debugger
  - metric heatmap
  - baseline heatmap
- Selection in the heatmap updates the inspected records.
- Selection in the record gallery highlights corresponding query/winner rows.
- Static SVG fallback remains available.
- No algorithm result is computed in JavaScript.
- No C++ dependency is added.

## Relationship To Hero Applications

Hero applications should use this library for interactive inspection. They
should not hand-build one-off visualizations for each demo.

The first hero application remains the UCR process-curve condition-monitoring
workflow. The visual-debug library makes it inspectable instead of merely
reported.
