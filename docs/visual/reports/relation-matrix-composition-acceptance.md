# Relation Matrix Composition Acceptance Report

Date: 2026-06-26

## Summary

The relation-matrix/neighborhood preview now treats the dense relation matrix as
the primary visual object. The example uses the reusable
`showRelationMatrixNeighborhood` command with a larger left-side matrix rect,
smaller supporting graph points, and a pair-preview offset that keeps the panel
outside the matrix center region.

Reusable engine metadata was added to `RelationMatrixView` so the matrix
descriptor declares its primary composition role, matrix center exclusion
region, and pair-preview placement strategy. `RelationMatrixLayer` diagnostics
now expose block coverage and block truncation explicitly in addition to the
existing tile-summary and shader-capacity diagnostics.

No DOM, SVG, or 2D canvas matrix renderer was added. No page-local matrix
renderer was added. GRAE10 files and hero acceptance files were not edited.
The relation-matrix preview remains `public-preview-only`; it is not hero
accepted.

## Evidence

Evidence source:

```text
docs/examples/assets/relation-matrix/metric.visual.json
```

Final browser screenshot:

```text
output/visual/check-visual-regression-public-examples/relation-matrix-neighborhood.canvas.png
```

The visual regression probe reported native evidence with 130 records,
descriptor primitives `InstancedPointLayer`, `RelationEdgeLayer`, and
`RelationMatrixLayer`, full block coverage, 5 labelled blocks, and tile summary
source `exported-relation-texture-downsample`.

## Checks

Passed:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-views.mjs
METRIC_VISUAL_EXAMPLES=relation-matrix-neighborhood node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-relation-matrix-composition.mjs
```

Known local failures:

```bash
node visual/tools/check-relation-matrix-picker.mjs
```

Fails on an existing exact float assertion in the native evidence path:
actual `2.3447810497835038`, expected `2.34478`. I did not edit this checker
because it is outside the task owner scope.

```bash
node visual/tools/check-hero-visual-briefs.mjs
```

The relation-matrix row passes and remains review-pending with blockers
`visual-composition-not-human-accepted` and
`matrix-readability-not-human-accepted`. The command exits non-zero because the
current regression report was intentionally generated for
`relation-matrix-neighborhood` only, so other brief examples are missing from
that report.

```bash
node visual/tools/check-hero-screenshot-review.mjs
```

The relation-matrix row has screenshots, native evidence, grammar OK,
regression OK, and `heroAccepted: false`. The command exits non-zero because
the relation-only regression report does not include the protected
`grae10-metric-engine` entry. No hero acceptance was added.

## Changed Files

```text
visual/src/views/RelationMatrixView.js
visual/src/relational/RelationMatrixLayer.js
visual/examples/relation-matrix-neighborhood/index.html
visual/tools/check-relation-matrix-composition.mjs
docs/visual/reports/relation-matrix-composition-acceptance.md
```
