# Agent Task: Record Glyph And Property Field Grammar Workstream

## Owner

One worker owns only:

- `visual/src/glyphs/*`
- `visual/src/layers/InstancedGlyphLayer.js`
- `visual/src/layers/HeatFieldLayer.js`
- `visual/src/views/MetricSpaceView.js`
- `visual/src/views/SpacePropertiesView.js`
- glyph/field checks under `visual/tools/check-glyph-*.mjs` and
  `visual/tools/check-field-*.mjs`
- `visual/examples/mixed-record-hero/index.html` and
  `visual/examples/condition-monitoring-hero/index.html` only as thin
  acceptance fixtures

Do not edit command API, relation matrix code, mapping/dynamics code, native
C++ exporters, project page, or GRAE10. Other workers may edit other visual
subsystems in parallel; do not revert their changes.

## Goal

Make heterogeneous records and scalar properties visible as first-class metric
space evidence. Mixed records must not look like generic colored dots; property
evidence such as density, entropy, anomaly and outlier score must read as
fields over a finite metric space.

## Required Behavior

- Typed glyph grammar:
  - supports at least text/string, time-series, histogram/image-like, vector,
    and composed-record glyph families from exported record metadata
  - uses record IDs and record payload metadata from `metric.visual.v1`
  - can fall back to a neutral glyph without losing identity or selection
  - supports label anchors without HTML overlays
- Property field grammar:
  - maps scalar record properties to ground fields or lifted fields
  - supports density/anomaly/outlier-style ramps without page-specific shaders
  - remains linked to record selection and preview
- The mixed-record public preview must use typed glyphs plus relation edges.
  A point cloud may support the view, but it is not the primary grammar.
- The condition-monitoring preview must show property field plus trajectory/path
  context. A point cloud may support the view, but it is not the primary proof.

## Hard Stop Rules

- Stop if mixed records are represented only by color-coded points.
- Stop if property fields are generated from JavaScript-only fake data instead
  of exported properties.
- Do not add npm, build tooling or external visualization dependencies.

## Acceptance

Run:

```bash
node visual/tools/check-views.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-grae10-golden.mjs
```

Browser-verify mixed-record and condition-monitoring examples. Report:

- primary grammar used by each example
- native evidence path loaded
- visible glyph/field layers
- hover/selection preview behavior
