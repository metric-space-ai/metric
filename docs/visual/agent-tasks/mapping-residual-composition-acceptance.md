# Mapping Residual Composition Acceptance

## Purpose

Make the mapping/dimensionality preview read as a coordinate-state morph with
preservation/residual evidence, not as a point cloud hidden behind a dense
starburst of residual vectors.

This task improves reusable engine/view behavior. It must not mutate native
evidence in the page and must not draw a custom mapping page outside the engine.

## Owner Scope

Primary write scope:

- `visual/src/views/MappingView.js`
- `visual/examples/mapping-dimensionality-hero/index.html`
- optional checker: `visual/tools/check-mapping-composition.mjs`
- report: `docs/visual/reports/mapping-residual-composition-acceptance.md`

Allowed supporting files only if necessary:

- `visual/src/layers/RelationEdgeLayer.js`
- `visual/src/views/MetricSpaceView.js`

Do not edit:

- `visual/examples/grae10-metric-engine/**`
- `visual/regression-baselines/grae10-metric-engine.sha256`
- native C++ exporters
- other public hero pages
- `visual/hero-acceptance.manifest.json`

## Current Evidence

Use only:

```text
docs/examples/assets/mapping-dimensionality/metric.visual.json
```

The native evidence currently has 1,000 records and explicit residual evidence.
The open blocker is visual composition.

Current screenshot reference:

```text
output/visual/check-visual-regression-public-examples/mapping-dimensionality-hero.canvas.png
```

Current issue: the residual vectors are present and native, but visually they
form a dense radial bundle that overwhelms the data. The viewer should see the
source/target coordinate morph and the residual evidence as readable distortion,
not a single opaque fan.

## Required Outcome

Improve `MappingView` and/or reusable layer options so the preview shows:

- source-to-target coordinate morph as the primary scene;
- residual/error evidence as readable vectors, bands, sampled representatives,
  or grouped structure without hiding the point structure;
- residual vectors only when an explicit residual property exists;
- no residual fallback with null `residualPropertyId`;
- labels and ground projection that help orientation;
- timing profile with a clear source hold, quick transition and target hold;
- no page-local evidence rewrite and no JavaScript mapping algorithm.

Prefer engine-level options such as residual sampling, representative residual
selection, opacity scaling, vector length clamping, bundling or layer ordering.

## Acceptance

Run:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-views.mjs
node visual/tools/check-hero-visual-briefs.mjs
METRIC_VISUAL_EXAMPLES=mapping-dimensionality-hero node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

If a custom checker is added, it must prove at least:

- selected view kind is `mapping`;
- descriptors include `InstancedPointLayer`, `GroundProjectionLayer` and
  `RelationEdgeLayer`;
- residual layer has a non-empty native residual property id;
- no page-local mutation clears relation values or rewrites native evidence;
- mapping motion timing contains hold/transition/hold phases.

Do not mark the hero accepted. Leave `visual-composition-not-human-accepted`
until a separate human review clears it.
