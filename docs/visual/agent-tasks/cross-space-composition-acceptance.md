# Cross-Space Composition Acceptance

## Purpose

Move the native cross-space dependency preview closer to accepted-hero quality
by making two linked finite metric spaces and their exported dependence evidence
readable through `CrossSpaceView`.

This task is about paired-space visualization. It must not merge both domains
into one point cloud, fake dependence, compute correlations in JavaScript, or
draw page-local bridge helpers.

## Owner Scope

Primary write scope:

- `visual/src/views/CrossSpaceView.js`
- `visual/examples/cross-space-dependency-hero/index.html`
- optional checker: `visual/tools/check-cross-space-composition.mjs`
- report: `docs/visual/reports/cross-space-composition-acceptance.md`

Allowed supporting files only if necessary:

- `visual/src/layers/RelationEdgeLayer.js`
- `visual/src/views/MetricSpaceView.js`
- `visual/src/interaction/selection-store.js`

Do not edit:

- `visual/examples/grae10-metric-engine/**`
- `visual/regression-baselines/grae10-metric-engine.sha256`
- native C++ exporters
- other public hero pages
- `visual/hero-acceptance.manifest.json`

## Current Evidence

Use only:

```text
docs/examples/assets/cross-space-dependency/metric.visual.json
```

The native evidence currently has 512 paired records and multiple exported
relations. The open blocker is visual composition.

Current screenshot reference:

```text
output/visual/check-visual-regression-public-examples/cross-space-dependency-hero.canvas.png
```

Current issue: the scene must read as two finite metric spaces connected by
dependence evidence. The bridges, paired selection and relation semantics must
be the primary proof, not supporting decoration on top of point clouds.

## Required Outcome

Improve `CrossSpaceView` and/or reusable layer options so the preview shows:

- two clearly separated spaces with consistent camera/floor/focus staging;
- dependence bridges or bands that are readable without becoming clutter;
- linked selection metadata that connects records/pairs across spaces;
- labels or group anchors that explain the paired structure without public
  debug/status language;
- hover/selection previews resolving pair evidence through the shared preview
  path;
- no JavaScript MGC/correlation/dependence computation and no page-local bridge
  renderer.

Prefer engine-level options such as bridge sampling, bundling, alpha scaling,
space separation, edge ordering, label anchoring and linked-selection metadata.

## Acceptance

Run:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-cross-space-linked-selection.mjs
node visual/tools/check-linked-selection-presentation.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-views.mjs
node visual/tools/check-hero-visual-briefs.mjs
METRIC_VISUAL_EXAMPLES=cross-space-dependency-hero node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

If a custom checker is added, it must prove at least:

- selected view kind is `cross-space`;
- descriptors include separate source/target record layers and exported
  bridge/relation evidence;
- bridge counts and sampling metadata are explicit;
- linked selection can identify paired records or pair evidence;
- no synthetic evidence or page-local render helper is used.

Do not mark the hero accepted. Leave `visual-composition-not-human-accepted`
until a separate human review clears it.
