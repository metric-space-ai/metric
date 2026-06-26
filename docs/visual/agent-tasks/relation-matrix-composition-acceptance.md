# Relation Matrix Composition Acceptance

## Purpose

Make the relation-matrix/neighborhood preview visually readable enough for
manual review by improving matrix scale, block readability, pair focus,
neighborhood graph balance and preview placement through reusable engine code.

This task is about a relation visualization grammar, not a point-cloud hero.

## Owner Scope

Primary write scope:

- `visual/src/views/RelationMatrixView.js`
- `visual/src/relational/RelationMatrixLayer.js`
- `visual/examples/relation-matrix-neighborhood/index.html`
- optional checker: `visual/tools/check-relation-matrix-composition.mjs`
- report: `docs/visual/reports/relation-matrix-composition-acceptance.md`

Allowed supporting files only if necessary:

- `visual/src/views/NeighborhoodGraphView.js`
- `visual/src/relational/matrix-readability.js`
- `visual/tools/check-relation-matrix-readability.mjs`

Do not edit:

- `visual/examples/grae10-metric-engine/**`
- `visual/regression-baselines/grae10-metric-engine.sha256`
- native C++ exporters
- other public hero pages
- `visual/hero-acceptance.manifest.json`

## Current Evidence

Use only:

```text
docs/examples/assets/relation-matrix/metric.visual.json
```

The native evidence currently has 130 records and passes the scale target. The
open blockers are visual composition and matrix readability.

Current screenshot reference:

```text
output/visual/check-visual-regression-public-examples/relation-matrix-neighborhood.canvas.png
```

Current issue: the matrix is present and metadata-backed, but block structure,
selection focus and the relationship to the graph are still too weak for
acceptance. The preview panel can also occlude the selected matrix region.

## Required Outcome

Improve reusable relation-matrix/neighborhood behavior so the screenshot shows:

- a large, readable relation matrix as the primary visual object;
- visible block boundaries and block context without aliasing or moire;
- selected row/column/cell focus that can be read without text;
- neighborhood graph as supporting context rather than competing clutter;
- pair preview positioned so it does not hide the selected matrix evidence;
- matrix diagnostics report block coverage, tile summary source and any block
  truncation explicitly;
- no canvas/SVG fallback or page-local matrix renderer.

## Acceptance

Run:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-views.mjs
node visual/tools/check-hero-visual-briefs.mjs
METRIC_VISUAL_EXAMPLES=relation-matrix-neighborhood node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

If a custom checker is added, it must prove at least:

- selected view kind is `relation-matrix-neighborhood`;
- descriptors include `RelationMatrixLayer`, `RelationEdgeLayer` and
  `InstancedPointLayer`;
- matrix readability diagnostics have at least two labelled blocks and full
  block coverage;
- selected cell/row/column metadata is present after the interaction probe;
- preview positioning does not overlap the matrix center region by default.

Do not mark the hero accepted. Leave `visual-composition-not-human-accepted`
and `matrix-readability-not-human-accepted` until a separate human review clears
them.
